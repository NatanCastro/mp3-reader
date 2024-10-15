#include "./byte_manipulation.h"
#include "./mp3_reader.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ID3V2_TAG_SIZE 10

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please provide the path to the MP3 audio file\n");
    exit(EXIT_FAILURE);
  }

  char *audio_file_path = argv[1];
  FILE *file = fopen(audio_file_path, "rb");

  if (file == NULL) {
    printf("Could not open file: %s\n", audio_file_path);
    return EXIT_FAILURE;
  }

  uint8_t id3v2_tag[ID3V2_TAG_SIZE] = {0};

  if (fread(&id3v2_tag, 1, ID3V2_TAG_SIZE, file)) {
    if (id3v2_tag[0] != 'I' || id3v2_tag[1] != 'D' || id3v2_tag[2] != '3') {
      rewind(file);
      printf("No id3v2 tag\n");
      goto jump_ID3V2;
    }

    printf("version: %u.%u\n", id3v2_tag[3], id3v2_tag[4]);
    printf("flags: %x\n", id3v2_tag[5]);
    int length = (id3v2_tag[6] << 21) | (id3v2_tag[7] << 14) |
                 (id3v2_tag[8] << 7) | (id3v2_tag[9]);
    printf("tag length: %u\n", length);
    printf("tag length: %x\n", length);
    fseek(file, 10 + length, SEEK_SET);
  }
jump_ID3V2:
  printf("\n");

  uint8_t header_buffer[4] = {0};

  if (fread(header_buffer, 1, 4, file) != 4) {
    printf("Could not read file: %s\nEmpty file provided\n", audio_file_path);
    return EXIT_FAILURE;
  }

  mp3_frame_header frame_header = create_mp3_frame_header(header_buffer);

  print_header(&frame_header, NULL);

  fclose(file);
  return EXIT_SUCCESS;
}
