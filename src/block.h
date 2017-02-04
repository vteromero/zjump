#ifndef BLOCK_H_
#define BLOCK_H_

#include <cstddef>
#include <cstdint>

#include "huffman.h"

struct ZjumpBlock {
    uint32_t bwt_primary_index;
    HuffmanEncoding *huff_encoding;
    uint16_t num_jseqs;
    uint16_t *jseq_stream;
    size_t jseq_stream_size;
    uint8_t *jseq_literals;
    size_t jseq_literals_size;
    uint8_t *padding_literals;
    size_t padding_literals_size;

    ZjumpBlock();

    ~ZjumpBlock();

    void Clear();
};

#endif // BLOCK_H_

