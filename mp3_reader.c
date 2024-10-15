#include "./mp3_reader.h"
#include "./byte_manipulation.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int get_frame_bit_rate(uint8_t bit_rate_index, mpeg_version version,
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

MP3FrameHeader create_mp3_frame_header(uint8_t header_data[4]) {
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

  int bit_rate = get_frame_bit_rate(bit_rate_index, version, layer_description);
  int sampling_frequency = get_sampling_frequency(sample_rate_index, version);

  MP3FrameHeader mp3header = {
      frame_sync,     version,   layer_description,
      protected,      bit_rate,  sampling_frequency,
      padding,        private,   channel_mode,
      mode_extension, copyright, original,
      emphasis,
  };
  return mp3header;
}

char *version_str(mpeg_version version) {
  switch (version) {
  case MPEG_1:
    return "MPEG Version 1 (ISO/IEC 11172-3)";
  case MPEG_2:
    return "MPEG Version 2 (ISO/IEC 13818-3)";
  case MPEG_2_5:
    return "MPEG Version 2.5 (unofficial)";
  case MPEG_RESERVED:
    return "reserved";
  }

  return "invalid MPEG version ID";
}

char *layer_str(layer_description layer) {
  switch (layer) {
  case LAYER_1:
    return "Layer I";
  case LAYER_2:
    return "Layer II";
  case LAYER_3:
    return "Layer III";
  case LAYER_RESERVED:
    return "reserved";
  }

  return "Invalid Layer ID";
}

char *padding_str(bool padding, layer_description layer) {
  if (padding) {
    switch (layer) {
    case LAYER_RESERVED:
      return "Not Known";
    case LAYER_3:
    case LAYER_2:
      return "8 bits";
    case LAYER_1:
      return "32 bits";
      break;
    }
  } else {
    return "No Padding";
  }

  return "Not Known";
}

char *channel_mode_str(channel_mode channel_mode) {
  switch (channel_mode) {
  case STEREO:
    return "Stereo";
  case JOINT_STEREO:
    return "Joint Stereo";
  case DUAL_CHANNEL:
    return "Dual Channel";
  case SINGLE_CHANNEL:
    return "Single Channel";
  }

  return "Invalid channel mode";
}

char *mode_extension_str(uint8_t mode_extension, channel_mode channel_mode,
                         layer_description layer) {
  if (channel_mode == JOINT_STEREO) {
    switch (layer) {
    case LAYER_RESERVED:
      return "Not Known";
    case LAYER_3:
      switch (mode_extension) {
      case 0:
        return "Intensity stereo OFF, MS stereo OFF";
      case 1:
        return "Intensity stereo ON, MS stereo OFF";
      case 2:
        return "Intensity stereo OFF, MS stereo ON";
      case 3:
        return "Intensity stereo ON, MS stereo ON";
      }
    case LAYER_2:
    case LAYER_1:
      switch (mode_extension) {
      case 0:
        return "bands 4 to 31";
      case 1:
        return "bands 8 to 31";
      case 2:
        return "bands 12 to 31";
      case 3:
        return "bands 16 to 31";
      }
    }
  } else {
    return "Only Joint Stereo uses mode extension";
  }

  return "Invalid mode extension";
}

char *emphasis_str(emphasis emphasis) {
  switch (emphasis) {
  case EMPHASIS_NONE:
    return "No Emphasis";
  case MS_50_15:
    return "50/15 ms";
  case EMPHASIS_RESERVED:
    return "reserved";
  case CCIT_J17:
    return "CCIT J.17";
  }

  return "Invalid emphasis ID";
}

char *bool_str(bool boolean) { return boolean ? "Yes" : "No"; }

void print_header(MP3FrameHeader *header) {
  printf("frame sync: %x\n", header->frame_sync);
  printf("version: %s\n", version_str(header->version));
  printf("layer: %s\n", layer_str(header->layer));
  printf("is protected: %s\n", bool_str(header->is_protected));
  printf("bit rate: %ukb/s\n", header->bit_rate);
  printf("sampling frequency: %uHz\n", header->sampling_frequency);
  printf("padding: %s\n", padding_str(header->padding, header->layer));
  printf("is private: %s\n", bool_str(header->is_private));
  printf("channel mode: %s\n", channel_mode_str(header->channel_mode));
  char *mode_extension = mode_extension_str(
      header->mode_extension, header->channel_mode, header->layer);
  printf("mode extension: %s\n", mode_extension);
  printf("is copyrighted: %s\n", bool_str(header->is_copyrighted));
  printf("is original: %s\n", bool_str(header->is_original));
  printf("emphasis: %s\n", emphasis_str(header->emphasis));
}
