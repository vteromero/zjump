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
        const EncodedSymbol *enc_symbol = he.GetEncodedSymbol(enc_symbols[i].symbol);
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
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete encoding;
}

TEST(HuffmanFrequencyBuilderTest, BuildWithOneSymbolOnly) {
    const uint16_t symbols[] = {0};
    const uint32_t freqs[] = {1};
    const size_t n_symbols = sizeof(symbols) / sizeof(symbols[0]);
    const EncodedSymbol expected_enc[] = {
        {0, 1, 0}
    };

    HuffmanFrequencyBuilder builder(8, 16);
    for(size_t i=0; i<n_symbols; ++i) {
        builder.SetSymbolFrequency(symbols[i], freqs[i]);
    }

    HuffmanEncoding *encoding = builder.Build();
    ASSERT_TRUE(encoding != nullptr);

    for(size_t i=0; i<n_symbols; ++i) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

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

    delete encoding;
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
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete encoding;
}

TEST(HuffmanBitLengthBuilderTest, BuildWithOneSymbolOnly) {
    const uint16_t symbols[] = {0};
    const uint8_t bit_lengths[] = {1};
    const size_t n_symbols = sizeof(symbols) / sizeof(symbols[0]);
    const EncodedSymbol expected_enc[] = {
        {0, 1, 0}
    };

    HuffmanBitLengthBuilder builder(8, 16);
    for(size_t i=0; i<n_symbols; ++i) {
        builder.SetSymbolBitLength(symbols[i], bit_lengths[i]);
    }

    HuffmanEncoding *encoding = builder.Build();
    ASSERT_TRUE(encoding != nullptr);

    for(size_t i=0; i<n_symbols; ++i) {
        const EncodedSymbol *enc = encoding->GetEncodedSymbol(symbols[i]);

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

    delete encoding;
}

TEST(HuffmanWriterTest, EncodingType0) {
    const size_t data_size = 5;
    uint8_t data[data_size] = {0};
    uint8_t expected[data_size] = {0x8C, 0xCA, 0x4C, 0x00, 0x00};

    HuffmanFrequencyBuilder huff_builder(8, 15);
    huff_builder.SetSymbolFrequency(0, 10);
    huff_builder.SetSymbolFrequency(1, 4);
    huff_builder.SetSymbolFrequency(5, 6);
    huff_builder.SetSymbolFrequency(7, 12);
    HuffmanEncoding *huff_enc = huff_builder.Build();
    ASSERT_TRUE(huff_enc != nullptr);

    BitStreamWriter bit_stream_writer(data, data_size);

    HuffmanWriter huff_writer(*huff_enc);
    EXPECT_TRUE(huff_writer.Write(&bit_stream_writer));

    // encoding type: 2 first bits
    uint8_t enc_type = data[0] & 0x3;
    EXPECT_EQ(enc_type, 0u);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }

    EXPECT_EQ(bit_stream_writer.Get().size, 26u);

    delete huff_enc;
}

TEST(HuffmanWriterTest, EncodingType1) {
    const size_t data_size = 5;
    uint8_t data[data_size] = {0};
    uint8_t expected[data_size] = {0x35, 0x2A, 0x33, 0x01, 0x00};

    HuffmanFrequencyBuilder huff_builder(16, 15);
    huff_builder.SetSymbolFrequency(0, 10);
    huff_builder.SetSymbolFrequency(1, 4);
    huff_builder.SetSymbolFrequency(5, 6);
    huff_builder.SetSymbolFrequency(7, 12);
    HuffmanEncoding *huff_enc = huff_builder.Build();
    ASSERT_TRUE(huff_enc != nullptr);

    BitStreamWriter bit_stream_writer(data, data_size);

    HuffmanWriter huff_writer(*huff_enc);
    EXPECT_TRUE(huff_writer.Write(&bit_stream_writer));

    // encoding type: 2 first bits
    uint8_t enc_type = data[0] & 0x3;
    EXPECT_EQ(enc_type, 1u);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }

    EXPECT_EQ(bit_stream_writer.Get().size, 28u);

    delete huff_enc;
}

