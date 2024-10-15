CC=gcc
CFLAGS=-Wall -Wextra -std=c2x -pedantic -ggdb -O3
SRC=src/*.c
OUT_DIR=./bin
OUT=$(OUT_DIR)/main

AUDIO_FILE=./assets/audio_sample.mp3
ERROR_AUDIO_FILE=./assets/error.mp3

build:
	[ -d $(OUT_DIR) ] || mkdir $(OUT_DIR)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)


run:
	./main $(AUDIO_FILE)


# test reading empty file
error:
	./main $(ERROR_AUDIO_FILE)
