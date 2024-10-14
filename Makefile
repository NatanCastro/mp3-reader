CC=gcc
CFLAGS=-Wall -Wextra -std=c2x -pedantic -ggdb -O3
SRC=main.c

AUDIO_FILE=./assets/audio_sample.mp3
ERROR_AUDIO_FILE=./assets/error.mp3

build: main.c
	$(CC) $(CFLAGS) -o main $(SRC)


run: main.c
	./main $(AUDIO_FILE)


# test reading empty file
error:
	./main $(ERROR_AUDIO_FILE)
