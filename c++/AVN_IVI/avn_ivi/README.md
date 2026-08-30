# AVN/IVI 학습용 시뮬레이터 (C++17)

전장(자동차 전자장치)에서 쓰이는 AVN/IVI 헤드유닛의 핵심 동작 원리를
콘솔 프로그램으로 축소해 놓은 학습용 예제입니다. 외부 라이브러리 없이
표준 C++17만 사용합니다.

---

## 1. AVN과 IVI란?

### AVN (Audio · Video · Navigation)
- **오디오**(라디오/USB/BT 음악) + **비디오**(DMB, 동영상, 후방카메라) +
  **내비게이션**을 하나의 장치(헤드유닛)에 통합한 차량용 멀티미디어 기기.
- 한국 완성차/부품 업계에서 오래 써 온 용어로, **"장치(하드웨어 유닛)" 관점**의
  이름입니다. 사양서, 조직명(AVN개발팀 등)에서 여전히 많이 쓰입니다.

### IVI (In-Vehicle Infotainment)
- Information + Entertainment = Infotainment. **글로벌 표준 용어**이며
  AVN 기능을 포함하는 **더 넓은 개념**입니다:
  - 커넥티비티: 블루투스, Android Auto / Apple CarPlay 프로젝션
  - 텔레매틱스/커넥티드카: 원격 제어, eCall, 실시간 교통, OTA 업데이트
  - 음성인식 비서, 앱 생태계(스토어), 결제
- 관계: **AVN ⊂ IVI**. "AVN에 통신·서비스·SW 플랫폼이 얹히며 IVI로 확장됐다"고
  이해하면 됩니다.
- 최근 흐름: 계기판(클러스터)·HUD·조수석 디스플레이까지 SoC 하나로 통합하는
  **디지털 콕핏(Digital Cockpit)**, 그리고 차량 전체 SW를 묶는 SDV
  (Software-Defined Vehicle)로 진화 중입니다.

### 전장 ECU로서의 IVI 특수성 (일반 SW와 다른 점)
| 항목 | 내용 |
|---|---|
| 차량 네트워크 | CAN/CAN-FD, LIN, 차량 이더넷으로 차량 신호 수신 (기어, 차속, 조도...) |
| 부팅 시간 | 후진 기어 후 **2초 내 후방카메라 표시** (FMVSS 111 등) → early-camera 설계 |
| 전원 관리 | OFF/ACC/IG 전원 단계, 슬립/웨이크업, 암전류(배터리 방전) 관리 |
| 법규 | 주행 중 영상 시청 제한(도로교통법 제49조), 운전자 주의분산(distraction) 규제 |
| 품질/안전 | ISO 26262 기능안전(IVI는 주로 QM~ASIL B), A-SPICE 프로세스 |
| 보안 | ISO/SAE 21434, UN R155(사이버보안)/R156(SW 업데이트) |
| 환경 | -40~85°C 동작, 진동/EMC 시험 |

### 실무 플랫폼 (이 예제가 흉내내는 실제 기술)
- **OS**: Android Automotive OS(AAOS), Automotive Grade Linux(AGL), QNX
- **IPC 미들웨어**: SOME/IP(vsomeip, CommonAPI), D-Bus, Binder/AIDL,
  AUTOSAR Adaptive `ara::com`
- **HMI**: Qt/QML, Kanzi, Android UI
- **오디오**: 오디오 DSP + 외장앰프(A2B/AVB), CarAudioService, PulseAudio/PipeWire

---

## 2. 빌드 & 실행

