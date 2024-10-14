#include <stdbool.h>
#include <stdint.h>

#ifndef MP3_READER_H_
#define MP3_READER_H_

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

int get_bit_rate(uint8_t, mpeg_version, layer_description);
int get_sampling_frequency(uint8_t, mpeg_version);

MP3Header create_mp3_header(uint8_t[4]);

#endif // !MP3_READER_H_
