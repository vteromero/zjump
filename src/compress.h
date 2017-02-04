#ifndef COMPRESS_H_
#define COMPRESS_H_

#include <cstdint>
#include <cstdio>

#include "constants.h"

class Compressor {
public:
    Compressor();

    ~Compressor();

    ZjumpErrorCode Compress(FILE *in_file, FILE *out_file);

private:
    uint8_t *in_stream_;
    uint8_t *out_stream_;
    size_t in_stream_size_;
    size_t out_stream_size_;
    FILE *out_file_;
    uint16_t num_blocks_;

    ZjumpErrorCode ReserveNumBlocksField();

    ZjumpErrorCode WriteNumBlocksField();

    ZjumpErrorCode WriteBlock();
};

#endif // COMPRESS_H_

