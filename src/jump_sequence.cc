/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "jump_sequence.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "mem.h"

using namespace std;

struct ByteCount {
    uint8_t byte;
    uint32_t count;
};

struct ByteCountCompareAsc {
    bool operator()(const ByteCount& lhs, const ByteCount& rhs) {
        return lhs.count < rhs.count;
    }
} ByteCountCompareAscObj;

struct ByteCountCompareDesc {
    bool operator()(const ByteCount& lhs, const ByteCount& rhs) {
        return lhs.count > rhs.count;
    }
} ByteCountCompareDescObj;

void CountBytes(const uint8_t* stream,
                const size_t stream_size,
                ByteCount* byte_count_list) {
    for(size_t i=0; i<256; ++i) {
        byte_count_list[i] = {static_cast<uint8_t>(i), 0};
    }

    for(size_t i=0; i<stream_size; ++i) {
        byte_count_list[stream[i]].count++;
    }
}

void SortBytes(ByteCount* byte_count_list, bool asc_order) {
    if(asc_order) {
        sort(byte_count_list, byte_count_list+256, ByteCountCompareAscObj);
    } else {
        sort(byte_count_list, byte_count_list+256, ByteCountCompareDescObj);
    }
}

size_t AppendJumpSequence(const uint8_t byte,
                        const uint8_t* stream,
                        const size_t stream_size,
                        ZjumpBlock* block) {
    size_t start = block->jseq_stream_size;
    size_t end = start;
    uint16_t jump = 1;

    for(size_t i=0; i<stream_size; ++i, ++jump) {
        if(stream[i] == byte) {
            block->jseq_stream[end++] = jump;
            jump = 0;
        }
    }

    // joining big jumps together
    size_t len = 1;
    for(size_t i=start+1; i<end; ++i) {
        if( (block->jseq_stream[i] > kMaxJumpSize) &&
            (block->jseq_stream[start + len - 1] > kMaxJumpSize)) {
            block->jseq_stream[start + len - 1] += block->jseq_stream[i];
        } else {
            block->jseq_stream[start + (len++)] = block->jseq_stream[i];
        }
    }

    // remove the last jump if it is a big jump
    if(block->jseq_stream[start + len - 1] > kMaxJumpSize) {
        --len;
    }

    return len;
}

void ShrinkStream(const uint16_t* jseq_stream,
                  const size_t jseq_stream_size,
                  uint8_t* data_stream,
                  size_t* data_stream_size) {
    const size_t sz = *data_stream_size;
    size_t n = 0;
    uint16_t jump = 1;

    for(size_t i=0, j=0; i<sz; ++i, ++jump) {
        if((j < jseq_stream_size) && (jump == jseq_stream[j])) {
            jump = 0;
            ++j;
        } else {
            data_stream[n++] = data_stream[i];
        }
    }

    *data_stream_size = n;
}

ZjumpErrorCode JumpSequenceTransform(uint8_t* stream,
                                     size_t stream_size,
                                     ZjumpBlock* block) {
    ByteCount byte_count_list[256];

    CountBytes(stream, stream_size, byte_count_list);

    SortBytes(byte_count_list, false);

    for(size_t i=0; i<256; ++i) {
        if(byte_count_list[i].count == 0) {
            continue;
        }

        if(stream_size == 0) {
            return ZJUMP_ERROR_UNEXPECTED;
        }

        uint16_t *jseq_stream = &(block->jseq_stream[block->jseq_stream_size]);
        size_t len = AppendJumpSequence(byte_count_list[i].byte, stream, stream_size, block);
        if(len < 3) {
            continue;
        }

        ShrinkStream(jseq_stream, len, stream, &stream_size);

        jseq_stream[len++] = kEndOfSequenceSymbol;
        block->jseq_stream_size += len;

        block->jseq_literals[block->jseq_literals_size++] = byte_count_list[i].byte;

        ++block->num_jseqs;
        if(block->num_jseqs == kBlockMaxNumJumpSequences) {
            break;
        }
    }

    memcpy(&(block->padding_literals[block->padding_literals_size]), stream, stream_size);
    block->padding_literals_size += stream_size;

    return ZJUMP_NO_ERROR;
}

static bool EnlargeStream(const uint8_t byte,
                          const vector<uint16_t>& jseq_stream,
                          const uint8_t* in_data,
                          const size_t in_data_size,
                          uint8_t* out_data,
                          size_t* out_data_size) {
    size_t jseq_stream_size = jseq_stream.size();
    size_t n = 0;
    size_t m = 0;

    for(size_t i=0; i<jseq_stream_size; ++i) {
        if(jseq_stream[i] == 0) {
            continue;
        }

        size_t sz = jseq_stream[i] - 1u;
        if((m + sz) > in_data_size) {
            return false;
        }

        std::copy_n(in_data+m, sz, out_data+n);
        n += sz;
        m += sz;

        out_data[n++] = byte;
    }

    size_t sz = in_data_size - m;
    std::copy_n(in_data+m, sz, out_data+n);
    n += sz;
    m += sz;

    *out_data_size = n;

    return true;
}

ZjumpErrorCode InverseJumpSequenceTransform(const ZjumpBlock& block,
                                            uint8_t* stream,
                                            size_t* stream_size) {
    vector<vector<uint16_t> > jseq_list(block.num_jseqs);

    for(size_t i=0, j=0; j<block.num_jseqs; ++j) {
        for(; i<block.jseq_stream_size; ++i) {
            if(block.jseq_stream[i] == kEndOfSequenceSymbol) {
                ++i;
                break;
            }
            jseq_list[j].push_back(block.jseq_stream[i]);
        }
    }

    uint8_t *in = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    size_t in_size = 0;
    uint8_t *out = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    size_t out_size = 0;

    std::copy_n(block.padding_literals, block.padding_literals_size, out);
    out_size = block.padding_literals_size;

    for(size_t i=block.num_jseqs; i-->0; ) {
        uint8_t *p_aux = in;
        in = out;
        in_size = out_size;
        out = p_aux;
        out_size = 0;

        if(!EnlargeStream(block.jseq_literals[i], jseq_list[i], in, in_size, out, &out_size)) {
            SecureFree<uint8_t>(in);
            SecureFree<uint8_t>(out);
            return ZJUMP_ERROR_RECONSTRUCTING_STREAM;
        }
    }

    std::copy_n(out, out_size, stream);
    *stream_size = out_size;

    SecureFree<uint8_t>(in);
    SecureFree<uint8_t>(out);

    return ZJUMP_NO_ERROR;
}

