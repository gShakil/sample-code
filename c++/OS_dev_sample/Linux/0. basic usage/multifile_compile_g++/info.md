## Wall

## `Wall`

**이름과 달리 "모든 경고"가 아님.** "거의 항상 실수인 것들" 묶음입니다. MSVC의 `/W3`~`/W4` 사이 정도.

여기 포함되는 것들:

- `Wreturn-type` — 값을 반환해야 하는 함수에서 `return` 누락 (MSVC C4715)
- `Wunused-variable` — 안 쓰는 지역 변수
- `Wuninitialized` — 초기화 안 된 변수 사용 (단, **`O1` 이상에서만 제대로 동작**합니다. 최적화 패스의 데이터 흐름 분석 결과를 쓰기 때문)
- `Wsign-compare` — signed/unsigned 비교 (MSVC C4018). `for (int i = 0; i < vec.size(); ++i)` 가 여기 걸립니다
- `Wswitch` — enum switch에서 빠진 케이스
- `Wformat` — `printf` 서식 문자열과 인자 타입 불일치

## Wextra

## `Wextra`

- `Wall`에 안 들어간 추가 경고. 둘을 합치면 대략 MSVC `/W4` 수준입니다.
- `Wunused-parameter` — 안 쓰는 함수 매개변수 (MSVC C4100)
- `Wmissing-field-initializers` — 구조체 초기화 시 빠진 멤버
- `Wempty-body` — `if (x);` 같은 빈 본문
- `Wtype-limits` — 항상 참/거짓인 비교 (`unsigned x; if (x >= 0)`)
- **`Wall -Wextra`로도 안 켜지는 것들**이 꽤 있습니다. 레거시 코드 정리하실 때 유용한 것들:

```
g++ -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Wold-style-cast ...
```

- `Wshadow` — 바깥 스코프 변수를 가리는 선언
- `Wconversion` — 암묵적 축소 변환 (`double` → `int` 등). 처음 켜면 경고가 폭발하지만 실제 버그를 잘 잡습니다
- `Wpedantic` — 비표준 확장 사용 경고

그리고 경고는 **기본적으로 빌드를 막지 않습니다.** 막으려면 `-Werror` (MSVC `/WX`).

### g

## `g`

디버그 정보(DWARF)를 생성합니다. MSVC와 결정적으로 다른 점: **별도 PDB 파일이 아니라 실행 파일 안에 섹션으로 들어갑니다.** 그래서 `-g`로 빌드하면 바이너리 크기가 몇 배로 커집니다.

- `g` 없이 빌드하면 gdb에서 소스 줄도, 변수 이름도 못 봅니다 (`??` 만 나옴)
- `g3` — 매크로 정의까지 포함해서 gdb에서 매크로를 평가할 수 있음
- **`g`는 최적화와 무관합니다.** MSVC의 Debug 구성처럼 "디버그 = 최적화 끔"이 자동으로 엮이지 않습니다. `O` 옵션을 아예 안 주면 기본이 `O0`(최적화 없음)이라 지금은 결과적으로 VS Debug 빌드와 비슷하지만, `O2 -g`도 완전히 유효한 조합입니다 (릴리스 바이너리 + 심볼)

### IInclude

## `Iinclude` ← 핵심

### 하는 일

헤더 검색 경로에 `include` 디렉터리를 **추가**합니다. MSVC의 `/I` — 프로젝트 속성의 **"추가 포함 디렉터리"** 와 정확히 같은 것입니다.

- `Iinclude`, `I include`, `I./include` 모두 동일하게 동작합니다. 붙여 쓰는 게 관례입니다.

### 왜 지금 필요한가

파일 배치가 이렇습니다:

```
calcdemo/
├── include/calc.h
└── src/
    ├── main.cpp   →  #include "calc.h"
    └── calc.cpp   →  #include "calc.h"
```

`src/main.cpp`가 `"calc.h"`를 찾을 때 GCC의 탐색 순서는:

1. **`#include`를 쓴 파일이 있는 디렉터리** → `src/` → 없음
2. `I` 로 준 디렉터리들 → `include/` → **찾음** ✅
3. 시스템 표준 디렉터리 (`/usr/include` 등)
- `Iinclude`를 빼면 1번에서 실패하고 3번까지 가서 못 찾은 뒤:

