#include "decompress.h"

#include <cassert>

#include "block_decompressor.h"
#include "mem.h"

Decompressor::Decompressor() {
    in_stream_ = SecureAlloc<uint8_t>(kBlockMaxCompressedStreamSize);
    out_stream_ = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    in_stream_size_ = 0;
    out_stream_size_ = 0;
    in_file_ = nullptr;
    num_blocks_ = 0;
}

Decompressor::~Decompressor() {
    SecureFree<uint8_t>(in_stream_);
    SecureFree<uint8_t>(out_stream_);
}

ZjumpErrorCode Decompressor::Decompress(FILE* in_file, FILE* out_file) {
    assert(in_file != nullptr);
    assert(out_file != nullptr);

    in_file_ = in_file;
    num_blocks_ = 0;

    ZjumpErrorCode ret_code = ReadNumBlocks();
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    uint16_t processed_blocks = 0;
    while(processed_blocks < num_blocks_) {
        out_stream_size_ = 0;

        ret_code = ReadBlock();
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }

        BlockDecompressor block_decomp;
        ret_code = block_decomp.Decompress(in_stream_, in_stream_size_,
            out_stream_, &out_stream_size_);
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }

        size_t written = fwrite(out_stream_, 1, out_stream_size_, out_file);
        if((written != out_stream_size_) || ferror(out_file)) {
            return ZJUMP_ERROR_FILE;
        }

        ++processed_blocks;
    }

    if(AnyRemainingData(in_file)) {
        return ZJUMP_ERROR_FORMAT_STREAM_TOO_LARGE;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode Decompressor::ReadNumBlocks() {
    size_t read = fread(&num_blocks_, 2, 1, in_file_);

    if(read != 1) {
        if(ferror(in_file_)) {
            return ZJUMP_ERROR_FILE;
        } else {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }
    }

    if(num_blocks_ == 0) {
        return ZJUMP_ERROR_FORMAT_NUM_BLOCKS;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode Decompressor::ReadBlock() {
    size_t read = fread(&in_stream_size_, 3, 1, in_file_);

    if(read != 1) {
        if(ferror(in_file_)) {
            return ZJUMP_ERROR_FILE;
        } else {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }
    }

    if(in_stream_size_ == 0) {
        return ZJUMP_ERROR_FORMAT_BLOCK_LENGTH;
    }

    read = fread(in_stream_, 1, in_stream_size_, in_file_);

    if(read != in_stream_size_) {
        if(ferror(in_file_)) {
            return ZJUMP_ERROR_FILE;
        } else {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }
    }

    return ZJUMP_NO_ERROR;
}

bool Decompressor::AnyRemainingData(FILE* file) {
    uint8_t single_byte;
    return fread(&single_byte, 1, 1, file) == 1;
}

