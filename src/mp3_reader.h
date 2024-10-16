#include "dlinked_list.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
} mp3_frame_header;

typedef struct {
  mp3_frame_header header;
  int data;
} MP3Frame;

typedef struct {
  dll_t frames;
} mp3_data;

int get_bit_rate(uint8_t, mpeg_version, layer_description);
int get_sampling_frequency(uint8_t, mpeg_version);

mp3_frame_header create_mp3_frame_header(uint8_t[4]);
mp3_data read_mp3_file(FILE *);

void print_header(mp3_frame_header *, FILE *);
char *version_str(mpeg_version);
char *layer_str(layer_description);
char *padding_str(bool, layer_description);
char *channel_mode_str(channel_mode);
char *mode_extension_str(uint8_t, channel_mode, layer_description);
char *emphasis_str(emphasis);
char *bool_str(bool);
#endif // !MP3_READER_H_
