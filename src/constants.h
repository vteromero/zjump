/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <cstddef>
#include <cstdint>

typedef enum {
    ZJUMP_NO_ERROR,
    ZJUMP_ERROR_UNEXPECTED,
    ZJUMP_ERROR_ARGUMENT,
    ZJUMP_ERROR_MEMORY_ALLOC,
    ZJUMP_ERROR_FILE,
    ZJUMP_ERROR_BWT,
    ZJUMP_ERROR_HUFFMAN,
    ZJUMP_ERROR_BIT_WRITER,
    ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT,
    ZJUMP_ERROR_FORMAT_STREAM_TOO_LARGE,
    ZJUMP_ERROR_FORMAT_NUM_BLOCKS,
    ZJUMP_ERROR_FORMAT_BLOCK_LENGTH,
    ZJUMP_ERROR_FORMAT_BWT_PRIMARY_INDEX,
    ZJUMP_ERROR_FORMAT_HUFFMAN_BIT_LENGTH,
    ZJUMP_ERROR_FORMAT_LITERALS_LENGTH,
    ZJUMP_ERROR_FORMAT_NUM_JSEQS,
    ZJUMP_ERROR_FORMAT_HUFFMAN_ENCODED_SYMBOL,
    ZJUMP_ERROR_RECONSTRUCTING_STREAM
} ZjumpErrorCode;

// Zjump version = MAJOR*10000 + MINOR*100 + PATCH
static const uint32_t kZjumpVersion = 201;

static const size_t kBlockMaxExpandedStreamSize     = 200000;
static const size_t kBlockMaxCompressedStreamSize   = 250000;

static const size_t kBlockMaxNumJumpSequences = 65535;

static const uint16_t kRUNASymbol           = 0;
static const uint16_t kRUNBSymbol           = 1;
static const uint16_t kMinJumpSymbol        = 2;
static const uint16_t kMaxJumpSymbol        = 252;
static const uint16_t kSkipChunkSymbol      = 253;
static const uint16_t kEndOfSequenceSymbol  = 254;
static const uint16_t kShrinkStreamSymbol   = 255;

static const uint16_t kMinJumpSize = 2;
static const uint16_t kMaxJumpSize = kMaxJumpSymbol - kMinJumpSymbol + kMinJumpSize;

static const uint16_t kBlockMaxEncodingSymbols  = 256;
static const uint8_t kBlockMaxEncodingBitLength = 15;

static const uint8_t kBlockBwtPrimaryIndexFieldSize     = 24;
static const uint8_t kBlockHuffmanBitLengthFieldSize    = 4;
static const uint8_t kBlockNumLiteralsFieldSize         = 24;
static const uint8_t kBlockNumJumpSequencesFieldSize    = 16;

#endif //CONSTANTS_H_

