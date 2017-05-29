/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "bit_stream.h"

#include <cassert>
#include <cstdio>

static const size_t kBitStreamMaxNumBitsToWrite = 56;
static const size_t kBitStreamMaxNumBitsToRead  = 56;

template<typename T>
uint8_t ReadBits(uint8_t num_bits,
                 size_t pos,
                 T* bits,
                 BitStream* bit_stream) {
    const uint8_t max_num_bits = sizeof(T) * 8;
    assert(num_bits <= max_num_bits);

    uint64_t value = *bits;
    uint8_t read = ReadBits(num_bits, pos, &value, bit_stream);

    *bits = static_cast<T>(value);

    return read;
}

template<>
uint8_t ReadBits(uint8_t num_bits,
                 size_t pos,
                 uint64_t* bits,
                 BitStream* bit_stream) {
    assert(num_bits > 0);
    assert(num_bits <= kBitStreamMaxNumBitsToRead);
    assert(pos < bit_stream->size);

    if((pos + num_bits) > bit_stream->size) {
        num_bits = bit_stream->size - pos;
    }

    uint64_t value = *((uint64_t *)(&bit_stream->bytes[pos >> 3]));
    uint64_t mask = (1ULL << num_bits) - 1ULL;
    uint8_t shift = pos % 8;
    *bits = (value >> shift) & mask;

    return num_bits;
}

BitStreamWriter::BitStreamWriter(uint8_t* stream,
                                 size_t stream_size) {
    bit_stream_.bytes = stream;
    bit_stream_.allocated = stream_size;
    bit_stream_.size = 0;
}

uint8_t BitStreamWriter::Write(uint64_t bits,
                            uint8_t num_bits,
                            size_t pos) {
    assert(num_bits > 0);
    assert(num_bits <= kBitStreamMaxNumBitsToWrite);

    const size_t max_pos = bit_stream_.allocated * 8;
    assert(pos < max_pos);

    if((pos + num_bits) > max_pos) {
        num_bits = max_pos - pos;
    }

    uint8_t shift = pos % 8;
    uint64_t mask = ((1ULL << num_bits) - 1ULL) << shift;
    uint64_t *data = (uint64_t *)(&bit_stream_.bytes[pos >> 3]);
    *data &= ~mask;
    *data |= (bits << shift);

    if((pos + num_bits) > bit_stream_.size) {
        bit_stream_.size = pos + num_bits;
    }

    return num_bits;
}

uint8_t BitStreamWriter::Append(uint64_t bits,
                             uint8_t num_bits) {
    return Write(bits, num_bits, bit_stream_.size);
}

BitStream BitStreamWriter::Get() const {
    return bit_stream_;
}

BitStreamReader::BitStreamReader(uint8_t* stream,
                                 size_t stream_size) {
    bit_stream_.bytes = stream;
    bit_stream_.allocated = stream_size;
    bit_stream_.size = stream_size * 8;

    next_pos_ = 0;
}

uint8_t BitStreamReader::Read(uint8_t num_bits,
                              size_t pos,
                              uint8_t* bits) {
    return ReadBits<uint8_t>(num_bits, pos, bits, &bit_stream_);
}

uint8_t BitStreamReader::Read(uint8_t num_bits,
                              size_t pos,
                              uint16_t* bits) {
    return ReadBits<uint16_t>(num_bits, pos, bits, &bit_stream_);
}

uint8_t BitStreamReader::Read(uint8_t num_bits,
                              size_t pos,
                              uint32_t* bits) {
    return ReadBits<uint32_t>(num_bits, pos, bits, &bit_stream_);
}

uint8_t BitStreamReader::Read(uint8_t num_bits,
                              size_t pos,
                              uint64_t* bits) {
    return ReadBits(num_bits, pos, bits, &bit_stream_);
}

uint8_t BitStreamReader::ReadNext(uint8_t num_bits,
                                  uint8_t* bits) {
    uint8_t read = Read(num_bits, next_pos_, bits);
    next_pos_ += read;
    return read;
}

uint8_t BitStreamReader::ReadNext(uint8_t num_bits,
                                  uint16_t* bits) {
    uint8_t read = Read(num_bits, next_pos_, bits);
    next_pos_ += read;
    return read;
}

uint8_t BitStreamReader::ReadNext(uint8_t num_bits,
                                  uint32_t* bits) {
    uint8_t read = Read(num_bits, next_pos_, bits);
    next_pos_ += read;
    return read;
}

uint8_t BitStreamReader::ReadNext(uint8_t num_bits,
                                  uint64_t* bits) {
    uint8_t read = Read(num_bits, next_pos_, bits);
    next_pos_ += read;
    return read;
}

void BitStreamReader::Reset() {
    MoveTo(0);
}

void BitStreamReader::MoveTo(size_t bit_pos) {
    next_pos_ = bit_pos;
}

size_t BitStreamReader::NextPos() const {
    return next_pos_;
}