```bash
# MinGW g++
g++ -std=c++17 -Wall -Wextra -O2 -o avn_sim src/main.cpp
./avn_sim

# 또는 CMake (MinGW 환경)
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

실행 후 `demo` 를 입력하면 아래 시나리오가 자동 시연됩니다.
`help` 로 전체 명령을 볼 수 있습니다.

### 데모가 보여주는 것
1. `ign on` — 전원 상태 머신, 웰컴/라스트 모드 복원
2. `media play` — 소스 선택 → 오디오 포커스 획득 → 재생
3. `speed 80` — CAN 차속 수신 → 속도 감응 볼륨(SDVC) 자동 보정
4. `nav tick` — 내비 TTS가 음악을 **덕킹**(볼륨 30%)했다가 복원
5. `call in` / `call end` — 전화가 음악을 **일시정지**, 종료 후 **자동 재개**
6. `gear r` — 후진 기어 → **후방카메라 강제 전환** (안전 최우선)
7. `video on` (주행 중) — **주행 규제**: 화면 차단, 소리만 유지 → 정차+P단이 되면 자동 해제
8. `radio` → `ign off` → `ign acc` — **라스트 모드** 저장/복원
9. `can 100 02 03 20 03 00 00 00 00` — 원시 CAN 프레임을 DBC 규칙으로 디코딩

---

## 3. 아키텍처

실제 IVI의 계층 구조를 그대로 축소했습니다. 화살표는 의존 방향입니다.

```
┌──────────────────────────────────────────────────────┐
│  HMI 계층            Hmi (표시/입력만, 로직 없음)      │
├──────────────────────────────────────────────────────┤
│  서비스 계층         MediaPlayer  RadioTuner          │
│                     NavigationService  PhoneService   │
│                     DisplayManager  PowerManager      │
├──────────────────────────────────────────────────────┤
│  플랫폼(미들웨어)    AudioManager (오디오 중재)        │
│                     EventBus (pub/sub ≒ IPC)          │
├──────────────────────────────────────────────────────┤
│  차량 인터페이스     CanBusSimulator (CAN 신호 수신)   │
└──────────────────────────────────────────────────────┘
```

### 파일별 학습 포인트
| 파일 | 개념 | 실무 대응 |
|---|---|---|
| `EventBus.hpp` | 이벤트 기반 아키텍처, pub/sub | D-Bus, SOME/IP, Binder |
| `CanBusSimulator.hpp` | 차량 신호 수신, DBC 디코딩, dedupe | CAN 게이트웨이, 신호 SW |
| `PowerManager.hpp` | 전원 상태 머신, 라스트 모드(NVM) | 전원/NM 관리, early-RVC |
| `AudioManager.hpp` | **오디오 중재**: 소스 전환, 덕킹, 일시정지, SDVC | CarAudioService, GENIVI AM |
| `MediaPlayer.hpp` | 포커스에 반응하는 소스, 자동 재개 | 미디어 스택, A2DP/AVRCP |
| `RadioTuner.hpp` | 방송형 소스(일시정지 불가→mute), 프리셋/seek | 튜너 IC, RDS/DAB |
| `NavigationService.hpp` | TBT 안내, TTS ↔ 오디오 중재 연동 | 내비 엔진, GNSS+DR |
| `PhoneService.hpp` | BT HFP 통화, 최고 우선 오디오 | BT 스택(HFP/PBAP) |
| `DisplayManager.hpp` | **안전/규제 인터록**: 후방카메라, 주행 중 영상 제한 | 화면 정책 관리자 |
| `Hmi.hpp` | 표시/입력과 로직의 분리 | QML/Kanzi/AAOS UI |

### 핵심 설계 패턴 3가지
1. **오디오 포커스**: 소스는 스스로 재생을 결정하지 않는다. AudioManager에게
   요청하고, `onAudioFocus()` 콜백이 시키는 대로 Playing/Ducked/Paused/Stopped
   상태를 따른다. (Android AudioFocus와 동일한 사상)
2. **base vs shown 화면 분리**: 사용자가 고른 화면(base)과 실제 표시 화면(shown)을
   분리하면, 규제(기어 R, 주행 중 영상)가 풀렸을 때 자연스럽게 원래 화면으로
   복귀한다. 상태를 덮어쓰지 않고 "계산"하는 것이 포인트.
3. **신호 → 이벤트 → 반응**: 차량 신호(CAN)는 값이 바뀔 때만 이벤트로 발행되고,
   여러 서비스(SDVC, 화면 규칙)가 각자 구독해 반응한다. 모듈 간 직접 호출 없음.

---

## 4. 이 예제가 단순화한 것 (실차와의 차이)
- 단일 프로세스/단일 스레드 (실차: 프로세스 분리 + IPC + 워치독)
- 오디오/영상 실출력 없음 (실차: DSP 믹싱, HW 코덱, 디스플레이 파이프라인)
- 진단(UDS/DoIP), OTA, 슬립/웨이크업, NVM 실제 저장 없음
- 오디오 존(zone) 없음 (실차: 앞좌석/뒷좌석 독립 오디오 등 멀티존)

## 5. 다음 학습 키워드
CAN/DBC → SOME/IP & vsomeip → AUTOSAR(Classic/Adaptive) → Android Automotive OS
(Car API, VHAL, CarAudioService) → Qt/QML HMI → ISO 26262 / A-SPICE →
UN R155/R156 → 디지털 콕핏(하이퍼바이저로 클러스터+IVI 통합) → SDV
