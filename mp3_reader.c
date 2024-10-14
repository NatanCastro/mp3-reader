#include "./mp3_reader.h"
#include "./byte_manipulation.h"
#include <stddef.h>
#include <stdint.h>

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
  uint16_t first_two_bytes = join_two_u8(&header_data[0], &header_data[1]);
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