```
src/main.cpp:2:10: fatal error: calc.h: No such file or directory
```

MSVC의 `C1083`과 같은 상황입니다. 직접 확인해 보세요:

```
g++ -std=c++17 src/main.cpp src/calc.cpp -o calc
```

### `""` 와 `<>` 의 차이

| 형태 | 탐색 순서 |
| --- | --- |
| `#include "calc.h"` | ① 현재 파일의 디렉터리 → ② `-iquote` → ③ `-I` → ④ `-isystem` → ⑤ 시스템 |
| `#include <calc.h>` | ③ `-I` → ④ `-isystem` → ⑤ 시스템 (**①②를 건너뜀**) |
- `Iinclude`가 있으면 `#include <calc.h>` 로 써도 잘 찾아집니다. 관례상 **내 프로젝트 헤더는 `""`, 외부/시스템 헤더는 `<>`** 로 씁니다.

### ⚠️ 함정 1 — 상대 경로의 기준은 "현재 작업 디렉터리"

- `Iinclude`의 `include`는 **소스 파일 위치가 아니라 `g++`를 실행한 디렉터리 기준**입니다. 그래서 이건 실패합니다:

```
cd src && g++ -std=c++17 -Iinclude main.cpp calc.cpp -o calc
```

`src/include/` 를 찾으려 하기 때문입니다. 스크립트에서는 절대 경로를 쓰거나, 항상 프로젝트 루트에서 실행하는 규칙을 지키세요. CMake가 이 문제를 아예 없애 주는데, `target_include_directories(calc PRIVATE include)` 를 **절대 경로**(`-I/home/you/dev/calcdemo/include`)로 변환해서 넘기기 때문입니다.

```
cmake --build build -v
```

로 실제 전달되는 명령줄을 확인해 볼 수 있습니다.

### ⚠️ 함정 2 — 여러 개 줄 때는 순서가 먼저 이기는 구조

```
g++ -Iinclude -Ivendor/include -Ithird_party ...
```

같은 이름의 헤더가 여러 곳에 있으면 **먼저 적은 `-I`가 이깁니다.** 엉뚱한 버전의 헤더가 잡히는 사고가 여기서 납니다.

### `I` 대신 `isystem` 을 써야 할 때

외부 라이브러리 헤더에는 `-I` 대신 `-isystem`을 쓰세요. **그 헤더에서 발생하는 경고를 억제**해 줍니다. 안 그러면 `-Wall -Wextra`를 켠 순간 남의 라이브러리 경고가 내 코드 경고를 덮어버립니다.

```
g++ -Wall -Wextra -Iinclude -isystem /opt/boost/include src/main.cpp -o calc
```

### 현재 검색 경로 전체 보기

```
echo | g++ -x c++ -E -v - 2>&1
```

출력 중간의 `#include <...> search starts here:` 아래 목록이 실제 탐색 순서입니다.

## -o calc

## `o calc`

출력 파일 이름. 생략하면 **`a.out`** 이 됩니다 (윈도우에서 `main.exe`가 나오는 것과 달리 소스 이름을 따라가지 않습니다).

- `c` 와 같이 쓰면 오브젝트 파일 이름: `g++ -c calc.cpp -o build/calc.o`
- 리눅스 실행 파일에는 **확장자가 없습니다.** 실행 가능 여부는 파일 이름이 아니라 퍼미션의 `x` 비트로 결정됩니다 (`ls -l`로 확인)

---

## 명시하지 않았지만 적용된 기본값

| 항목 | 기본값 | 비고 |
| --- | --- | --- |
| 최적화 | `-O0` | 최적화 없음. 릴리스에는 `-O2` |
| 링크 | `libstdc++`, `libm`, `libc` 자동 | `g++`라서 |
| 예외/RTTI | 켜짐 | `-fno-exceptions`, `-fno-rtti`로 끌 수 있음 |
| 매크로 | `NDEBUG` **정의 안 됨** | 그래서 `assert()`가 살아 있음 |