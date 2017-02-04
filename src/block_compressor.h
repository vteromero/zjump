#ifndef BLOCK_COMPRESSOR_H_
#define BLOCK_COMPRESSOR_H_

#include <cstddef>
#include <cstdint>

#include "block.h"
#include "constants.h"

class BlockCompressor {
public:
    BlockCompressor();

    ~BlockCompressor();

    ZjumpErrorCode Compress(uint8_t* in,
                            size_t in_size,
                            uint8_t* out,
                            size_t* out_size);

private:
    uint8_t *source_stream_;
    size_t source_stream_size_;
    ZjumpBlock block_;

    void Init(uint8_t *stream, size_t stream_size);

    ZjumpErrorCode ApplyBwt();

    ZjumpErrorCode EncodeJSeqStream();

    ZjumpErrorCode CreateEncodingTable();
};

#endif // BLOCK_COMPRESSOR_H_

