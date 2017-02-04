#include "compress.h"

#include <cassert>
#include <cstdio>

#include "block_compressor.h"
#include "mem.h"

Compressor::Compressor() {
    in_stream_ = SecureAlloc<uint8_t>(kBlockMaxExpandedStreamSize);
    out_stream_ = SecureAlloc<uint8_t>(kBlockMaxCompressedStreamSize);
    in_stream_size_ = 0;
    out_stream_size_ = 0;
    out_file_ = nullptr;
    num_blocks_ = 0;
}

Compressor::~Compressor() {
    SecureFree<uint8_t>(in_stream_);
    SecureFree<uint8_t>(out_stream_);
}

ZjumpErrorCode Compressor::Compress(FILE *in_file, FILE *out_file) {
    assert(in_file != nullptr);
    assert(out_file != nullptr);

    out_file_ = out_file;
    num_blocks_ = 0;

    ZjumpErrorCode ret_code = ReserveNumBlocksField();
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    do {
        out_stream_size_ = 0;
        in_stream_size_ = fread(in_stream_, 1, kBlockMaxExpandedStreamSize, in_file);

        if(ferror(in_file)) {
            return ZJUMP_ERROR_FILE;
        }

        if(in_stream_size_ == 0) {
            break;
        }

        BlockCompressor block_comp;
        ret_code = block_comp.Compress(in_stream_, in_stream_size_, out_stream_, &out_stream_size_);
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }

        ret_code = WriteBlock();
        if(ret_code != ZJUMP_NO_ERROR) {
            return ret_code;
        }

        ++num_blocks_;

    } while(!feof(in_file));

    ret_code = WriteNumBlocksField();
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }


    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode Compressor::ReserveNumBlocksField() {
    uint16_t zero = 0;

    size_t written = fwrite(&zero, 2, 1, out_file_);
    if((written != 1) || ferror(out_file_)) {
        return ZJUMP_ERROR_FILE;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode Compressor::WriteNumBlocksField() {
    fseek(out_file_, 0, SEEK_SET);
    if(ferror(out_file_)) {
        return ZJUMP_ERROR_FILE;
    }

    size_t written = fwrite(&num_blocks_, 2, 1, out_file_);
    if((written != 1) || ferror(out_file_)) {
        return ZJUMP_ERROR_FILE;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode Compressor::WriteBlock() {
    uint32_t block_length_field = static_cast<uint32_t>(out_stream_size_);

    size_t written = fwrite(&block_length_field, 3, 1, out_file_);
    if((written != 1) || ferror(out_file_)) {
        return ZJUMP_ERROR_FILE;
    }

    written = fwrite(out_stream_, 1, out_stream_size_, out_file_);
    if((written != out_stream_size_) || ferror(out_file_)) {
        return ZJUMP_ERROR_FILE;
    }

    return ZJUMP_NO_ERROR;
}

