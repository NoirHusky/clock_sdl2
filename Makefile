.SUFFIXES: .o .c

CC=gcc

CXXFLAGS=-Wall -Wextra -ggdb #-std=c++11 -ggdb # -D_GLIBCXX_DEBUG

SDLTTF=
SDLIMAGE=
SDLMIXER=
LDFLAGS=-I/usr/include/SDL2 -I./UI -I./Utils -I./json -D_REENTRANT -I/usr/local/include/SDL2 -I/usr/include/SDL2

UTILS=$(patsubst %.c,%.o,$(wildcard Utils/*.c))
UI=$(patsubst %.c,%.o,$(wildcard UI/*.c))

OBJS= $(UI) $(UTILS) $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.c,%.o,$(wildcard json/*.c))

all: game
game: $(OBJS)
	@echo Making the game...
	@$(CC) $(CXXFLAGS) -o a.out $(OBJS) -lm  $(LDFLAGS)  -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -L/usr/local/lib
	@echo Done.

.c.o:
	@echo Compiling: "$*.c"
	@$(CC) $(CXXFLAGS) -o $*.o -c $*.c $(LDFLAGS)

test: $(OBJS)
	clang-tidy  $(patsubst %.o,%.c,$(wildcard $(OBJS)))

clean:
	rm $(OBJS)
