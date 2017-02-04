#ifndef BLOCK_WRITER_H_
#define BLOCK_WRITER_H_

#include <cstddef>
#include <cstdint>

#include "bit_stream.h"
#include "block.h"
#include "constants.h"

class BlockWriter {
public:
    BlockWriter(const ZjumpBlock& block);

    ZjumpErrorCode Write(const size_t allocated_size,
                         uint8_t* stream,
                         size_t* stream_size);

private:
    const ZjumpBlock &block_;

    ZjumpErrorCode WriteBwtMetadata(BitStreamWriter* writer);

    ZjumpErrorCode WriteHuffmanTree(BitStreamWriter* writer);

    ZjumpErrorCode WriteLiterals(BitStreamWriter* writer);

    ZjumpErrorCode WriteJumpSequences(BitStreamWriter* writer);
};

#endif // BLOCK_WRITER_H_

