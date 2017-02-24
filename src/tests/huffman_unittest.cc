/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include <cstdint>

#include "gtest/gtest.h"
#include "../encode.h"
#include "../huffman.h"

TEST(HuffmanEncodingTest, MaxSymbols) {
    HuffmanEncoding he4(4, 16);
    HuffmanEncoding he16(16, 16);
    HuffmanEncoding he256(256, 16);

    EXPECT_EQ(4, he4.MaxSymbols());
    EXPECT_EQ(16, he16.MaxSymbols());
    EXPECT_EQ(256, he256.MaxSymbols());
}

TEST(HuffmanEncodingTest, MaxBitLength) {
    HuffmanEncoding he1(16, 4);
    HuffmanEncoding he2(16, 8);
    HuffmanEncoding he3(16, 16);

    EXPECT_EQ(4, he1.MaxBitLength());
    EXPECT_EQ(8, he2.MaxBitLength());
    EXPECT_EQ(16, he3.MaxBitLength());
}

TEST(HuffmanEncodingTest, SetEncodedSymbols) {
    const EncodedSymbol enc_symbols[] = {
        {1, 4, 14},
        {2, 4, 15},
        {3, 3, 4},
        {4, 3, 5},
        {5, 3, 6},
        {6, 1, 0}
    };
    const size_t n_symbols = sizeof(enc_symbols) / sizeof(enc_symbols[0]);

    HuffmanEncoding he(8, 16);
    he.SetEncodedSymbols(enc_symbols, n_symbols);

    for(size_t i=0; i<n_symbols; ++i) {
        EncodedSymbol *enc_symbol = he.GetEncodedSymbol(enc_symbols[i].symbol);
        ASSERT_TRUE(enc_symbol != nullptr);
        EXPECT_EQ(enc_symbols[i].symbol, enc_symbol->symbol);
        EXPECT_EQ(enc_symbols[i].enc_bit_length, enc_symbol->enc_bit_length);
        EXPECT_EQ(enc_symbols[i].enc_value, enc_symbol->enc_value);
    }
}

TEST(HuffmanEncodingTest, SymbolIsNotAvailable) {
    HuffmanEncoding he(16, 16);

    for(uint16_t i=0; i<16; ++i) {
        EXPECT_TRUE(nullptr == he.GetEncodedSymbol(i));
    }
}

TEST(HuffmanFrequencyBuilderTest, Build) {
    const uint16_t symbols[] = {1, 2, 3, 4, 5, 6};
    const uint32_t freqs[] = {5, 7, 10, 15, 20, 45};
    const size_t n_symbols = sizeof(symbols) / sizeof(symbols[0]);
    const EncodedSymbol expected_enc[] = {
        {1, 4, 14},
        {2, 4, 15},
        {3, 3, 4},
        {4, 3, 5},
        {5, 3, 6},
        {6, 1, 0}
    };

    HuffmanFrequencyBuilder builder(8, 16);
    for(size_t i=0; i<n_symbols; ++i) {
        builder.SetSymbolFrequency(symbols[i], freqs[i]);
    }

    HuffmanEncoding *encoding = builder.Build();
    ASSERT_TRUE(encoding != nullptr);

    for(size_t i=0; i<n_symbols; ++i) {
        EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete encoding;
}

TEST(HuffmanFrequencyBuilderTest, BuildWithNoSymbol) {
    HuffmanFrequencyBuilder builder(8, 8);
    HuffmanEncoding *encoding = builder.Build();

    for(uint16_t i=0; i<8; ++i) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(i);
        EXPECT_TRUE(enc == nullptr);
    }
}

TEST(HuffmanBitLengthBuilderTest, Build) {
    const uint16_t symbols[] = {1, 2, 3, 4, 5, 6};
    const uint8_t bit_lengths[] = {4, 4, 3, 3, 3, 1};
    const size_t n_symbols = sizeof(symbols) / sizeof(symbols[0]);
    const EncodedSymbol expected_enc[] = {
        {1, 4, 14},
        {2, 4, 15},
        {3, 3, 4},
        {4, 3, 5},
        {5, 3, 6},
        {6, 1, 0}
    };

    HuffmanBitLengthBuilder builder(8, 16);
    for(size_t i=0; i<n_symbols; ++i) {
        builder.SetSymbolBitLength(symbols[i], bit_lengths[i]);
    }

    HuffmanEncoding *encoding = builder.Build();
    ASSERT_TRUE(encoding != nullptr);

    for(size_t i=0; i<n_symbols; ++i) {
        EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete encoding;
}

TEST(HuffmanBitLengthBuilderTest, BuildWithNoSymbol) {
    HuffmanBitLengthBuilder builder(8, 8);
    HuffmanEncoding *encoding = builder.Build();

    for(uint16_t i=0; i<8; ++i) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(i);
        EXPECT_TRUE(enc == nullptr);
    }
}

