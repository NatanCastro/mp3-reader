#include "./byte_manipulation.h"

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

uint16_t join_two_u8(uint8_t *first_byte, uint8_t *second_byte) {
  return (*first_byte << 8) | *second_byte;
}
