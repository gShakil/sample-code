// ─────────────────────────────────────────────────────────────────────────────
// main.cpp — AVN/IVI 학습용 시뮬레이터: 조립(composition root) + HMI 입력 루프
//
// 빌드:  g++ -std=c++17 -Wall -Wextra -O2 -o avn_sim src/main.cpp
// 실행:  ./avn_sim   →  'demo' 입력 시 전체 시나리오 자동 시연
//
// 실차와 이 예제의 차이(중요):
//   - 실차는 서비스마다 별도 프로세스/스레드 + IPC(D-Bus, SOME/IP, Binder).
//     여기서는 단일 스레드 + EventBus 콜백으로 단순화 (동작 순서가 결정적이라
//     학습에 유리).
//   - 실차의 오디오/영상은 DSP·HW코덱·디스플레이 파이프라인이 처리. 여기서는
//     로그 출력으로 대신한다.
//   - 실차는 진단(UDS, DoIP), OTA, 기능안전(ISO 26262), 사이버보안(ISO 21434,
//     UN R155/156) 요구가 추가로 얹힌다.
// ─────────────────────────────────────────────────────────────────────────────
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "AudioManager.hpp"
#include "CanBusSimulator.hpp"
#include "DisplayManager.hpp"
#include "EventBus.hpp"
#include "Hmi.hpp"
#include "MediaPlayer.hpp"
#include "NavigationService.hpp"
#include "PhoneService.hpp"
#include "PowerManager.hpp"
#include "RadioTuner.hpp"
#include "VehicleTypes.hpp"

// ── 시스템 조립 ───────────────────────────────────────────────────────────────
// 계층 구조(위 → 아래):
//   HMI(Hmi) → 애플리케이션/서비스(Media, Radio, Nav, Phone, Display, Power)
//   → 플랫폼(AudioManager, EventBus) → 차량 인터페이스(CanBusSimulator)
struct IviSystem {
    EventBus          bus;
    CanBusSimulator   can{bus};
    AudioManager      audio{bus};
    PowerManager      power{bus, audio};
    DisplayManager    display{bus, audio};
    MediaPlayer       media{audio};
    RadioTuner        radio{audio};
    NavigationService nav{bus, audio};
    PhoneService      phone{bus, audio};
    Hmi               hmi{bus, power, can, display, audio, media, radio, nav};
};

// ── 유틸 ─────────────────────────────────────────────────────────────────────
static std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> out;
    std::istringstream iss(line);
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static std::string toLower(std::string s) {
    for (char& c : s)
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    return s;
}

static void printHelp() {
    std::cout <<
        "\n--- vehicle simulation (= CAN signals arriving at the head unit) ---\n"
        "  ign off|acc|on         ignition state\n"
        "  gear p|r|n|d           shift lever\n"
        "  speed <kph>            vehicle speed\n"
        "  brake on|off           parking brake\n"
        "  can 100 <b0>..<b7>     inject raw CAN frame, hex (DBC decode demo)\n"
        "                         e.g. can 100 02 03 20 03 00 00 00 00\n"
        "--- AVN functions (= user touch/key input on the HMI) ---\n"
        "  media play|pause|next|prev     USB music\n"
        "  radio | radio seek | radio tune <MHz>\n"
        "  video on|off                   video player (driving restriction demo)\n"
        "  nav go <dest> | nav tick | nav stop\n"
        "  call in [name] | call end      BT hands-free call\n"
        "  vol up|down|<0..40>            volume\n"
        "--- etc ---\n"
        "  status | demo | help | quit\n\n";
}

// ── 명령 처리 (HMI 입력 → 서비스 호출로 변환) ────────────────────────────────
static bool dispatch(IviSystem& s, const std::string& line, bool inDemo);

static void runDemo(IviSystem& s) {
    struct Step { const char* note; const char* cmd; };
    static const Step steps[] = {
        {"power on -> welcome, last-mode restore (first boot: nothing saved)", "ign on"},
        {"select USB media source and start playback",                         "media play"},
        {"drive at 80 km/h -> SDVC raises effective volume",                   "speed 80"},
        {"set a navigation destination",                                       "nav go Gangnam Station"},
        {"nav TTS ducks media volume, then restores it",                       "nav tick"},
        {"incoming BT call -> media pauses",                                   "call in Mom"},
        {"call ends -> media auto-resumes",                                    "call end"},
        {"reverse gear -> rear camera forced (safety top priority)",           "gear r"},
        {"leave reverse -> previous screen restored",                          "gear d"},
        {"video while driving -> screen locked, audio continues",              "video on"},
        {"stop the car ...",                                                   "speed 0"},
        {"... shift to P -> video screen auto-unlocks",                        "gear p"},
        {"close video, switch source to FM radio",                            "video off"},
        {"",                                                                   "radio"},
        {"seek to next preset station",                                        "radio seek"},
        {"ignition off -> last mode (Radio) saved",                            "ign off"},
        {"ACC power -> boot and restore last mode (Radio)",                    "ign acc"},
    };
    for (const Step& st : steps) {
        std::cout << "\n";
        if (st.note[0]) std::cout << "### " << st.note << "\n";
        std::cout << ">>> " << st.cmd << "\n";
        dispatch(s, st.cmd, /*inDemo=*/true);
    }
    std::cout << "\n### demo finished. try your own commands (help)\n";
}

