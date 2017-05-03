/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef JUMP_SEQUENCE_H_
#define JUMP_SEQUENCE_H_

#include <cstddef>
#include <cstdint>

#include "block.h"
#include "constants.h"

// Jump Sequence Transform (Jst).
// It turns a byte stream into a ZjumpBlock object.
class Jst {
public:
    Jst(uint8_t* stream, size_t stream_size);

    ZjumpErrorCode Transform(ZjumpBlock* block);

private:
    struct SearchingContext;
    struct SearchingStepContext;

    uint8_t *stream_;
    size_t stream_size_;
    ZjumpBlock *block_;

    void SearchJumpSequences(SearchingContext *search_ctx);

    size_t AppendJumpSequences(const SearchingContext& search_ctx);

    void AddJumpSequenceBackward(const uint8_t byte,
                                 const SearchingContext& search_ctx,
                                 uint16_t** jseq_stream_ptr,
                                 uint32_t* index);

    void ShrinkStream(const uint16_t* jseq_stream,
                      const size_t jseq_stream_size);
};

// Inverse Jump Sequence Transform.
// It produces a data stream from a ZjumpBlock object.
class InverseJst {
public:
    InverseJst(const ZjumpBlock& block);

    ZjumpErrorCode Transform(uint8_t* stream,
                             size_t* stream_size);

private:
    const ZjumpBlock &block_;

    bool EnlargeStream(const uint8_t* jseq_literals,
                       const size_t jseq_literals_size,
                       const uint16_t* jseq_stream,
                       const size_t jseq_stream_size,
                       const uint8_t* in_data,
                       const size_t in_data_size,
                       uint8_t* out_data,
                       size_t* out_data_size);
};

#endif // JUMP_SEQUENCE_H_

