#pragma once
#include <iostream>

#include "AudioManager.hpp"
#include "EventBus.hpp"
#include "VehicleTypes.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// PowerManager.hpp — 전원 상태 머신 + 라스트 모드(Last Mode)
//
// AVN은 PC처럼 "켜고 끄는" 장치가 아니라, 차량 전원 단계(OFF → ACC → IG ON)를
// 따라 움직이는 임베디드 장치다. 전장 SW에서 전원 관리가 중요한 이유:
//
//   - 부팅 시간 요구: 후진 기어 체결 후 2초 내 후방 카메라 영상 출력
//     (미국 FMVSS 111 / 한국 자동차규칙의 후방 시계 규정). 그래서 실차는
//     풀 OS 부팅 전에 카메라 경로만 먼저 살리는 early-RVC 설계를 쓴다.
//   - 암전류(dark current) 관리: 시동 OFF 후에도 SoC가 깨어 있으면 배터리가
//     방전된다. 슬립/웨이크업, CAN 네트워크 매니지먼트(NM)와 연동해야 한다.
//   - 라스트 모드: 시동을 다시 걸면 끄기 전 상태(소스, 볼륨, 화면)로 복원.
//     실차에서는 전원이 내려가기 전 짧은 시간 안에 NVM(EEPROM/eMMC)에 저장한다.
//
// 이 예제는 OFF/ACC/RUN 3단계와 라스트 모드 복원만 구현한다.
// ─────────────────────────────────────────────────────────────────────────────

class PowerManager {
public:
    PowerManager(EventBus& bus, AudioManager& audio) : bus_(bus), audio_(audio) {
        // CAN에서 올라온 원시 점화 신호를 받아 시스템 전원 상태로 확정한다.
        bus_.subscribe(EventType::CanIgnition, [this](const Event& e) {
            apply(static_cast<Ignition>(e.i32));
        });
    }

    Ignition state()   const { return state_; }
    bool     systemOn() const { return state_ != Ignition::Off; }

private:
    void apply(Ignition next) {
        if (next == state_) return;
        Ignition prev = state_;
        state_ = next;
        std::cout << "[Power] state " << toString(prev) << " -> " << toString(next) << "\n";

        if (next == Ignition::Off) {
            // 종료 시퀀스: 라스트 모드 저장 → 소스 정지 → (실차: NVM 기록, 슬립 진입)
            lastSource_ = audio_.mainSource();
            audio_.selectMainSource(AudioSourceId::None);
            std::cout << "[Power] shutdown. last mode saved to NVM: "
                      << toString(lastSource_) << "\n";
        } else if (prev == Ignition::Off) {
            // 기동 시퀀스: (실차: 부트로더 → 커널 → 서비스 기동) → 라스트 모드 복원
            std::cout << "[Power] welcome! restoring last mode: "
                      << toString(lastSource_) << "\n";
            if (lastSource_ != AudioSourceId::None)
                audio_.selectMainSource(lastSource_);
        }

        // 검증이 끝난 "시스템 전원 상태"를 다른 서비스에 알린다.
        Event e; e.type = EventType::IgnitionChanged; e.i32 = static_cast<int>(next);
        bus_.publish(e);
    }

    EventBus&     bus_;
    AudioManager& audio_;
    Ignition      state_      = Ignition::Off;
    AudioSourceId lastSource_ = AudioSourceId::None;  // 라스트 모드 (실차: NVM 저장)
};
