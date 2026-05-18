CC :=g++
CFLAGS=-Wall -D_FILE_OFFSET_BITS=64-g
LDFLAGS=-static -lm
CFLAGS+=$(foreach dir, $(INC_DIRS), -l$(dir))

EXE=bin/play
OBJ_DIRS=obj
SRC_DIRS=src \
	 src/print_world
SRCS=$(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
OBJS=$(SRCS:.cpp=.o)

all:clean $(OBJS)
	${CC} -o $(EXE) $(OBJS) $(LDFLAGS)
	
clean:
	rm -f $(OBJS)
	rm -f $(EXE)

