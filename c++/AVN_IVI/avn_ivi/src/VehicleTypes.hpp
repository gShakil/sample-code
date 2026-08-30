#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// VehicleTypes.hpp — 차량 공용 타입 정의
//
// 실제 양산 프로젝트에서는 이런 열거형/신호 타입이 차량 신호 데이터베이스
// (DBC, FIBEX, AUTOSAR ARXML)로부터 코드 생성기를 통해 자동 생성되는 경우가
// 많다. 여기서는 학습용으로 손으로 정의한다.
// ─────────────────────────────────────────────────────────────────────────────

enum class Ignition {
    Off,  // 전원 차단 (B+ 상시 전원만 살아있는 상태)
    Acc,  // 액세서리 전원 — 오디오 등 편의장치 동작 가능
    Run   // IG ON — 엔진/주행 상태
};

enum class Gear { P, R, N, D };  // 변속 레버 위치 (PRND)

inline const char* toString(Ignition v) {
    switch (v) {
        case Ignition::Off: return "OFF";
        case Ignition::Acc: return "ACC";
        case Ignition::Run: return "RUN";
    }
    return "?";
}

inline const char* toString(Gear v) {
    switch (v) {
        case Gear::P: return "P";
        case Gear::R: return "R";
        case Gear::N: return "N";
        case Gear::D: return "D";
    }
    return "?";
}
