/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef BIT_STREAM_H_
#define BIT_STREAM_H_

#include <cstddef>
#include <cstdint>

struct BitStream {
    uint8_t *bytes;
    size_t allocated;
    size_t size;
};

class BitStreamWriter {
public:
    BitStreamWriter(uint8_t* stream,
                    size_t stream_size);

    uint8_t Write(uint64_t bits,
                  uint8_t num_bits,
                  size_t pos);

    uint8_t Append(uint64_t bits,
                   uint8_t num_bits);

    BitStream Get() const;

private:
    BitStream bit_stream_;
};

class BitStreamReader {
public:
    BitStreamReader(uint8_t* stream,
                    size_t stream_size);

    uint8_t Read(uint8_t num_bits,
                 size_t pos,
                 uint8_t* bits);

    uint8_t Read(uint8_t num_bits,
                 size_t pos,
                 uint16_t* bits);

    uint8_t Read(uint8_t num_bits,
                 size_t pos,
                 uint32_t* bits);

    uint8_t Read(uint8_t num_bits,
                 size_t pos,
                 uint64_t* bits);

    uint8_t ReadNext(uint8_t num_bits,
                     uint8_t* bits);

    uint8_t ReadNext(uint8_t num_bits,
                     uint16_t* bits);

    uint8_t ReadNext(uint8_t num_bits,
                     uint32_t* bits);

    uint8_t ReadNext(uint8_t num_bits,
                     uint64_t* bits);

    void Reset();

    void MoveTo(size_t bit_pos);

    size_t NextPos() const;

private:
    BitStream bit_stream_;
    size_t next_pos_;
};

#endif // BIT_STREAM_H_

