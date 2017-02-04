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

static const size_t kBlockMaxExpandedStreamSize     = 65534;
static const size_t kBlockMaxCompressedStreamSize   = 120000;

static const size_t kBlockMaxNumJumpSequences = 1023;

static const uint16_t kEndOfSequenceSymbol  = 0;
static const uint16_t kRUNASymbol           = 1;
static const uint16_t kRUNBSymbol           = 2;
static const uint16_t kMinJumpSymbol        = 3;
static const uint16_t kMaxJumpSymbol        = 254;
static const uint16_t kBigJumpSymbol        = 255;

static const uint16_t kMinJumpSize = 2;
static const uint16_t kMaxJumpSize = kMaxJumpSymbol - kMinJumpSymbol + kMinJumpSize;

static const uint16_t kBlockMaxEncodingSymbols  = kBigJumpSymbol + 1;
static const uint8_t kBlockMaxEncodingBitLength = 15;

static const uint8_t kBlockBwtPrimaryIndexFieldSize     = 16;
static const uint8_t kBlockHuffmanBitLengthFieldSize    = 4;
static const uint8_t kBlockNumLiteralsFieldSize         = 16;
static const uint8_t kBlockNumJumpSequencesFieldSize    = 10;
static const uint8_t kBlockBigJumpFieldSize             = 16;

#endif //CONSTANTS_H_

