/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef ENCODE_H_
#define ENCODE_H_

#include <cstdint>

#include "constants.h"

struct EncodedSymbol {
    uint16_t symbol;
    uint8_t enc_bit_length;
    uint16_t enc_value;

    EncodedSymbol() : symbol(0), enc_bit_length(0), enc_value(0) {}

    EncodedSymbol(uint16_t s) : symbol(s), enc_bit_length(0), enc_value(0) {}

    EncodedSymbol(uint16_t s, uint8_t len, uint16_t enc_s) :
        symbol(s), enc_bit_length(len), enc_value(enc_s) {}

};

#endif // ENCODE_H_

