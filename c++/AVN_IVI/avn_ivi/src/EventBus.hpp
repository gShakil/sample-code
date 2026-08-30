#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// EventBus.hpp — 발행/구독(pub/sub) 이벤트 버스
//
// IVI 시스템은 수십 개의 서비스(오디오, 미디어, 내비, 폰, 공조, 차량신호...)가
// 서로 직접 함수를 호출하지 않고, 미들웨어를 통해 "이벤트/메시지"로 통신하는
// 이벤트 기반(event-driven) 아키텍처를 쓴다. 모듈 간 결합도를 낮추기 위해서다.
//
// 실차에서 이 역할을 하는 것들(= 이 파일이 흉내내는 것):
//   - Linux IVI (AGL/GENIVI·COVESA 계열): D-Bus, CommonAPI + SOME/IP(vsomeip)
//   - AUTOSAR Adaptive: ara::com (SOME/IP 바인딩)
//   - Android Automotive OS: Binder/AIDL, Car API + VHAL(Vehicle HAL)
//   - QNX: PPS(Persistent Publish/Subscribe)
//
// 실제로는 "프로세스 간(IPC)" 또는 "ECU 간(차량 이더넷)" 통신이지만,
// 여기서는 학습용으로 단일 프로세스 내 콜백 호출로 단순화했다.
// ─────────────────────────────────────────────────────────────────────────────

enum class EventType {
    // 차량 신호 계열 (CAN 게이트웨이가 수신해 뿌려주는 원시 신호)
    CanIgnition,      // i32 = Ignition
    CanGear,          // i32 = Gear
    CanSpeed,         // f64 = km/h
    CanParkBrake,     // b   = 주차 브레이크 체결 여부

    // 서비스 계열 (각 서비스가 가공해 발행하는 상태)
    IgnitionChanged,  // i32 = Ignition  (PowerManager가 검증 후 발행한 시스템 전원 상태)
    ScreenChanged,    // i32 = Screen
    CallStateChanged, // b = 통화 중 여부, str = 상대방
    NavGuidance       // str = TTS 안내 문구
};

// 학습용 범용 payload. 실무에서는 이벤트마다 강타입 메시지(IDL로 정의)를 쓴다.
struct Event {
    EventType type{};
    int         i32 = 0;
    double      f64 = 0.0;
    bool        b   = false;
    std::string str;
};

class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    void subscribe(EventType type, Handler handler) {
        subscribers_[type].push_back(std::move(handler));
    }

    // 주의: 같은 이벤트의 구독자들은 "등록된 순서"로 호출된다.
    // 실무에서는 이런 암묵적 순서 의존을 피하고, 순서가 중요한 초기화/종료는
    // 상태 관리자(AUTOSAR StateManager, systemd target 등)가 명시적으로 지휘한다.
    void publish(const Event& e) {
        auto it = subscribers_.find(e.type);
        if (it == subscribers_.end()) return;
        for (auto& handler : it->second) handler(e);
    }

private:
    std::map<EventType, std::vector<Handler>> subscribers_;
};
