/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include <cstddef>
#include <cstdint>

#include "bit_stream.h"
#include "encode.h"

class HuffmanEncoding {
public:
    HuffmanEncoding(const uint16_t max_symbols,
                    const uint8_t max_bit_length);

    ~HuffmanEncoding();

    void SetEncodedSymbols(const EncodedSymbol* enc_symbols,
                           const size_t num_symbols);

    EncodedSymbol* GetEncodedSymbol(const uint16_t symbol) const;

    uint16_t MaxSymbols() const;

    uint8_t MaxBitLength() const;

private:
    uint16_t max_symbols_;
    uint8_t max_bit_length_;
    EncodedSymbol *enc_symbols_;
};

class HuffmanFrequencyBuilder {
public:
    HuffmanFrequencyBuilder(const uint16_t max_symbols,
                            const uint8_t max_bit_length);

    ~HuffmanFrequencyBuilder();

    void SetSymbolFrequency(const uint16_t symbol,
                            const uint32_t freq);

    void AddSymbolFrequency(const uint16_t symbol,
                            const uint32_t freq);

    HuffmanEncoding* Build();

private:
    uint16_t max_symbols_;
    uint8_t max_bit_length_;
    uint32_t *symbol_freqs_;
};

class HuffmanBitLengthBuilder {
public:
    HuffmanBitLengthBuilder(const uint16_t max_symbols,
                            const uint8_t max_bit_length);

    ~HuffmanBitLengthBuilder();

    void SetSymbolBitLength(const uint16_t symbol,
                            const uint8_t bit_length);

    HuffmanEncoding* Build();

private:
    uint16_t max_symbols_;
    uint8_t max_bit_length_;
    uint8_t *bit_lengths_;
};

// HuffmanWriter class
//
// It writes a HuffmanEncoding object on a bit stream by using a BitStreamWriter object.
//
// It evaluates 4 different encodings and uses the one that produces the more compressed
// version or, what it is the same, the one that encodes the Huffman tree with fewer bits.
class HuffmanWriter {
public:
    // Constructor.
    HuffmanWriter(const HuffmanEncoding& huff_tree);

    // Destructor.
    ~HuffmanWriter();

    // Writes the encoded representation of the HuffmanEncoding object huff_tree_
    // on the bit stream contained in writer.
    bool Write(BitStreamWriter* writer);

private:
    const HuffmanEncoding &huff_tree_;
    BitStreamWriter *writer_;
    uint8_t encoding_type_;
    size_t range_size_;
    uint8_t *range_flags_;
    size_t range_flags_size_;

    uint8_t EncodingType();

    size_t EncodingLength(size_t num_symbols,
                          size_t range_size,
                          size_t* sum_symbols);

    // Writes the encoding type.
    bool WriteEncodingType();

    // Writes the flags that show the symbols present on the Huffman tree.
    bool WriteFlags();

    // Writes a bit for every range of size range_size_ from 0 to the maximum
    // number of symbols. A bit 1 means that the range contains a least one
    // symbol. If there is no symbol, the bit will be 0.
    bool WriteRangeFlags();

    // For every range that has at least a symbol in it, this method will write
    // range_size_ bits indicating the actual existing symbols in the range.
    bool WriteSymbolFlags();

    // For every value in the range [start, start+length), this function
    // writes a bit on the bit stream that tells whether the symbol exists
    // (bit 1) in that position or not (bit 0).
    bool WriteSymbolFlags(size_t start,
                          size_t length);

    // Writes the Huffman encoding length of every existing symbol.
    // They are written on the bit stream in a sequential way in a strict
    // ascending order.
    // The number of bits used to write every encoding length is fixed.
    bool WriteSymbolBitLengths();
};

// HuffmanReader class
//
// It reads a bit stream through a BitStreamReader object and builds a HuffmanEncoding
// object.
class HuffmanReader {
public:
    // Returning codes.
    static constexpr int NO_ERROR         = 0;
    static constexpr int ERROR_BIT_STREAM = 1;
    static constexpr int ERROR_HUFFMAN    = 2;

    // Constructor.
    // The max_symbols and max_bit_length values must be known in order to build
    // a HuffmanEncoding object.
    HuffmanReader(BitStreamReader& reader,
                  const uint16_t max_symbols,
                  const uint8_t max_bit_length);

    // Destructor.
    ~HuffmanReader();

    // Reads the bit stream and creates a HuffmanEncoding object.
    // The object allocation is made inside this function and must be freed
    // eventually.
    // It returns an integer telling the final state of the reading. The
    // returning codes have been defined above as public constant expressions.
    int Read(HuffmanEncoding** huff_tree);

private:
    BitStreamReader& reader_;
    const uint16_t max_symbols_;
    const uint8_t max_bit_length_;
    uint8_t encoding_type_;
    uint16_t *symbols_;
    uint8_t *bit_lengths_;
    size_t num_symbols_;

    bool ReadEncodingType();

    bool ReadFlags();

    bool ReadRangeFlags(size_t range_size);

    bool ReadSymbolFlags(size_t start,
                         size_t length);

    bool ReadSymbolBitLengths();
};

#endif // HUFFMAN_H_

