#CC: c++ 컴파일, c 컴파일은 gcc
#CFLAGS: Wall: 모호한 코드에 대하여 경고 출력, -g: 디버그 정보 추가, -O: 컴파일 최적화
#TARGET: bin 파일 경로, 생성될 실행 파일 이름 저장
#SRC_DIRS: src 파일이 들어있는 경로.
#SRCS: SRC_DIRS로 들어가서 모든 *.cpp 정보를 담음. foreach는 반복문 매크로로 foreach dir, $(SRC_DIRS)에 있는 directory를 차례대로 들어감. 이후 *.cpp 확장자의 파일 정보를 모두 SRC에 담음
#OBJS: object 파일의 정보. 여기서는 (SRCS.cpp=.o)라는 매크로 이용. SRCS에 들어있는 모든 .cpp확장자를 .o로 변환해서 OBJS에 저장
#ex) SRCS에 main.cpp가 담겨있다면 OBJS에 main.o를 저장
#all: make all을 수행하면 동작하는 명령어.
#clean: make clean을 수행하면 동작하는 명령어

CC :=g++                      
CFLAGS=-Wall -g                
TARGET=bin/helloworld          
SRC_DIRS=src                   

SRCS=$(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))      
OBJS=$(SRCS:.cpp=.o)

all:clean $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)
	
clean:
	rm -f $(OBJS)
	rm -f $(EXE)