/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "jump_sequence.h"

#include <algorithm>

#include "mem.h"

static const uint32_t kJSeqExtraSize = 8 + kStaticBitLengths[kEndOfSequenceSymbol];

struct Jst::SearchingStepContext {
    uint8_t byte;
    uint32_t index;
    uint32_t jseqs_so_far;
    uint32_t symbols_so_far;
    uint32_t bytes_so_far;
    uint32_t enc_bits_so_far;

    void Init(uint8_t b, uint32_t idx) {
        byte = b;
        index = idx;
        jseqs_so_far = 0;
        symbols_so_far = 0;
        bytes_so_far = 0;
        enc_bits_so_far = 0;
    }

    int Reduction() const {
        return (bytes_so_far * 8) - enc_bits_so_far;
    }

    void LinkTo(uint8_t b, uint32_t idx) {
        if(idx <= index) {
            return;
        }

        uint32_t jump = idx - index;

        while(jump > kMaxJumpSize) {
            ++symbols_so_far;
            enc_bits_so_far += kStaticBitLengths[kSkipChunkSymbol];
            jump -= kMaxJumpSize;
        }

        enc_bits_so_far += kStaticBitLengths[jump];
        ++symbols_so_far;
        ++bytes_so_far;

        if((jseqs_so_far == 0) || (b != byte)) {
            enc_bits_so_far += kJSeqExtraSize;
            ++jseqs_so_far;
            ++symbols_so_far;
        }

        byte = b;
        index = idx;
    }
};

struct Jst::SearchingContext {
    uint32_t num_steps;
    uint32_t *prev_step_index;
    SearchingStepContext last_byte_step_ctx[256];
    SearchingStepContext best_step_ctx;

    SearchingContext(uint32_t n_steps) {
        num_steps = n_steps;
        prev_step_index = SecureAlloc<uint32_t>(n_steps);

        prev_step_index[0] = 0;

        for(size_t i=0; i<256; ++i) {
            last_byte_step_ctx[i].Init(i, 0);
        }

        best_step_ctx.Init(0, 0);
    }

    ~SearchingContext() {
        SecureFree<uint32_t>(prev_step_index);
    }

    void Update(uint8_t byte, uint32_t index) {
        if((index - last_byte_step_ctx[byte].index) > kMaxJumpSize) {
            last_byte_step_ctx[byte] = best_step_ctx;
        }

        prev_step_index[index] = last_byte_step_ctx[byte].index;

        last_byte_step_ctx[byte].LinkTo(byte, index);

        if(last_byte_step_ctx[byte].Reduction() > best_step_ctx.Reduction()) {
            best_step_ctx = last_byte_step_ctx[byte];
        }
    }
};

Jst::Jst(uint8_t* stream, size_t stream_size) {
    assert(stream != nullptr);
    assert(stream_size > 0);

    stream_ = stream;
    stream_size_ = stream_size;
    block_ = nullptr;
}

ZjumpErrorCode Jst::Transform(ZjumpBlock* block) {
    assert(block != nullptr);

    block_ = block;

    while(stream_size_) {
        SearchingContext search_ctx(stream_size_ + 1);

        SearchJumpSequences(&search_ctx);

        if( search_ctx.best_step_ctx.jseqs_so_far == 0 ||
            search_ctx.best_step_ctx.Reduction() <= 0) {
            break;
        }

        uint16_t *jseq_stream = &block_->jseq_stream[block_->jseq_stream_size];
        size_t jseq_stream_size = AppendJumpSequences(search_ctx);

        ShrinkStream(jseq_stream, jseq_stream_size);

        block_->jseq_stream[block_->jseq_stream_size++] = kShrinkStreamSymbol;
    }

    // remove the last kShrinkStreamSymbol, it is unnecesary
    --block_->jseq_stream_size;

    // the remaining data is copied as padding literals
    std::copy_n(stream_, stream_size_, &(block_->padding_literals[block_->padding_literals_size]));
    block_->padding_literals_size += stream_size_;

    return ZJUMP_NO_ERROR;
}

void Jst::SearchJumpSequences(SearchingContext *search_ctx) {
    for(uint32_t i=0; i<stream_size_; ++i) {
        search_ctx->Update(stream_[i], i + 1);
    }
}

size_t Jst::AppendJumpSequences(const SearchingContext& search_ctx) {
    size_t jseq_stream_len = search_ctx.best_step_ctx.symbols_so_far;
    size_t jseq_lit_len = search_ctx.best_step_ctx.jseqs_so_far;
    uint16_t *jseq_stream_ptr = &block_->jseq_stream[block_->jseq_stream_size + jseq_stream_len - 1];
    uint8_t *jseq_lit_ptr = &block_->jseq_literals[block_->jseq_literals_size + jseq_lit_len - 1];
    uint32_t cur_index = search_ctx.best_step_ctx.index;

    while(cur_index) {
        *(jseq_lit_ptr--) = stream_[cur_index - 1];
        AddJumpSequenceBackward(stream_[cur_index - 1], search_ctx, &jseq_stream_ptr, &cur_index);
    }

    block_->jseq_stream_size += jseq_stream_len;
    block_->jseq_literals_size += jseq_lit_len;
    block_->num_jseqs += jseq_lit_len;

    return jseq_stream_len;
}