static bool dispatch(IviSystem& s, const std::string& line, bool inDemo) {
    std::vector<std::string> t = tokenize(line);
    if (t.empty()) return true;
    const std::string cmd = toLower(t[0]);
    const std::string arg = (t.size() > 1) ? toLower(t[1]) : "";

    try {
        // ── 항상 가능한 명령 ────────────────────────────────────────────────
        if (cmd == "quit" || cmd == "exit") return false;
        if (cmd == "help")   { printHelp(); return true; }
        if (cmd == "status") { s.hmi.renderStatus(); return true; }
        if (cmd == "demo" && !inDemo) { runDemo(s); return true; }

        // ── 차량 신호 시뮬레이션 (전원과 무관하게 CAN은 항상 흐른다) ───────
        if (cmd == "ign") {
            if      (arg == "off") s.can.setIgnition(Ignition::Off);
            else if (arg == "acc") s.can.setIgnition(Ignition::Acc);
            else if (arg == "on" || arg == "run") s.can.setIgnition(Ignition::Run);
            else { std::cout << "usage: ign off|acc|on\n"; return true; }
        }
        else if (cmd == "gear") {
            if      (arg == "p") s.can.setGear(Gear::P);
            else if (arg == "r") s.can.setGear(Gear::R);
            else if (arg == "n") s.can.setGear(Gear::N);
            else if (arg == "d") s.can.setGear(Gear::D);
            else { std::cout << "usage: gear p|r|n|d\n"; return true; }
        }
        else if (cmd == "speed") {
            if (t.size() < 2) { std::cout << "usage: speed <kph>\n"; return true; }
            s.can.setSpeed(std::stod(t[1]));
        }
        else if (cmd == "brake") {
            if (arg == "on" || arg == "off") s.can.setParkBrake(arg == "on");
            else { std::cout << "usage: brake on|off\n"; return true; }
        }
        else if (cmd == "can") {
            if (t.size() != 10) {
                std::cout << "usage: can <id-hex> <8 data bytes in hex>\n";
                return true;
            }
            uint32_t id = static_cast<uint32_t>(std::stoul(t[1], nullptr, 16));
            uint8_t data[8];
            for (int i = 0; i < 8; ++i)
                data[i] = static_cast<uint8_t>(std::stoul(t[2 + i], nullptr, 16));
            s.can.injectFrame(id, data);
        }

        // ── 이하 AVN 기능: 전원이 있어야 동작 ───────────────────────────────
        else if (!s.power.systemOn()) {
            std::cout << "[Power] head unit is OFF (try: ign acc / ign on)\n";
            return true;
        }
        else if (cmd == "media") {
            if      (arg == "play")  s.media.play();
            else if (arg == "pause") s.media.pause();
            else if (arg == "next")  s.media.next();
            else if (arg == "prev")  s.media.prev();
            else { std::cout << "usage: media play|pause|next|prev\n"; return true; }
            if (arg == "play") s.display.requestScreen(Screen::Media);
        }
        else if (cmd == "radio") {
            if      (arg.empty())   { s.radio.on(); s.display.requestScreen(Screen::Radio); }
            else if (arg == "seek")   s.radio.seek();
            else if (arg == "tune" && t.size() > 2) s.radio.tune(std::stod(t[2]));
            else { std::cout << "usage: radio | radio seek | radio tune <MHz>\n"; return true; }
        }
        else if (cmd == "video") {
            if (arg == "on") {
                s.display.requestScreen(Screen::Video);       // 화면은 규칙이 결정
                s.audio.selectMainSource(AudioSourceId::Video); // 소리는 계속 나감
            } else if (arg == "off") {
                s.display.requestScreen(Screen::Home);
                s.audio.selectMainSource(AudioSourceId::None);
            } else { std::cout << "usage: video on|off\n"; return true; }
        }
        else if (cmd == "nav") {
            if (arg == "go" && t.size() > 2) {
                std::string dest = t[2];
                for (size_t i = 3; i < t.size(); ++i) dest += " " + t[i];
                s.nav.go(dest);
                s.display.requestScreen(Screen::Nav);
            }
            else if (arg == "tick") s.nav.tick();
            else if (arg == "stop") s.nav.stop();
            else { std::cout << "usage: nav go <dest> | nav tick | nav stop\n"; return true; }
        }
        else if (cmd == "call") {
            if (arg == "in") {
                std::string who = (t.size() > 2) ? t[2] : "Unknown";
                for (size_t i = 3; i < t.size(); ++i) who += " " + t[i];
                s.phone.incomingCall(who);
            }
            else if (arg == "end") s.phone.endCall();
            else { std::cout << "usage: call in [name] | call end\n"; return true; }
        }
        else if (cmd == "vol") {
            if      (arg == "up")   s.audio.volumeStep(+2);
            else if (arg == "down") s.audio.volumeStep(-2);
            else if (!arg.empty())  s.audio.setVolume(std::stoi(t[1]));
            else { std::cout << "usage: vol up|down|<0..40>\n"; return true; }
        }
        else {
            std::cout << "unknown command: '" << cmd << "' (help)\n";
            return true;
        }
    } catch (const std::exception&) {
        std::cout << "invalid number in command (help)\n";
        return true;
    }

    s.hmi.renderStatus();  // 매 조작 후 HMI 상태 표시줄 갱신
    return true;
}

int main() {
    std::cout <<
        "=====================================================\n"
        "  AVN/IVI head-unit simulator (educational, C++17)\n"
        "  type 'demo' for a guided scenario, 'help' for keys\n"
        "=====================================================\n";

    IviSystem sys;
    sys.hmi.renderStatus();

    std::string line;
    while (true) {
        std::cout << "\navn> " << std::flush;
        if (!std::getline(std::cin, line)) break;   // EOF
        if (!dispatch(sys, line, /*inDemo=*/false)) break;
    }
    std::cout << "bye.\n";
    return 0;
}
