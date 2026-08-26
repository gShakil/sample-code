## g++ 를 사용한 수동 컴파일 예제
 간단한 소스 작성 (./main.cpp)
```c++
// main.cpp

#include <iostream>
#include <vector>
#include <string>

int main()
{
    std::string s = "Hello World !";
    std::cout << s << std::endl;

    return 0;
}

```

## 컴파일, 링크 분리 작동 방법

### 1-1. g++ 명령어 입력
``` bash
g++ -std=c++17 -c main.cpp
```

### 1-2. 파일 확인
``` bash
-rw-r--r-- 1 ksw ksw   81 Aug 16 21:12 main.cpp
-rw-r--r-- 1 ksw ksw 2744 Aug 16 21:18 main.o
```

### 1-3. 링크
``` bash
g++ main.o -o main
```

### 1-4. 파일 확인
``` bash
-rwxr-xr-x 1 ksw ksw 16528 Aug 16 21:18 main
-rw-r--r-- 1 ksw ksw    81 Aug 16 21:12 main.cpp
-rw-r--r-- 1 ksw ksw  2744 Aug 16 21:18 main.o
```

### 1-5. 실행 확인
``` bash
./main
```

``` c++
Hello world !
```
---

## 컴파일, 링크 통합 작동 방법

### 2-1. g++ 명령어 입력
>-o main : 출력 이름. 없는 경우 a.out으로 출력됨
>-g : 디버그 심볼 (VS의 PDB. 실행 파일 내부에 들어감)
>-Wall -Wextra: /W4 정도의 경고 수준.
```bash
g++ -std=c++17 -Wall -Wextra -g main.cpp -o main
```

### 2-2. 파일 확인
```bash
-rwxr-xr-x 1 ksw ksw 33224 Aug 16 21:20 main
-rw-r--r-- 1 ksw ksw    81 Aug 16 21:12 main.cpp
```

### 2-3. 실행 확인
``` bash
./main
```

``` c++
Hello world !
```