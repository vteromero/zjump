/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "block.h"

#include "constants.h"
#include "mem.h"

ZjumpBlock::ZjumpBlock() {
    jseq_stream = SecureAlloc<uint16_t>(kBlockMaxCompressedStreamSize); //TODO: review alloc size
    jseq_literals = SecureAlloc<uint8_t>(kBlockMaxNumJumpSequences);
    padding_literals = SecureAlloc<uint8_t>(kBlockMaxCompressedStreamSize);
    huff_encoding = nullptr;
    Clear();
}

ZjumpBlock::~ZjumpBlock() {
    SecureFree<uint16_t>(jseq_stream);
    SecureFree<uint8_t>(jseq_literals);
    SecureFree<uint8_t>(padding_literals);
}

void ZjumpBlock::Clear() {
    num_jseqs = 0;
    jseq_stream_size = 0;
    jseq_literals_size = 0;
    padding_literals_size = 0;
}

