#pragma once
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "AudioManager.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// RadioTuner.hpp — FM 라디오 튜너 서비스
//
// 라디오는 AVN의 뿌리다 (A/V/N의 'A'). 실차 구성:
//   안테나 → 튜너 IC(RF 복조) → 오디오 경로.  요즘은 DAB/DAB+(디지털 라디오),
//   HD Radio, RDS(방송국명/교통정보 문자 데이터) 지원이 일반적이다.
//
// 방송의 특성상 "일시정지"가 없다 — 포커스를 잃으면 수신은 유지한 채
// 음소거(mute)만 한다. (전화가 끝나면 그 시점의 방송이 바로 들려야 하므로)
// 이 차이를 MediaPlayer(일시정지/재개)와 비교해 보자.
// ─────────────────────────────────────────────────────────────────────────────

class RadioTuner : public IAudioFocusListener {
public:
    explicit RadioTuner(AudioManager& audio) : audio_(audio) {
        audio_.registerSource(AudioSourceId::Radio, this);
    }

    // ── 사용자(HMI) 조작 ────────────────────────────────────────────────────
    void on() { audio_.selectMainSource(AudioSourceId::Radio); }

    void tune(double mhz) {
        if (mhz < 87.5) mhz = 87.5;
        if (mhz > 107.9) mhz = 107.9;
        freq_ = mhz;
        std::cout << "[Radio] tuned " << stationText() << "\n";
    }

    void seek() {
        // 실제 seek은 신호 세기(RSSI) 기준 스캔. 여기서는 다음 프리셋으로 이동.
        for (const auto& p : presets_) {
            if (p.mhz > freq_ + 0.01) { tune(p.mhz); return; }
        }
        tune(presets_.front().mhz);  // 밴드 끝 → 처음으로 랩어라운드
    }

    std::string stationText() const {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "FM %.1f MHz", freq_);
        std::string s = buf;
        for (const auto& p : presets_) {
            if (p.mhz == freq_) { s += std::string(" (") + p.name + ")"; break; }
        }
        if (muted_) s += " [MUTED]";
        return s;
    }

    // ── AudioManager로부터의 포커스 통보 ───────────────────────────────────
    void onAudioFocus(FocusState st) override {
        switch (st) {
            case FocusState::Playing:
                if (muted_) {
                    muted_ = false;
                    std::cout << "[Radio] unmuted > " << stationText() << "\n";
                } else if (!active_) {
                    active_ = true;
                    std::cout << "[Radio] on > " << stationText() << "\n";
                }
                break;
            case FocusState::Ducked:
                if (active_) std::cout << "[Radio] playing at ducked volume\n";
                break;
            case FocusState::Paused:
                muted_ = true;  // 방송은 멈출 수 없다 → 음소거로 처리
                std::cout << "[Radio] muted (broadcast keeps receiving; cannot pause)\n";
                break;
            case FocusState::Stopped:
                active_ = false;
                muted_  = false;
                std::cout << "[Radio] off (source deselected)\n";
                break;
        }
    }

private:
    struct Preset { double mhz; const char* name; };

    AudioManager& audio_;
    std::vector<Preset> presets_{
        {89.1, "KBS CoolFM"},
        {91.9, "MBC FM4U"},
        {93.1, "KBS ClassicFM"},
        {103.5, "SBS PowerFM"},
    };
    double freq_   = 89.1;
    bool   active_ = false;
    bool   muted_  = false;
};
