#pragma once
#include <iostream>

#include "AudioManager.hpp"
#include "EventBus.hpp"
#include "VehicleTypes.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// DisplayManager.hpp — 화면(스크린) 관리 + 안전/규제 인터록
//
// AVN 화면은 "사용자가 고른 화면"이 아니라 "규칙이 허용하는 화면"을 띄운다.
// 우선순위가 걸린 대표 규칙:
//
//  1) 후방 카메라 강제 전환 (최우선)
//     기어 R 체결 → 무조건 후방 카메라 화면. 법규 요구사항이며(FMVSS 111,
//     한국 자동차규칙 후방 시계장치), 시동 직후에도 2초 내 표시돼야 해서
//     실차는 OS 부팅 전 early-camera 경로를 별도로 설계한다.
//
//  2) 주행 중 영상 시청 제한 (driving restriction / lockout)
//     한국 도로교통법 제49조: 운전 중 운전자 시야의 영상 표시 금지.
//     → 주행 판정(차속 > 0, P단 아님 등) 시 비디오 화면 차단, 소리만 유지.
//     내비/후방카메라 등 주행 관련 영상은 예외.
//
//  3) 통화 화면 오버레이 등 일시적 화면 우선권
//
// 이 클래스는 차량 신호(기어/차속/브레이크)를 구독해 위 규칙대로 최종 표시
// 화면을 계산한다. "사용자 선택(base)"과 "실제 표시(shown)"를 분리하는 것이
// 포인트다. (제약이 풀리면 사용자 선택 화면으로 자동 복귀)
// ─────────────────────────────────────────────────────────────────────────────

enum class Screen { Off, Home, Media, Radio, Nav, Video, VideoLocked, RearCamera, PhoneCall };

inline const char* toString(Screen s) {
    switch (s) {
        case Screen::Off:         return "OFF";
        case Screen::Home:        return "Home";
        case Screen::Media:       return "Media";
        case Screen::Radio:       return "Radio";
        case Screen::Nav:         return "Nav";
        case Screen::Video:       return "Video";
        case Screen::VideoLocked: return "Video(LOCKED)";
        case Screen::RearCamera:  return "RearCamera";
        case Screen::PhoneCall:   return "PhoneCall";
    }
    return "?";
}

class DisplayManager : public IAudioFocusListener {
public:
    DisplayManager(EventBus& bus, AudioManager& audio) : bus_(bus) {
        // 비디오 재생의 오디오 트랙 (간략화: 별도 VideoPlayer 서비스 대신 여기서 처리)
        audio.registerSource(AudioSourceId::Video, this);

        bus_.subscribe(EventType::CanGear, [this](const Event& e) {
            gear_ = static_cast<Gear>(e.i32);
            recompute();
        });
        bus_.subscribe(EventType::CanSpeed, [this](const Event& e) {
            speed_ = e.f64;
            recompute();
        });
        bus_.subscribe(EventType::CanParkBrake, [this](const Event& e) {
            parkBrake_ = e.b;
            recompute();
        });
        bus_.subscribe(EventType::CallStateChanged, [this](const Event& e) {
            inCall_ = e.b;
            recompute();
        });
        bus_.subscribe(EventType::IgnitionChanged, [this](const Event& e) {
            ignOn_ = (static_cast<Ignition>(e.i32) != Ignition::Off);
            if (!ignOn_) base_ = Screen::Home;  // 전원 OFF 시 사용자 선택 초기화
            recompute();
        });
    }

    // 사용자(HMI)의 화면 전환 요청 — "요청"일 뿐, 최종 화면은 규칙이 결정한다.
    void requestScreen(Screen s) {
        base_ = s;
        recompute();
        if (base_ == Screen::Video && shown_ == Screen::VideoLocked) {
            std::cout << "[Display] video is locked while driving"
                         " (stop the car and shift to P)\n";
        }
    }

    Screen shown() const { return shown_; }

    // 주정차 판정: 정지 + (P단 또는 주차 브레이크)
    bool videoAllowed() const {
        return speed_ < 1.0 && (gear_ == Gear::P || parkBrake_);
    }

    // Video 소스의 오디오 포커스 콜백 (간략화)
    void onAudioFocus(FocusState st) override {
        switch (st) {
            case FocusState::Playing: std::cout << "[Video] audio playing\n"; break;
            case FocusState::Ducked:  std::cout << "[Video] audio ducked\n";  break;
            case FocusState::Paused:  std::cout << "[Video] audio paused\n";  break;
            case FocusState::Stopped: std::cout << "[Video] audio stopped\n"; break;
        }
    }

private:
    void recompute() {
        Screen next;
        if (!ignOn_)                 next = Screen::Off;
        else if (gear_ == Gear::R)   next = Screen::RearCamera;   // 1순위: 안전
        else if (inCall_)            next = Screen::PhoneCall;    // 2순위: 통화 오버레이
        else if (base_ == Screen::Video && !videoAllowed())
                                     next = Screen::VideoLocked;  // 3순위: 주행 규제
        else                         next = base_;                // 사용자 선택 화면

        if (next == shown_) return;
        shown_ = next;

        std::cout << "[Display] screen -> " << toString(shown_);
        if (shown_ == Screen::RearCamera)
            std::cout << "  (safety override: gear R, must show within 2s)";
        if (shown_ == Screen::VideoLocked)
            std::cout << "  (driving restriction: video blacked out, audio continues)";
        std::cout << "\n";

        Event e; e.type = EventType::ScreenChanged; e.i32 = static_cast<int>(shown_);
        bus_.publish(e);
    }

    EventBus& bus_;
    Screen base_  = Screen::Home;  // 사용자가 선택한 화면
    Screen shown_ = Screen::Off;   // 규칙 적용 후 실제 표시 화면
    Gear   gear_  = Gear::P;
    double speed_ = 0.0;
    bool   parkBrake_ = true;
    bool   inCall_    = false;
    bool   ignOn_     = false;
};