TEST(HuffmanWriterTest, EncodingType2) {
    const size_t data_size = 5;
    uint8_t data[data_size] = {0};
    uint8_t expected[data_size] = {0x0A, 0xA3, 0x20, 0x33, 0x01};

    HuffmanFrequencyBuilder huff_builder(32, 15);
    huff_builder.SetSymbolFrequency(20, 10);
    huff_builder.SetSymbolFrequency(21, 4);
    huff_builder.SetSymbolFrequency(25, 6);
    huff_builder.SetSymbolFrequency(27, 12);
    HuffmanEncoding *huff_enc = huff_builder.Build();
    ASSERT_TRUE(huff_enc != nullptr);

    BitStreamWriter bit_stream_writer(data, data_size);

    HuffmanWriter huff_writer(*huff_enc);
    EXPECT_TRUE(huff_writer.Write(&bit_stream_writer));

    // encoding type: 2 first bits
    uint8_t enc_type = data[0] & 0x3;
    EXPECT_EQ(enc_type, 2u);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }

    EXPECT_EQ(bit_stream_writer.Get().size, 36u);

    delete huff_enc;
}

TEST(HuffmanWriterTest, EncodingType3) {
    const size_t data_size = 8;
    uint8_t data[data_size] = {0};
    uint8_t expected[data_size] = {0x1B, 0x10, 0x40, 0x00, 0x21, 0x33, 0x01, 0x00};

    HuffmanFrequencyBuilder huff_builder(64, 15);
    huff_builder.SetSymbolFrequency(32, 10);
    huff_builder.SetSymbolFrequency(40, 4);
    huff_builder.SetSymbolFrequency(50, 6);
    huff_builder.SetSymbolFrequency(60, 12);
    HuffmanEncoding *huff_enc = huff_builder.Build();
    ASSERT_TRUE(huff_enc != nullptr);

    BitStreamWriter bit_stream_writer(data, data_size);

    HuffmanWriter huff_writer(*huff_enc);
    EXPECT_TRUE(huff_writer.Write(&bit_stream_writer));

    // encoding type: 2 first bits
    uint8_t enc_type = data[0] & 0x3;
    EXPECT_EQ(enc_type, 3u);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }

    EXPECT_EQ(bit_stream_writer.Get().size, 52u);

    delete huff_enc;
}

TEST(HuffmanReaderTest, EncodingType0) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x8C, 0xCA, 0x4C, 0x00};
    const uint16_t max_symbols = 8;
    const uint8_t max_symbol_bit_length = 15;
    HuffmanEncoding *huff_tree = nullptr;
    const size_t num_symbols = 4;
    const EncodedSymbol expected_enc[num_symbols] = {
        {0, 2, 2},
        {1, 3, 6},
        {5, 3, 7},
        {7, 1, 0}
    };

    BitStreamReader bit_reader(data, data_size);

    HuffmanReader huff_reader(bit_reader, max_symbols, max_symbol_bit_length);
    EXPECT_EQ(huff_reader.Read(&huff_tree), HuffmanReader::NO_ERROR);
    ASSERT_TRUE(huff_tree != nullptr);

    EXPECT_EQ(huff_tree->MaxSymbols(), max_symbols);
    EXPECT_EQ(huff_tree->MaxBitLength(), max_symbol_bit_length);

    for(size_t i=0; i<num_symbols; ++i) {
        const EncodedSymbol *enc = huff_tree->GetEncodedSymbol(expected_enc[i].symbol);
        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete huff_tree;
}

