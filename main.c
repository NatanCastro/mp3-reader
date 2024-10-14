#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  REST_OF_BITS,
  ONE_BIT = 0x01,
  TWO_BITS = 0x03,
  THREE_BITS = 0x05,
  FOUR_BITS = 0x07,
  FIVE_BITS = 0x09,
} num_bits;

typedef enum {
  MPEG_2_5,
  MPEG_RESERVED,
  MPEG_2,
  MPEG_1,
} mpeg_version;

typedef enum {
  LAYER_RESERVED,
  LAYER_3,
  LAYER_2,
  LAYER_1,
} layer_description;

typedef enum {
  STEREO,
  JOINT_STEREO,
  DUAL_CHANNEL,
  SINGLE_CHANNEL,
} channel_mode;

typedef enum {
  EMPHASIS_NONE,
  MS_50_15,
  EMPHASIS_RESERVED,
  CCIT_J17,
} emphasis;

typedef struct {
  uint16_t frame_sync;
  mpeg_version version;
  layer_description layer;
  bool is_protected; // Indicates if CRC is used.
  int bit_rate;
  int sampling_frequency;
  uint8_t padding;
  bool is_private;
  channel_mode channel_mode;
  uint8_t mode_extension;
  bool is_copyrighted;
  bool is_original;
  emphasis emphasis;
} MP3Header;

uint8_t bits_from_u8(uint8_t data, int offset, num_bits number_bits) {
  uint8_t result = data >> offset;
  if (number_bits != REST_OF_BITS)
    result &= number_bits;
  return result;
}

uint16_t bits_from_u16(uint16_t data, int offset, num_bits number_bits) {
  uint16_t result = data >> offset;
  if (number_bits != REST_OF_BITS)
    result &= number_bits;
  return result;
}

int get_bit_rate(uint8_t bit_rate_index, mpeg_version version,
                 layer_description layer) {

  uint8_t version_layer = (version << 2) | layer;

  // Handle special cases for "free" and "bad" bit rates
  if (bit_rate_index == 0b0000) {
    return 0; // Free format bit rate
  }
  if (bit_rate_index == 0b1111) {
    return -1; // Bad format bit rate
  }

  // Define bitrate tables for different version_layer values
  const int bit_rate_table_v1_l1[] = {0,   32,  64,  96,  128, 160, 192, 224,
                                      256, 288, 320, 352, 384, 416, 448};
  const int bit_rate_table_v1_l2[] = {0,   32,  48,  56,  64,  80,  96, 112,
                                      128, 160, 192, 224, 256, 320, 384};
  const int bit_rate_table_v1_l3[] = {0,   32,  40,  48,  56,  64,  80, 96,
                                      112, 128, 160, 192, 224, 256, 320};
  const int bit_rate_table_v2_l1[] = {0,   32,  48,  56,  64,  80,  96, 112,
                                      128, 144, 160, 176, 192, 224, 256};
  const int bit_rate_table_v2_l2_l3[] = {0,  8,  16, 24,  32,  40,  48, 56,
                                         64, 80, 96, 112, 128, 144, 160};

  // Select appropriate bit rate table based on version_layer
  const int *bit_rate_table = NULL;
  int table_size = 0;

  switch (version_layer) {
  case 0b1111: // MPEG Version 1, Layer I
    bit_rate_table = bit_rate_table_v1_l1;
    table_size = sizeof(bit_rate_table_v1_l1) / sizeof(bit_rate_table_v1_l1[0]);
    break;
  case 0b1110: // MPEG Version 1, Layer II
    bit_rate_table = bit_rate_table_v1_l2;
    table_size = sizeof(bit_rate_table_v1_l2) / sizeof(bit_rate_table_v1_l2[0]);
    break;
  case 0b1101: // MPEG Version 1, Layer III
    bit_rate_table = bit_rate_table_v1_l3;
    table_size = sizeof(bit_rate_table_v1_l3) / sizeof(bit_rate_table_v1_l3[0]);
    break;
  case 0b1011: // MPEG Version 2, Layer I
  case 0b0011: // MPEG Version 2.5, Layer I
    bit_rate_table = bit_rate_table_v2_l1;
    table_size = sizeof(bit_rate_table_v2_l1) / sizeof(bit_rate_table_v2_l1[0]);
    break;
  case 0b1010: // MPEG Version 2, Layer II
  case 0b1001: // MPEG Version 2, Layer III
  case 0b0010: // MPEG Version 2.5, Layer II
  case 0b0001: // MPEG Version 2.5, Layer III
    bit_rate_table = bit_rate_table_v2_l2_l3;
    table_size =
        sizeof(bit_rate_table_v2_l2_l3) / sizeof(bit_rate_table_v2_l2_l3[0]);
    break;
  default:
    return -1; // Invalid version_layer
  }

  // Return bitrate if bit_rate_index is valid
  if (bit_rate_index - 1 < table_size) {
    return bit_rate_table[bit_rate_index - 1];
  }

  return -1; // Invalid bit_rate_index
}

