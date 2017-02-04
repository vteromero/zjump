#ifndef DECOMPRESS_H_
#define DECOMPRESS_H_

#include <cstdint>
#include <cstdio>

#include "constants.h"

class Decompressor {
public:
    Decompressor();

    ~Decompressor();

    ZjumpErrorCode Decompress(FILE* in_file, FILE* out_file);

private:
    uint8_t *in_stream_;
    uint8_t *out_stream_;
    size_t in_stream_size_;
    size_t out_stream_size_;
    FILE *in_file_;
    uint16_t num_blocks_;

    ZjumpErrorCode ReadNumBlocks();

    ZjumpErrorCode ReadBlock();

    bool AnyRemainingData(FILE* file);
};

#endif // DECOMPRESS_H_

