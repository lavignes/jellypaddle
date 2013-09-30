CC = gcc
CFLAGS = -DGLEW_STATIC -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -std=gnu99
LDFLAGS = -lglut -lGLEW -lGL -lm

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

BIN = jellypaddle

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(BIN) 

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

play: $(BIN)
	# This runs the game at max-fps
	__GL_SYNC_TO_VBLANK=0 vblank_mode=0 ./$(BIN)

clean:
	rm -f $(OBJS) $(BIN)