int get_sampling_frequency(uint8_t sampling_freq_index, mpeg_version version) {
  // Define sampling frequency tables based on MPEG versions
  const int sampling_freq_mpeg1[] = {44100, 48000, 32000,
                                     -1}; // -1 for reserved
  const int sampling_freq_mpeg2[] = {22050, 24000, 16000,
                                     -1}; // -1 for reserved
  const int sampling_freq_mpeg25[] = {11025, 12000, 8000,
                                      -1}; // -1 for reserved

  const int *sampling_freq_table = NULL;

  // Select the appropriate table based on the MPEG version
  switch (version) {
  case 0b11: // MPEG Version 1
    sampling_freq_table = sampling_freq_mpeg1;
    break;
  case 0b10: // MPEG Version 2
    sampling_freq_table = sampling_freq_mpeg2;
    break;
  case 0b00: // MPEG Version 2.5
    sampling_freq_table = sampling_freq_mpeg25;
    break;
  default:
    return -1; // Invalid version
  }

  // Return the sampling frequency based on the index
  if (sampling_freq_index < 4) {
    return sampling_freq_table[sampling_freq_index];
  }

  return -1; // Invalid sampling frequency index
}

MP3Header create_mp3_header(uint8_t header_data[4]) {
  uint16_t first_two_bytes = (header_data[0] << 8) | header_data[1];
  uint16_t frame_sync = bits_from_u16(first_two_bytes, 5, REST_OF_BITS);
  uint8_t version = bits_from_u16(first_two_bytes, 3, TWO_BITS);
  uint8_t layer_description = bits_from_u16(first_two_bytes, 1, TWO_BITS);
  uint8_t protected = bits_from_u16(first_two_bytes, 0, ONE_BIT);
  uint8_t bit_rate_index = bits_from_u8(header_data[2], 4, REST_OF_BITS);
  uint8_t sample_rate_index = bits_from_u8(header_data[2], 2, TWO_BITS);
  uint8_t padding = bits_from_u8(header_data[2], 1, ONE_BIT);
  uint8_t private = bits_from_u8(header_data[2], 0, ONE_BIT);
  uint8_t channel_mode = bits_from_u8(header_data[3], 6, REST_OF_BITS);
  uint8_t mode_extension = bits_from_u8(header_data[3], 4, TWO_BITS);
  uint8_t copyright = bits_from_u8(header_data[3], 3, ONE_BIT);
  uint8_t original = bits_from_u8(header_data[3], 2, ONE_BIT);
  uint8_t emphasis = bits_from_u8(header_data[3], 0, TWO_BITS);

  int bit_rate = get_bit_rate(bit_rate_index, version, layer_description);
  int sampling_frequency = get_sampling_frequency(sample_rate_index, version);

  MP3Header mp3header = {
      frame_sync,     version,   layer_description,
      protected,      bit_rate,  sampling_frequency,
      padding,        private,   channel_mode,
      mode_extension, copyright, original,
      emphasis,
  };
  return mp3header;
}

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
  unsigned long read_file_error = fread(header_buffer, 1, 4, file);
  if (read_file_error == 0) {
    printf("Could not read file: %s\nEmpty file provided\n", audio_file_path);
    return EXIT_FAILURE;
  }

  MP3Header header = create_mp3_header(header_buffer);
  printf("%x\n", header.layer);

  fclose(file);
  return EXIT_SUCCESS;
}
