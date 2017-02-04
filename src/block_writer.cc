/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "block_writer.h"

#include <cstring>
#include <vector>

using namespace std;

// TODO: Improve function. See here: http://graphics.stanford.edu/~seander/bithacks.html
static void ReverseBits(const uint16_t bits,
                        const uint8_t num_bits,
                        uint16_t* reversed_bits) {
    uint16_t p = bits;
    uint16_t q = 0;
    for(uint8_t i=0; i<num_bits; ++i) {
        q <<= 1;
        q |= (p & 1);
        p >>= 1;
    }
    *reversed_bits = q;
}

BlockWriter::BlockWriter(const ZjumpBlock& block) : block_(block) {
}

ZjumpErrorCode BlockWriter::Write(const size_t allocated_size,
                                  uint8_t* stream,
                                  size_t* stream_size) {
    memset(stream, 0, allocated_size);

    BitStreamWriter writer(stream, allocated_size);

    ZjumpErrorCode code = WriteBwtMetadata(&writer);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = WriteHuffmanTree(&writer);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = WriteLiterals(&writer);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = WriteJumpSequences(&writer);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    const size_t num_bits = writer.Get().size;
    *stream_size = (num_bits / 8) + ((num_bits % 8) > 0);

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockWriter::WriteBwtMetadata(BitStreamWriter* writer) {
    uint8_t written = writer->Append(block_.bwt_primary_index, kBlockBwtPrimaryIndexFieldSize);
    if(written != kBlockBwtPrimaryIndexFieldSize) {
        return ZJUMP_ERROR_BIT_WRITER;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockWriter::WriteHuffmanTree(BitStreamWriter* writer) {
    const HuffmanEncoding *encoding = block_.huff_encoding;

    for(uint16_t s=0; s<kBlockMaxEncodingSymbols; ++s) {
        if(encoding->GetEncodedSymbol(s) == nullptr) {
            if(writer->Append(0, 1) != 1) {
                return ZJUMP_ERROR_BIT_WRITER;
            }
        } else {
            if(writer->Append(1, 1) != 1) {
                return ZJUMP_ERROR_BIT_WRITER;
            }
        }
    }

    for(uint16_t s=0; s<kBlockMaxEncodingSymbols; ++s) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(s);
        if(enc != nullptr) {
            uint8_t written = writer->Append(enc->enc_bit_length, kBlockHuffmanBitLengthFieldSize);
            if(written != kBlockHuffmanBitLengthFieldSize) {
                return ZJUMP_ERROR_BIT_WRITER;
            }
        }
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockWriter::WriteLiterals(BitStreamWriter* writer) {
    uint8_t written = writer->Append(block_.padding_literals_size, kBlockNumLiteralsFieldSize);
    if(written != kBlockNumLiteralsFieldSize) {
        return ZJUMP_ERROR_BIT_WRITER;
    }

    for(size_t i=0; i<block_.padding_literals_size; ++i) {
        if(writer->Append(block_.padding_literals[i], 8) != 8) {
            return ZJUMP_ERROR_BIT_WRITER;
        }
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockWriter::WriteJumpSequences(BitStreamWriter* writer) {
    const HuffmanEncoding *encoding = block_.huff_encoding;

    uint8_t written = writer->Append(block_.num_jseqs, kBlockNumJumpSequencesFieldSize);
    if(written != kBlockNumJumpSequencesFieldSize) {
        return ZJUMP_ERROR_BIT_WRITER;
    }

    for(size_t i=0; i<block_.jseq_literals_size; ++i) {
        if(writer->Append(block_.jseq_literals[i], 8) != 8) {
            return ZJUMP_ERROR_BIT_WRITER;
        }
    }

    for(size_t i=0; i<block_.jseq_stream_size; ++i) {
        const uint16_t symbol = block_.jseq_stream[i];
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(symbol);
        uint16_t reversed_bits;

        if(enc == nullptr) {
            return ZJUMP_ERROR_UNEXPECTED;
        }

        ReverseBits(enc->enc_value, enc->enc_bit_length, &reversed_bits);

        written = writer->Append(reversed_bits, enc->enc_bit_length);
        if(written != enc->enc_bit_length) {
            return ZJUMP_ERROR_BIT_WRITER;
        }

        if(symbol == kBigJumpSymbol) {
            written = writer->Append(block_.jseq_stream[++i], kBlockBigJumpFieldSize);
            if(written != kBlockBigJumpFieldSize) {
                return ZJUMP_ERROR_BIT_WRITER;
            }
        }
    }

    return ZJUMP_NO_ERROR;
}

