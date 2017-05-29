/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "block_reader.h"

#include <cassert>
#include <unordered_map>

#include "huffman.h"

BlockReader::BlockReader(uint8_t* stream, size_t stream_size) {
    assert(stream != nullptr);
    assert(stream_size > 0);
    stream_ = stream;
    stream_size_ = stream_size;
}

ZjumpErrorCode BlockReader::Read(ZjumpBlock* block) {
    block_ = block;

    BitStreamReader reader(stream_, stream_size_);

    ZjumpErrorCode code = ReadBwtMetadata(reader);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = ReadHuffmanTree(reader);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = ReadLiterals(reader);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    code = ReadJumpSequences(reader);
    if(code != ZJUMP_NO_ERROR) {
        return code;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockReader::ReadBwtMetadata(BitStreamReader& reader) {
    uint8_t read = reader.ReadNext(kBlockBwtPrimaryIndexFieldSize, &(block_->bwt_primary_index));
    if(read != kBlockBwtPrimaryIndexFieldSize) {
        return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
    }

    if(block_->bwt_primary_index > kBlockMaxExpandedStreamSize) {
        return ZJUMP_ERROR_FORMAT_BWT_PRIMARY_INDEX;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockReader::ReadHuffmanTree(BitStreamReader& reader) {
    HuffmanReader huff_reader(reader, kBlockMaxEncodingSymbols, kBlockMaxEncodingBitLength);
    int ret_code = huff_reader.Read(&(block_->huff_encoding));

    switch(ret_code) {
        case HuffmanReader::NO_ERROR:
            return ZJUMP_NO_ERROR;
        case HuffmanReader::ERROR_BIT_STREAM:
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        case HuffmanReader::ERROR_HUFFMAN:
            return ZJUMP_ERROR_HUFFMAN;
        default:
            return ZJUMP_ERROR_UNEXPECTED;
    }
}

ZjumpErrorCode BlockReader::ReadLiterals(BitStreamReader& reader) {
    uint8_t read = reader.ReadNext(kBlockNumLiteralsFieldSize, &(block_->padding_literals_size));
    if(read != kBlockNumLiteralsFieldSize) {
        return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
    }

    if(block_->padding_literals_size > kBlockMaxExpandedStreamSize) {
        return ZJUMP_ERROR_FORMAT_LITERALS_LENGTH;
    }

    for(size_t i=0; i<block_->padding_literals_size; ++i) {
        if(reader.ReadNext(8, &(block_->padding_literals[i])) != 8) {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockReader::ReadJumpSequences(BitStreamReader& reader) {
    uint8_t read = reader.ReadNext(kBlockNumJumpSequencesFieldSize, &(block_->num_jseqs));
    if(read != kBlockNumJumpSequencesFieldSize) {
        return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
    }

    if(block_->num_jseqs > kBlockMaxNumJumpSequences) {
        return ZJUMP_ERROR_FORMAT_NUM_JSEQS;
    }

    ZjumpErrorCode ret_code = ReadJSeqLiterals(reader);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    ret_code = ReadJSeqStream(reader);
    if(ret_code != ZJUMP_NO_ERROR) {
        return ret_code;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockReader::ReadJSeqLiterals(BitStreamReader& reader) {
    for(size_t i=0; i<block_->num_jseqs; ++i) {
        if(reader.ReadNext(8, &(block_->jseq_literals[i])) != 8) {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }
    }

    block_->jseq_literals_size = block_->num_jseqs;

    return ZJUMP_NO_ERROR;
}

static ZjumpErrorCode ReadEncodedSymbol(
        BitStreamReader& reader,
        std::unordered_map<uint32_t, uint16_t>& encoded_to_symbol_map,
        uint16_t* symbol) {
    uint8_t enc_bit_length = 0;
    uint16_t enc_value = 0;
    std::unordered_map<uint32_t, uint16_t>::iterator it;

    while(enc_bit_length < kBlockMaxEncodingBitLength) {
        uint8_t bit;

        if(reader.ReadNext(1, &bit) != 1) {
            return ZJUMP_ERROR_FORMAT_STREAM_TOO_SHORT;
        }

        enc_value = (enc_value << 1) | bit;
        ++enc_bit_length;

        uint32_t k = enc_bit_length | (enc_value << 8);
        it = encoded_to_symbol_map.find(k);
        if(it != encoded_to_symbol_map.end()) {
            *symbol = it->second;
            break;
        }
    }

    if(enc_bit_length > kBlockMaxEncodingBitLength) {
        return ZJUMP_ERROR_FORMAT_HUFFMAN_ENCODED_SYMBOL;
    }

    return ZJUMP_NO_ERROR;
}

ZjumpErrorCode BlockReader::ReadJSeqStream(BitStreamReader& reader) {
    // key: enc_bit_length | (enc_value << 8)
    // value: <symbol>
    std::unordered_map<uint32_t, uint16_t> encoded_to_symbol_map;
    const HuffmanEncoding *encoding = block_->huff_encoding;
    const uint16_t max_symbols = encoding->MaxSymbols();

    for(uint16_t s=0; s<max_symbols; ++s) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(s);
        if(enc != nullptr) {
            uint32_t k = enc->enc_bit_length | (enc->enc_value << 8);
            encoded_to_symbol_map[k] = s;
        }
    }

    block_->jseq_stream_size = 0;

    for(size_t i=0; i<block_->num_jseqs; ++i) {
        uint16_t symbol=0;

        do {
            ZjumpErrorCode code = ReadEncodedSymbol(reader, encoded_to_symbol_map, &symbol);
            if(code != ZJUMP_NO_ERROR) {
                return code;
            }

            block_->jseq_stream[block_->jseq_stream_size++] = symbol;

        } while(symbol != kEndOfSequenceSymbol);
    }

    return ZJUMP_NO_ERROR;
}

