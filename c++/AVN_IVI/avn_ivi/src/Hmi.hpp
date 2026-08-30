#pragma once
#include <iostream>

#include "AudioManager.hpp"
#include "CanBusSimulator.hpp"
#include "DisplayManager.hpp"
#include "EventBus.hpp"
#include "MediaPlayer.hpp"
#include "NavigationService.hpp"
#include "PowerManager.hpp"
#include "RadioTuner.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Hmi.hpp — HMI(Human-Machine Interface) 계층
//
// 실제 AVN의 HMI:
//   - 터치스크린 GUI (Qt/QML, Kanzi, Unreal/Unity HMI, Android Automotive UI 등)
//   - 하드키/로터리 노브, 스티어링 휠 리모컨(CAN으로 수신), 음성인식
//   - 주행 중 조작 제한(distraction 규제: 글자수 제한, 목록 스크롤 제한 등)
//
// 아키텍처 관점의 핵심: HMI는 "표시와 입력"만 담당하고, 비즈니스 로직은 전부
// 서비스 계층에 있다. HMI는 서비스의 상태를 구독(또는 바인딩)해서 그리고,
// 사용자 입력을 서비스 호출로 변환할 뿐이다. (화면을 갈아엎어도 서비스는 불변)
//
// 여기서는 콘솔 한 줄짜리 상태 표시가 "화면"이고, main.cpp의 명령 파서가
// "터치 입력"에 해당한다.
// ─────────────────────────────────────────────────────────────────────────────

class Hmi {
public:
    Hmi(EventBus& bus, PowerManager& power, CanBusSimulator& can,
        DisplayManager& display, AudioManager& audio,
        MediaPlayer& media, RadioTuner& radio, NavigationService& nav)
        : power_(power), can_(can), display_(display),
          audio_(audio), media_(media), radio_(radio), nav_(nav) {
        // 내비 TTS 이벤트를 받아 "스피커 출력"을 표현
        bus.subscribe(EventType::NavGuidance, [](const Event& e) {
            std::cout << "[HMI/TTS] \"" << e.str << "\"\n";
        });
    }

    // 클러스터의 상태 표시줄에 해당하는 요약 정보
    void renderStatus() const {
        std::cout << "  STATUS | " << toString(power_.state())
                  << " | gear " << toString(can_.gear())
                  << " " << static_cast<int>(can_.speed()) << "km/h"
                  << (can_.parkBrake() ? " PBRK" : "")
                  << " | screen: " << toString(display_.shown())
                  << (nav_.active() ? " | nav: " + nav_.destination() : std::string())
                  << " | audio: " << audioLine()
                  << "\n";
    }

private:
    std::string audioLine() const {
        switch (audio_.mainSource()) {
            case AudioSourceId::None:
                return "silent";
            case AudioSourceId::UsbMedia:
                return "USB " + std::string(media_.playing() ? "PLAYING " : "PAUSED ")
                     + media_.nowPlaying() + ", " + audio_.volumeText();
            case AudioSourceId::Radio:
                return "Radio " + radio_.stationText() + ", " + audio_.volumeText();
            case AudioSourceId::Video:
                return "Video audio, " + audio_.volumeText();
            default:
                return toString(audio_.mainSource());
        }
    }

    PowerManager&      power_;
    CanBusSimulator&   can_;
    DisplayManager&    display_;
    AudioManager&      audio_;
    MediaPlayer&       media_;
    RadioTuner&        radio_;
    NavigationService& nav_;
};
