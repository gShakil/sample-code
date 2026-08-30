#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

#include "EventBus.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// AudioManager.hpp — 오디오 중재(Audio Arbitration / Audio Focus)
//
// AVN에서 가장 중요한 서비스 중 하나. 차 안에는 스피커를 쓰고 싶어하는 소스가
// 매우 많다: 라디오, USB 음악, BT 오디오, 내비 안내음성(TTS), 전화(HFP),
// 각종 경고음(차임), 후방 경고음, 긴급콜(eCall)...
// 이들을 "동시에 어떻게 섞고, 누구를 우선할지" 결정하는 것이 오디오 중재다.
//
// 대표 정책:
//   - 주(main) 소스는 한 번에 하나 (라디오 ↔ USB ↔ BT 전환 = 소스 전환)
//   - 내비 TTS: 주 소스를 잠시 "덕킹(ducking)" — 볼륨을 확 낮추고 위에 얹음
//   - 전화: 주 소스를 "일시정지(pause)", 통화 종료 후 자동 복귀(resume)
//   - 긴급음 > 전화 > 내비 안내 > 미디어  순의 우선순위
//
// 실무 대응물:
//   - Android Automotive: AudioFocus + CarAudioService (usage/zone 기반)
//   - GENIVI/COVESA AudioManager, PulseAudio/PipeWire 라우팅
//   - 하드웨어단: 오디오 DSP, 외장 앰프(A2B/AVB 링크)로 실제 믹싱/라우팅
//
// 속도 감응 볼륨(SDVC): 차속이 올라가면 주행 소음을 보상해 볼륨을 자동 보정.
// ─────────────────────────────────────────────────────────────────────────────

enum class AudioSourceId { None, Radio, UsbMedia, BtAudio, Video, NavTts, Phone };

inline const char* toString(AudioSourceId s) {
    switch (s) {
        case AudioSourceId::None:     return "None";
        case AudioSourceId::Radio:    return "Radio";
        case AudioSourceId::UsbMedia: return "USB";
        case AudioSourceId::BtAudio:  return "BT";
        case AudioSourceId::Video:    return "Video";
        case AudioSourceId::NavTts:   return "NavTTS";
        case AudioSourceId::Phone:    return "Phone";
    }
    return "?";
}

// 소스 입장에서 "지금 나는 어떤 상태여야 하는가" (Android AudioFocus와 유사)
enum class FocusState { Stopped, Playing, Ducked, Paused };

inline const char* toString(FocusState s) {
    switch (s) {
        case FocusState::Stopped: return "STOPPED";
        case FocusState::Playing: return "PLAYING";
        case FocusState::Ducked:  return "DUCKED";
        case FocusState::Paused:  return "PAUSED";
    }
    return "?";
}

// 잠시 끼어드는(transient) 소리가 주 소스에 요구하는 동작
enum class TransientType {
    Duck,  // 주 소스 볼륨만 낮춤 (내비 TTS, 차임)
    Pause  // 주 소스를 멈춤 (전화, 긴급콜)
};

// 오디오 소스가 구현해야 하는 콜백 인터페이스
struct IAudioFocusListener {
    virtual ~IAudioFocusListener() = default;
    virtual void onAudioFocus(FocusState state) = 0;
};

class AudioManager {
public:
    explicit AudioManager(EventBus& bus) {
        // 속도 감응 볼륨(SDVC)을 위해 차속 신호 구독
        bus.subscribe(EventType::CanSpeed,
                      [this](const Event& e) { onSpeed(e.f64); });
    }

    void registerSource(AudioSourceId id, IAudioFocusListener* listener) {
        sources_[id] = listener;
    }

    AudioSourceId mainSource() const { return main_; }

