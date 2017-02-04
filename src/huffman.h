#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#include <cstddef>
#include <cstdint>

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

#endif // HUFFMAN_H_

