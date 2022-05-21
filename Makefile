CC     := g++

ifeq ($(OS),Windows_NT)
	CFLAGS := -std=c++17 -g -I inc/
	LIBS   := curl
	EXE    := out.exe
else
	CFLAGS := -std=c++17 -g -I inc/ -fsanitize=address
	LIBS   := asan
	EXE    := a.out
endif


SRCS   := $(wildcard src/*.cpp)
SRCS   := $(filter-out src/test.cpp, $(SRCS))
OBJS   := $(patsubst src/%.cpp,bin/%.o,$(SRCS))
DEPS   := $(patsubst src/%.cpp,bin/%.d,$(SRCS))
DIRS   := src inc bin

all: $(DIRS) $(EXE)

debug: CFLAGS += -D DEBUG
debug: $(DIRS) $(EXE)

$(DIRS):
	mkdir -p $@

$(EXE): $(OBJS)
	$(CC) -o $@ $^ -l $(LIBS)

bin/%.o: src/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $< -MMD -MF $(@:.o=.d)

-include $(DEPS)

clean:
	rm -rf bin *~ *.o *.out $(EXE)
