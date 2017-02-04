/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef BLOCK_READER_H_
#define BLOCK_READER_H_

#include <cstddef>
#include <cstdint>

#include "bit_stream.h"
#include "block.h"
#include "constants.h"

class BlockReader {
public:
    BlockReader(uint8_t* stream, size_t stream_size);

    ZjumpErrorCode Read(ZjumpBlock* block);

private:
    uint8_t *stream_;
    size_t stream_size_;
    ZjumpBlock *block_;

    ZjumpErrorCode ReadBwtMetadata(BitStreamReader& reader);

    ZjumpErrorCode ReadHuffmanTree(BitStreamReader& reader);

    ZjumpErrorCode ReadLiterals(BitStreamReader& reader);

    ZjumpErrorCode ReadJumpSequences(BitStreamReader& reader);

    ZjumpErrorCode ReadJSeqLiterals(BitStreamReader& reader);

    ZjumpErrorCode ReadJSeqStream(BitStreamReader& reader);
};

#endif // BLOCK_READER_H_

