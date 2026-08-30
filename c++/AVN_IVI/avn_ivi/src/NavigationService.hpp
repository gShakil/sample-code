#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "AudioManager.hpp"
#include "EventBus.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// NavigationService.hpp — 내비게이션 서비스 (AVN의 'N')
//
// 실제 내비 스택 구성 요소:
//   - 측위: GNSS(GPS) + 추측항법(DR: 자이로/차속 펄스로 터널 안 위치 보정)
//   - 지도 DB + 경로 탐색(A* 등) + 경로 이탈 재탐색
//   - 안내: 턴바이턴(TBT) 이벤트 → TTS 음성 합성 → 오디오 중재 요청
//   - 교통정보: TPEG/TMC 수신, 요즘은 클라우드 실시간 교통
//   - 클러스터/HUD에 TBT 정보 전달 (ECU 간 통신)
//
// 이 예제의 핵심은 "안내 음성이 나가는 동안 주 오디오를 덕킹"하는 흐름이다:
//   tick() → AudioManager에 Duck 포커스 요청 → TTS 발화(이벤트 발행) → 해제
// ─────────────────────────────────────────────────────────────────────────────

class NavigationService {
public:
    NavigationService(EventBus& bus, AudioManager& audio)
        : bus_(bus), audio_(audio) {}

    void go(const std::string& dest) {
        dest_   = dest;
        active_ = true;
        index_  = 0;
        // 실차라면 지도 DB에서 경로 탐색. 여기서는 고정 시나리오.
        steps_ = {
            "In 500 m, turn right onto Teheran-ro",
            "In 300 m, turn left onto Gangnam-daero",
            "In 1 km, enter the expressway",
            "You have arrived at: " + dest_,
        };
        std::cout << "[Nav] route calculated to '" << dest_ << "' ("
                  << steps_.size() - 1 << " maneuvers). advance with 'nav tick'\n";
    }

    // 주행이 진행되어 다음 안내 지점에 도달했다고 가정 (실차: 측위가 트리거)
    void tick() {
        if (!active_) {
            std::cout << "[Nav] no active route (use 'nav go <destination>')\n";
            return;
        }
        // 1) 안내음성 우선권 확보 → 주 오디오 덕킹
        audio_.requestTransient(AudioSourceId::NavTts, TransientType::Duck);

        // 2) TTS 발화 — HMI(스피커)가 이 이벤트를 받아 출력한다
        Event e; e.type = EventType::NavGuidance; e.str = steps_[index_];
        bus_.publish(e);

        // 3) 발화 종료 → 포커스 반납 → 주 오디오 볼륨 복귀
        audio_.releaseTransient(AudioSourceId::NavTts);

        if (++index_ >= steps_.size()) {
            active_ = false;
            std::cout << "[Nav] guidance finished (arrived)\n";
        }
    }

    void stop() {
        if (!active_) return;
        active_ = false;
        std::cout << "[Nav] guidance cancelled\n";
    }

    bool active() const { return active_; }
    const std::string& destination() const { return dest_; }

private:
    EventBus&     bus_;
    AudioManager& audio_;
    std::vector<std::string> steps_;
    std::string   dest_;
    size_t        index_  = 0;
    bool          active_ = false;
};