TEST(HuffmanReaderTest, EncodingType1) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x35, 0x2A, 0x33, 0x01};
    const uint16_t max_symbols = 16;
    const uint8_t max_symbol_bit_length = 15;
    HuffmanEncoding *huff_tree = nullptr;
    const size_t num_symbols = 4;
    const EncodedSymbol expected_enc[num_symbols] = {
        {0, 2, 2},
        {1, 3, 6},
        {5, 3, 7},
        {7, 1, 0}
    };

    BitStreamReader bit_reader(data, data_size);

    HuffmanReader huff_reader(bit_reader, max_symbols, max_symbol_bit_length);
    EXPECT_EQ(huff_reader.Read(&huff_tree), HuffmanReader::NO_ERROR);
    ASSERT_TRUE(huff_tree != nullptr);

    EXPECT_EQ(huff_tree->MaxSymbols(), max_symbols);
    EXPECT_EQ(huff_tree->MaxBitLength(), max_symbol_bit_length);

    for(size_t i=0; i<num_symbols; ++i) {
        const EncodedSymbol *enc = huff_tree->GetEncodedSymbol(expected_enc[i].symbol);
        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete huff_tree;
}

TEST(HuffmanReaderTest, EncodingType2) {
    const size_t data_size = 5;
    uint8_t data[data_size] = {0x0A, 0xA3, 0x20, 0x33, 0x01};
    const uint16_t max_symbols = 32;
    const uint8_t max_symbol_bit_length = 15;
    HuffmanEncoding *huff_tree = nullptr;
    const size_t num_symbols = 4;
    const EncodedSymbol expected_enc[num_symbols] = {
        {20, 2, 2},
        {21, 3, 6},
        {25, 3, 7},
        {27, 1, 0}
    };

    BitStreamReader bit_reader(data, data_size);

    HuffmanReader huff_reader(bit_reader, max_symbols, max_symbol_bit_length);
    EXPECT_EQ(huff_reader.Read(&huff_tree), HuffmanReader::NO_ERROR);
    ASSERT_TRUE(huff_tree != nullptr);

    EXPECT_EQ(huff_tree->MaxSymbols(), max_symbols);
    EXPECT_EQ(huff_tree->MaxBitLength(), max_symbol_bit_length);

    for(size_t i=0; i<num_symbols; ++i) {
        const EncodedSymbol *enc = huff_tree->GetEncodedSymbol(expected_enc[i].symbol);
        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete huff_tree;
}

TEST(HuffmanReaderTest, EncodingType3) {
    const size_t data_size = 8;
    uint8_t data[data_size] = {0x1B, 0x10, 0x40, 0x00, 0x21, 0x33, 0x01, 0x00};
    const uint16_t max_symbols = 64;
    const uint8_t max_symbol_bit_length = 15;
    HuffmanEncoding *huff_tree = nullptr;
    const size_t num_symbols = 4;
    const EncodedSymbol expected_enc[num_symbols] = {
        {32, 2, 2},
        {40, 3, 6},
        {50, 3, 7},
        {60, 1, 0}
    };

    BitStreamReader bit_reader(data, data_size);

    HuffmanReader huff_reader(bit_reader, max_symbols, max_symbol_bit_length);
    EXPECT_EQ(huff_reader.Read(&huff_tree), HuffmanReader::NO_ERROR);
    ASSERT_TRUE(huff_tree != nullptr);

    EXPECT_EQ(huff_tree->MaxSymbols(), max_symbols);
    EXPECT_EQ(huff_tree->MaxBitLength(), max_symbol_bit_length);

    for(size_t i=0; i<num_symbols; ++i) {
        const EncodedSymbol *enc = huff_tree->GetEncodedSymbol(expected_enc[i].symbol);
        ASSERT_TRUE(enc != nullptr);
        EXPECT_EQ(enc->symbol, expected_enc[i].symbol);
        EXPECT_EQ(enc->enc_bit_length, expected_enc[i].enc_bit_length);
        EXPECT_EQ(enc->enc_value, expected_enc[i].enc_value);
    }

    delete huff_tree;
}