void Jst::AddJumpSequenceBackward(const uint8_t byte,
                                  const SearchingContext& search_ctx,
                                  uint16_t** jseq_stream_ptr,
                                  uint32_t* index) {
    uint16_t *data = *jseq_stream_ptr;
    uint32_t idx = *index;

    *(data--) = kEndOfSequenceSymbol;

    while(idx && (stream_[idx - 1] == byte)) {
        uint32_t jump = idx - search_ctx.prev_step_index[idx];
        uint16_t *jump_pos_ptr = data--;

        while(jump > kMaxJumpSize) {
            *(data--) = kSkipChunkSymbol;
            jump -= kMaxJumpSize;
        }

        *jump_pos_ptr = static_cast<uint16_t>(jump);

        idx = search_ctx.prev_step_index[idx];
    }

    *jseq_stream_ptr = data;
    *index = idx;
}

void Jst::ShrinkStream(const uint16_t* jseq_stream,
                       const size_t jseq_stream_size) {
    size_t i = 0;
    size_t n = 0;

    for(size_t j=0; j<jseq_stream_size; ++j) {
        if(jseq_stream[j] == 0) {
            continue;
        }

        if(jseq_stream[j] == kEndOfSequenceSymbol) {
            continue;
        }

        if(jseq_stream[j] == kSkipChunkSymbol) {
            std::copy_n(stream_ + i, kMaxJumpSize, stream_ + n);
            i += kMaxJumpSize;
            n += kMaxJumpSize;
        } else {
            size_t sz = jseq_stream[j] - 1u;
            std::copy_n(stream_ + i, sz, stream_ + n);
            i += sz + 1;
            n += sz;
        }

    }

    size_t sz = stream_size_ - i;
    std::copy_n(stream_ + i, sz, stream_ + n);
    i += sz;
    n += sz;

    stream_size_ = n;
}

InverseJst::InverseJst(const ZjumpBlock& block) : block_(block) {
}

ZjumpErrorCode InverseJst::Transform(uint8_t* stream,
                                     size_t* stream_size) {
    assert(stream != nullptr);
    assert(stream_size != nullptr);

    uint8_t *in = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    size_t in_size = 0;
    uint8_t *out = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    size_t out_size = 0;

    // The padding literals are copied into the output stream
    std::copy_n(block_.padding_literals, block_.padding_literals_size, out);
    out_size = block_.padding_literals_size;

    size_t i = block_.jseq_stream_size;
    size_t j = block_.jseq_literals_size;
    while(i > 0 && j > 0) {
        // Find the next piece of block_.jseq_stream that is going to be added to the stream
        // These pieces are separated by kShrinkStreamSymbol symbols
        // The pieces are processed in reverse order
        uint16_t *jseq_stream = nullptr;
        size_t jseq_stream_size = 0;
        uint8_t *jseq_literals = nullptr;
        size_t jseq_literals_size = 0;

        while(i > 0) {
            --i;
            if(block_.jseq_stream[i] == kShrinkStreamSymbol) {
                break;
            } else if(block_.jseq_stream[i] == kEndOfSequenceSymbol) {
                --j;
                ++jseq_literals_size;
            }
            ++jseq_stream_size;
        }

        if(i == 0) {
            jseq_stream = &block_.jseq_stream[i];
        } else {
            jseq_stream = &block_.jseq_stream[i + 1];
        }

        jseq_literals = &block_.jseq_literals[j];

        // Swap in and out pointers
        uint8_t *aux_ptr = in;
        in = out;
        in_size = out_size;
        out = aux_ptr;
        out_size = 0;

        // Enlarge stream
        if(!EnlargeStream(jseq_literals, jseq_literals_size, jseq_stream, jseq_stream_size,
                in, in_size, out, &out_size)) {
            SecureFree<uint8_t>(in);
            SecureFree<uint8_t>(out);
            return ZJUMP_ERROR_RECONSTRUCTING_STREAM;
        }
    }

    if((i != 0) || (j != 0)) {
        SecureFree<uint8_t>(in);
        SecureFree<uint8_t>(out);
        return ZJUMP_ERROR_RECONSTRUCTING_STREAM;
    }

    // Finally, the output params are set
    std::copy_n(out, out_size, stream);
    *stream_size = out_size;

    // Free the allocated variables
    SecureFree<uint8_t>(in);
    SecureFree<uint8_t>(out);

    return ZJUMP_NO_ERROR;
}

bool InverseJst::EnlargeStream(const uint8_t* jseq_literals,
                               const size_t jseq_literals_size,
                               const uint16_t* jseq_stream,
                               const size_t jseq_stream_size,
                               const uint8_t* in_data,
                               const size_t in_data_size,
                               uint8_t* out_data,
                               size_t* out_data_size) {
    size_t n = 0;
    size_t m = 0;

    for(size_t i=0, j=0; i<jseq_stream_size; ++i) {
        if(jseq_stream[i] == 0) {
            continue;
        }

        if(jseq_stream[i] == kEndOfSequenceSymbol) {
            ++j;
            continue;
        }

        size_t sz = 0;
        while(jseq_stream[i] == kSkipChunkSymbol) {
            sz += kMaxJumpSize;
            ++i;
        }
        sz += jseq_stream[i] - 1u;

        if((m + sz) > in_data_size) {
            return false;
        }

        std::copy_n(in_data+m, sz, out_data+n);
        n += sz;
        m += sz;

        out_data[n++] = jseq_literals[j];
    }

    size_t sz = in_data_size - m;
    std::copy_n(in_data+m, sz, out_data+n);
    n += sz;
    m += sz;

    *out_data_size = n;

    return true;
}

