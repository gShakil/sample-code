#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "AudioManager.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// MediaPlayer.hpp — USB 미디어 재생 서비스
//
// 실제 AVN의 미디어 스택:
//   USB 마운트 → 파일 인덱싱(메타데이터 DB 구축) → 디코딩(대부분 HW 코덱)
//   → 오디오 라우팅(DSP/앰프) → 재생 상태를 HMI에 통지
//   블루투스 오디오는 A2DP(스트리밍) + AVRCP(곡 정보/리모컨) 프로파일 사용.
//
// 여기서 볼 핵심 개념은 "오디오 포커스에 반응하는 소스" 패턴이다:
//   - Paused  통보 → 재생 멈춤 + '원래 재생 중이었는지' 기억
//   - Playing 통보 → 전화 끊김 등으로 포커스 복귀 시 자동 재생 재개(resume)
//   - Ducked  통보 → 계속 재생 (볼륨은 AudioManager가 낮춤)
//   - Stopped 통보 → 다른 소스로 전환됨
// ─────────────────────────────────────────────────────────────────────────────

class MediaPlayer : public IAudioFocusListener {
public:
    explicit MediaPlayer(AudioManager& audio) : audio_(audio) {
        audio_.registerSource(AudioSourceId::UsbMedia, this);
    }

    // ── 사용자(HMI) 조작 ────────────────────────────────────────────────────
    void play() {
        if (audio_.mainSource() != AudioSourceId::UsbMedia) {
            audio_.selectMainSource(AudioSourceId::UsbMedia);  // 소스 전환 → 콜백에서 재생 시작
        } else if (!playing_) {
            playing_ = true;
            logNow("play");
        } else {
            std::cout << "[Media] already playing\n";
        }
    }

    void pause() {
        if (!playing_) { std::cout << "[Media] already paused\n"; return; }
        playing_ = false;
        wasPlaying_ = false;  // 사용자가 직접 멈췄으므로 포커스 복귀 시 자동 재개 안 함
        std::cout << "[Media] paused (by user)\n";
    }

    void next() { step(+1); }
    void prev() { step(-1); }

    bool playing() const { return playing_; }

    std::string nowPlaying() const {
        const Track& t = tracks_[index_];
        return "(" + std::to_string(index_ + 1) + "/" + std::to_string(tracks_.size())
             + ") " + t.title + " - " + t.artist;
    }

    // ── AudioManager로부터의 포커스 통보 ───────────────────────────────────
    void onAudioFocus(FocusState st) override {
        switch (st) {
            case FocusState::Playing:
                if (prev_ == FocusState::Paused) {
                    if (wasPlaying_) { playing_ = true; logNow("auto-resume"); }
                } else if (prev_ == FocusState::Stopped) {
                    playing_ = true;   // 소스로 선택됨 → 재생 시작
                    logNow("start");
                }
                // Ducked -> Playing 은 볼륨만 복귀 (AudioManager가 로그)
                break;
            case FocusState::Ducked:
                if (playing_) std::cout << "[Media] keep playing at ducked volume\n";
                break;
            case FocusState::Paused:
                wasPlaying_ = playing_;
                if (playing_) {
                    playing_ = false;
                    std::cout << "[Media] paused by focus loss (will auto-resume)\n";
                }
                break;
            case FocusState::Stopped:
                playing_ = false;
                wasPlaying_ = false;
                std::cout << "[Media] stopped (source deselected)\n";
                break;
        }
        prev_ = st;
    }

private:
    void step(int delta) {
        if (audio_.mainSource() != AudioSourceId::UsbMedia) {
            std::cout << "[Media] not the active source (try 'media play')\n";
            return;
        }
        int n = static_cast<int>(tracks_.size());
        index_ = (index_ + delta % n + n) % n;
        logNow(delta > 0 ? "next" : "prev");
    }

    void logNow(const char* action) {
        std::cout << "[Media] " << action << " > " << nowPlaying() << "\n";
    }

    struct Track { const char* title; const char* artist; };

    AudioManager& audio_;
    std::vector<Track> tracks_{
        {"Canon in D",    "Pachelbel"},
        {"Clair de Lune", "Debussy"},
        {"Arirang",       "Traditional"},
    };
    int  index_      = 0;
    bool playing_    = false;
    bool wasPlaying_ = false;                    // 포커스 뺏기기 직전 재생 여부
    FocusState prev_ = FocusState::Stopped;      // 직전 포커스 상태
};
