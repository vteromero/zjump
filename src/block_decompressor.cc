/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "block_decompressor.h"

#include <cassert>
#include <divsufsort.h>

#include "block_reader.h"
#include "jump_sequence.h"
#include "mem.h"
#include "rle.h"

BlockDecompressor::BlockDecompressor() {
}

BlockDecompressor::~BlockDecompressor() {
    if(block_.huff_encoding != nullptr) {
        delete block_.huff_encoding;
    }
}

ZjumpErrorCode BlockDecompressor::Decompress(uint8_t* in,
                                             size_t in_size,
                                             uint8_t* out,
                                             size_t* out_size) {
    assert(in != nullptr);
    assert(in_size > 0);
    assert(in_size <= kBlockMaxCompressedStreamSize);
    assert(out != nullptr);

    Init();

    BlockReader block_reader(in, in_size);
    ZjumpErrorCode ret_code = block_reader.Read(&block_);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    ApplyInverseRle1();

    DecodeJSeqStream();

    InverseJst inv_jst(block_);
    ret_code = inv_jst.Transform(out, out_size);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    ret_code = ApplyInverseBwt(out, *out_size);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    return ZJUMP_NO_ERROR;
}

void BlockDecompressor::Init() {
    block_.Clear();

    if(block_.huff_encoding != nullptr) {
        delete block_.huff_encoding;
    }
}

void BlockDecompressor::ApplyInverseRle1() {
    uint16_t *out = SecureAlloc<uint16_t>(kBlockMaxCompressedStreamSize);
    size_t out_size = 0;

    InverseRle1(block_.jseq_stream, block_.jseq_stream_size, out, &out_size);

    SecureFree<uint16_t>(block_.jseq_stream);

    block_.jseq_stream = out;
    block_.jseq_stream_size = out_size;
}

void BlockDecompressor::DecodeJSeqStream() {
    size_t n = 0;

    for(size_t i=0; i<block_.jseq_stream_size; ++i) {
        uint16_t symbol = block_.jseq_stream[i];

        if((symbol >= kMinJumpSymbol) && (symbol <= kMaxJumpSymbol)) {
            block_.jseq_stream[n++] = kMinJumpSize + (symbol - kMinJumpSymbol);
        } else {
            block_.jseq_stream[n++] = symbol;
        }
    }

    block_.jseq_stream_size = n;
}

ZjumpErrorCode BlockDecompressor::ApplyInverseBwt(uint8_t* stream, size_t stream_size) {
    int pidx = static_cast<int>(block_.bwt_primary_index);

    if(inverse_bw_transform(stream, stream, nullptr, stream_size, pidx) != 0) {
        return ZJUMP_ERROR_BWT;
    }

    return ZJUMP_NO_ERROR;
}

