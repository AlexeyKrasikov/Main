#ifndef CRC_H
#define CRC_H

#include <avr/pgmspace.h>

uint8_t computeCRC8(const uint8_t *Array, uint8_t len);

uint8_t computeTableCRC8(const uint8_t *Array, uint8_t len);

#endif // CRC_H