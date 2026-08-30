#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "EventBus.hpp"
#include "VehicleTypes.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// CanBusSimulator.hpp — 차량 신호(CAN) 수신부 시뮬레이터
//
// AVN/IVI는 차량 네트워크(CAN, CAN-FD, LIN, 차량 이더넷)에 물려 있는 하나의
// ECU다. 기어 위치, 차속, 주차 브레이크, 점화 상태, 후진 신호, 조도(주/야간),
// 스티어링 휠 리모컨 키 입력 같은 정보를 모두 CAN으로 받는다.
//
// 실차 수신 경로:
//   CAN 트랜시버 → MCU(또는 SoC의 CAN 컨트롤러) → 신호 게이트웨이 SW
//   → DBC(신호 정의 파일) 기반 디코딩 → 서비스에 신호 배포
//
// 이 클래스는 그 결과만 흉내낸다:
//   - setXxx()      : "디코딩이 끝난 신호가 도착했다"에 해당
//   - injectFrame() : 원시 8바이트 CAN 프레임을 DBC 규칙대로 직접 풀어보는 예제
//
// 값이 이전과 같으면 발행하지 않는 것(dedupe)도 실무 관례다. CAN 프레임은
// 보통 10~100ms 주기로 반복 송신되므로, 그대로 다 뿌리면 이벤트 폭주가 난다.
// ─────────────────────────────────────────────────────────────────────────────

class CanBusSimulator {
public:
    explicit CanBusSimulator(EventBus& bus) : bus_(bus) {}

    void setIgnition(Ignition v) {
        if (ign_ == v) return;
        ign_ = v;
        std::cout << "[CAN] ignition = " << toString(v) << "\n";
        Event e; e.type = EventType::CanIgnition; e.i32 = static_cast<int>(v);
        bus_.publish(e);
    }

    void setGear(Gear v) {
        if (gear_ == v) return;
        gear_ = v;
        std::cout << "[CAN] gear = " << toString(v) << "\n";
        Event e; e.type = EventType::CanGear; e.i32 = static_cast<int>(v);
        bus_.publish(e);
    }

    void setSpeed(double kph) {
        kph = std::max(0.0, std::min(kph, 250.0));
        if (speed_ == kph) return;
        speed_ = kph;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "[CAN] vehicle speed = %.1f km/h", kph);
        std::cout << buf << "\n";
        Event e; e.type = EventType::CanSpeed; e.f64 = kph;
        bus_.publish(e);
    }

    void setParkBrake(bool applied) {
        if (parkBrake_ == applied) return;
        parkBrake_ = applied;
        std::cout << "[CAN] parking brake = " << (applied ? "APPLIED" : "RELEASED") << "\n";
        Event e; e.type = EventType::CanParkBrake; e.b = applied;
        bus_.publish(e);
    }

    // ── 원시 CAN 프레임 디코딩 예제 ─────────────────────────────────────────
    // 가상의 DBC 정의:
    //   메시지 ID 0x100 "VehicleStatus", DLC 8, little-endian
    //     byte0      : ignition      (0=OFF, 1=ACC, 2=RUN)
    //     byte1      : gear          (0=P, 1=R, 2=N, 3=D)
    //     byte2..3   : vehicleSpeed  (u16 LE, 분해능 0.1 km/h/bit)
    //     byte4 bit0 : parkingBrake  (1=체결)
    //   → 예: can 100 02 03 20 03 00 00 00 00  =>  RUN, D, 80.0km/h, 브레이크 해제
    void injectFrame(uint32_t id, const uint8_t data[8]) {
        char buf[96];
        std::snprintf(buf, sizeof(buf),
                      "[CAN] RX id=0x%03X data=%02X %02X %02X %02X %02X %02X %02X %02X",
                      id, data[0], data[1], data[2], data[3],
                      data[4], data[5], data[6], data[7]);
        std::cout << buf << "\n";

        if (id != 0x100) {
            std::cout << "[CAN] unknown message id (not in DBC) — ignored\n";
            return;
        }
        setIgnition(static_cast<Ignition>(std::min<int>(data[0], 2)));
        setGear(static_cast<Gear>(std::min<int>(data[1], 3)));
        uint16_t rawSpeed = static_cast<uint16_t>(data[2] | (data[3] << 8));
        setSpeed(rawSpeed * 0.1);
        setParkBrake((data[4] & 0x01) != 0);
    }

    Ignition ignition()  const { return ign_; }
    Gear     gear()      const { return gear_; }
    double   speed()     const { return speed_; }
    bool     parkBrake() const { return parkBrake_; }

private:
    EventBus& bus_;
    Ignition ign_       = Ignition::Off;
    Gear     gear_      = Gear::P;
    double   speed_     = 0.0;
    bool     parkBrake_ = true;
};
