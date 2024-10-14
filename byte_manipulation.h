#include <stdint.h>

#ifndef BYTE_MANIPULATION_H_
#define BYTE_MANIPULATION_H_

typedef enum {
  REST_OF_BITS,
  ONE_BIT = 0x01,
  TWO_BITS = 0x03,
  THREE_BITS = 0x05,
  FOUR_BITS = 0x07,
  FIVE_BITS = 0x09,
} num_bits;

uint8_t bits_from_u8(uint8_t, int, num_bits);
uint16_t bits_from_u16(uint16_t, int, num_bits);
uint16_t join_two_u8(uint8_t *, uint8_t *);
#endif // !BYTE_MANIPULATION_H_
