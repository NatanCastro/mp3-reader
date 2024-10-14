#include "./byte_manipulation.h"
#include "./mp3_reader.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

  uint8_t header_buffer[4] = {0};

  if (!fread(header_buffer, 1, 4, file)) {
    printf("Could not read file: %s\nEmpty file provided\n", audio_file_path);
    return EXIT_FAILURE;
  }

  MP3Header header = create_mp3_header(header_buffer);

  print_header(&header);
  fclose(file);
  return EXIT_SUCCESS;
}
