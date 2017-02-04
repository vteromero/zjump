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

ZjumpErrorCode JumpSequenceTransform(uint8_t* stream,
                                     size_t stream_size,
                                     ZjumpBlock* block);

ZjumpErrorCode InverseJumpSequenceTransform(const ZjumpBlock& block,
                                            uint8_t* stream,
                                            size_t* stream_size);

#endif // JUMP_SEQUENCE_H_

