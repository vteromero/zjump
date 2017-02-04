/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "block_compressor.h"

#include <algorithm>
#include <cassert>
#include <divsufsort.h>

#include "block_writer.h"
#include "huffman.h"
#include "jump_sequence.h"
#include "mem.h"
#include "rle.h"

BlockCompressor::BlockCompressor() {
    source_stream_ = nullptr;
    source_stream_size_ = 0;
}

BlockCompressor::~BlockCompressor() {
    if(block_.huff_encoding != nullptr) {
        delete block_.huff_encoding;
    }
}

ZjumpErrorCode BlockCompressor::Compress(uint8_t* in,
                                         size_t in_size,
                                         uint8_t* out,
                                         size_t* out_size) {
    assert(in != nullptr);
    assert(in_size > 0);
    assert(in_size <= kBlockMaxExpandedStreamSize);
    assert(out != nullptr);

    Init(in, in_size);

    ZjumpErrorCode result = ApplyBwt();
    if(result != ZJUMP_NO_ERROR) {
        return result;
    }

    result = JumpSequenceTransform(source_stream_, source_stream_size_, &block_);
    if(result != ZJUMP_NO_ERROR) {
        return result;
    }

    result = EncodeJSeqStream();
    if(result != ZJUMP_NO_ERROR) {
        return result;
    }

    Rle1(block_.jseq_stream, &block_.jseq_stream_size);

    result = CreateEncodingTable();
    if(result != ZJUMP_NO_ERROR) {
        return result;
    }

    BlockWriter block_writer(block_);
    result = block_writer.Write(kBlockMaxCompressedStreamSize, out, out_size);
    if(result != ZJUMP_NO_ERROR) {
        return result;
    }

    return ZJUMP_NO_ERROR;
}

void BlockCompressor::Init(uint8_t* stream, size_t stream_size) {
    source_stream_ = stream;
    source_stream_size_ = stream_size;

    block_.Clear();

    if(block_.huff_encoding != nullptr) {
        delete block_.huff_encoding;
    }
}

ZjumpErrorCode BlockCompressor::ApplyBwt() {
    int pidx = divbwt(source_stream_, source_stream_, nullptr, source_stream_size_);
    if(pidx < 0) {
        return ZJUMP_ERROR_BWT;
    }

    block_.bwt_primary_index = static_cast<uint32_t>(pidx);

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockCompressor::EncodeJSeqStream() {
    uint16_t *encoded = SecureAlloc<uint16_t>(kBlockMaxCompressedStreamSize);
    size_t n = 0;

    for(size_t i=0; i<block_.jseq_stream_size; ++i) {
        uint16_t jump = block_.jseq_stream[i];

        if(jump > kMaxJumpSize) {
            encoded[n++] = kBigJumpSymbol;
            encoded[n++] = jump; //TODO: review
        } else if(jump >= kMinJumpSize) {
            encoded[n++] = kMinJumpSymbol + (jump - kMinJumpSize);
        } else {
            encoded[n++] = jump;
        }
    }

    SecureFree<uint16_t>(block_.jseq_stream);

    block_.jseq_stream = encoded;
    block_.jseq_stream_size = n;

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockCompressor::CreateEncodingTable() {
    HuffmanFrequencyBuilder builder(kBlockMaxEncodingSymbols, kBlockMaxEncodingBitLength);

    for(size_t i=0; i<block_.jseq_stream_size; ++i) {
        builder.AddSymbolFrequency(block_.jseq_stream[i], 1);

        if(block_.jseq_stream[i] == kBigJumpSymbol) {
            ++i; // skip next: big jump value
        }
    }

    block_.huff_encoding = builder.Build();
    if(block_.huff_encoding == nullptr) {
        return ZJUMP_ERROR_HUFFMAN;
    }

    return ZJUMP_NO_ERROR;
}