    // ── 주(main) 소스 전환 ──────────────────────────────────────────────────
    // 라디오 ↔ USB ↔ 비디오 전환. 이전 소스는 Stopped 통보를 받는다.
    void selectMainSource(AudioSourceId id) {
        if (main_ == id) { notifyMain(); return; }

        if (main_ != AudioSourceId::None) {
            std::cout << "[Audio] source switch: " << toString(main_)
                      << " -> " << toString(id) << "\n";
            if (auto* old = listenerOf(main_)) old->onAudioFocus(FocusState::Stopped);
        }
        main_ = id;
        lastNotified_ = FocusState::Stopped;

        if (main_ == AudioSourceId::None) {
            std::cout << "[Audio] no main source (silence)\n";
        } else {
            notifyMain();  // 새 소스에 현재 상태(보통 Playing) 통보 → 소스가 재생 시작
        }
    }

    // ── 끼어드는 소리(transient)의 포커스 요청/해제 ─────────────────────────
    void requestTransient(AudioSourceId who, TransientType type) {
        transients_[who] = type;
        std::cout << "[Audio] focus granted to " << toString(who) << " ("
                  << (type == TransientType::Pause ? "main PAUSE" : "main DUCK")
                  << ")\n";
        notifyMain();
    }

    void releaseTransient(AudioSourceId who) {
        if (transients_.erase(who) == 0) return;
        std::cout << "[Audio] focus released by " << toString(who) << "\n";
        notifyMain();
    }

    // ── 볼륨 ────────────────────────────────────────────────────────────────
    void setVolume(int v) {
        volume_ = std::max(0, std::min(v, 40));  // AVN은 보통 0~40 단계
        std::cout << "[Audio] " << volumeText() << "\n";
    }
    void volumeStep(int delta) { setVolume(volume_ + delta); }

    int effectiveVolume() const {
        int v = std::max(0, std::min(volume_ + speedOffset_, 40));
        if (mainStateNow() == FocusState::Ducked)
            v = std::max(1, static_cast<int>(std::lround(v * 0.3)));  // 덕킹: 30%로
        return v;
    }

    std::string volumeText() const {
        std::string s = "vol " + std::to_string(volume_);
        if (speedOffset_ > 0) s += "(+" + std::to_string(speedOffset_) + " speed)";
        s += " -> effective " + std::to_string(effectiveVolume());
        if (mainStateNow() == FocusState::Ducked) s += " [DUCKED]";
        return s;
    }

private:
    // 현재 주 소스가 있어야 할 상태 계산: Pause 요청 > Duck 요청 > Playing
    FocusState mainStateNow() const {
        if (main_ == AudioSourceId::None) return FocusState::Stopped;
        bool pause = false, duck = false;
        for (const auto& kv : transients_) {
            if (kv.second == TransientType::Pause) pause = true;
            else duck = true;
        }
        if (pause) return FocusState::Paused;
        if (duck)  return FocusState::Ducked;
        return FocusState::Playing;
    }

    void notifyMain() {
        if (main_ == AudioSourceId::None) return;
        FocusState st = mainStateNow();
        if (st == lastNotified_) return;
        lastNotified_ = st;
        std::cout << "[Audio] main(" << toString(main_) << ") -> " << toString(st)
                  << ", " << volumeText() << "\n";
        if (auto* l = listenerOf(main_)) l->onAudioFocus(st);
    }

    void onSpeed(double kph) {
        // 속도 감응 볼륨(SDVC): 주행 소음 보상. 실차는 차속/RPM 기반 커브 테이블 사용.
        int offset = (kph >= 100) ? 3 : (kph >= 60) ? 2 : (kph >= 30) ? 1 : 0;
        if (offset == speedOffset_) return;
        speedOffset_ = offset;
        std::cout << "[Audio] SDVC offset -> +" << offset << " (speed "
                  << static_cast<int>(kph) << " km/h), " << volumeText() << "\n";
    }

    IAudioFocusListener* listenerOf(AudioSourceId id) const {
        auto it = sources_.find(id);
        return (it != sources_.end()) ? it->second : nullptr;
    }

    std::map<AudioSourceId, IAudioFocusListener*> sources_;
    std::map<AudioSourceId, TransientType>        transients_;  // 활성 끼어들기 목록
    AudioSourceId main_         = AudioSourceId::None;
    FocusState    lastNotified_ = FocusState::Stopped;
    int           volume_       = 12;  // 사용자 볼륨 (0~40)
    int           speedOffset_  = 0;   // SDVC 보정치
};
