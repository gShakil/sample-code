#pragma once
#include <iostream>
#include <string>

#include "AudioManager.hpp"
#include "EventBus.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// PhoneService.hpp — 블루투스 핸즈프리 서비스
//
// 스마트폰 연동은 IVI가 AVN을 넘어서는 대표 영역이다. 관련 BT 프로파일:
//   - HFP (Hands-Free Profile) : 통화 (마이크/스피커 라우팅, 전화 제어)
//   - A2DP                     : 음악 스트리밍
//   - AVRCP                    : 곡 정보/재생 제어
//   - PBAP / MAP               : 전화번호부 / 문자 동기화
// 여기에 Android Auto / Apple CarPlay 미러링(프로젝션)까지 얹으면 요즘의
// "커넥티비티" 도메인이 된다.
//
// 통화는 오디오 우선순위가 매우 높다: 미디어를 '일시정지(Pause)'시키고,
// 통화가 끝나면 자동으로 원래 소스가 복귀(resume)해야 한다.
// 통화 음성은 에코 캔슬링/노이즈 억제(ECNR) DSP 처리를 거친다.
// ─────────────────────────────────────────────────────────────────────────────

class PhoneService {
public:
    PhoneService(EventBus& bus, AudioManager& audio) : bus_(bus), audio_(audio) {}

    void incomingCall(const std::string& caller) {
        if (inCall_) { std::cout << "[Phone] already in a call\n"; return; }
        inCall_ = true;
        caller_ = caller;
        std::cout << "[Phone] incoming call from '" << caller_
                  << "' (BT HFP) -> accepted\n";

        // 통화 우선순위: 주 오디오 소스를 일시정지시킨다.
        audio_.requestTransient(AudioSourceId::Phone, TransientType::Pause);

        Event e; e.type = EventType::CallStateChanged; e.b = true; e.str = caller_;
        bus_.publish(e);
    }

    void endCall() {
        if (!inCall_) { std::cout << "[Phone] no active call\n"; return; }
        inCall_ = false;
        std::cout << "[Phone] call ended\n";

        // 포커스 반납 → AudioManager가 주 소스에 Playing 통보 → 자동 재개
        audio_.releaseTransient(AudioSourceId::Phone);

        Event e; e.type = EventType::CallStateChanged; e.b = false;
        bus_.publish(e);
    }

    bool inCall() const { return inCall_; }

private:
    EventBus&     bus_;
    AudioManager& audio_;
    bool          inCall_ = false;
    std::string   caller_;
};
