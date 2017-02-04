#ifndef BLOCK_DECOMPRESSOR_H_
#define BLOCK_DECOMPRESSOR_H_

#include <cstddef>
#include <cstdint>

#include "block.h"
#include "constants.h"

class BlockDecompressor {
public:
    BlockDecompressor();

    ~BlockDecompressor();

    ZjumpErrorCode Decompress(uint8_t* in,
                              size_t in_size,
                              uint8_t* out,
                              size_t* out_size);

private:
    ZjumpBlock block_;

    void Init();

    void ApplyInverseRle1();

    void DecodeJSeqStream();

    ZjumpErrorCode ApplyInverseBwt(uint8_t* stream, size_t stream_size);
};

#endif // BLOCK_DECOMPRESSOR_H_

