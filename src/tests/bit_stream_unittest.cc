/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "gtest/gtest.h"

#include "../bit_stream.h"

TEST(BitStreamWriterTest, AppendBitsInAZeroStream) {
    const size_t data_size = 5;
    uint8_t data[data_size] = {0, 0, 0, 0, 0};
    uint8_t expected[data_size] = {0x6B, 0x28, 0xFF, 0xFF, 0x00};

    BitStreamWriter writer(data, data_size);
    writer.Append(3, 2);
    writer.Append(2, 3);
    writer.Append(3, 6);
    writer.Append(5, 5);
    writer.Append(0xFFFF, 16);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, AppendBitsInANotZeroStream) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x00, 0x01, 0x11, 0x11};
    uint8_t expected[data_size] = {0xFF, 0xFE, 0x00, 0x11};

    BitStreamWriter writer(data, data_size);
    writer.Append(0xFEFF, 16);
    writer.Append(0x00, 8);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, AppendABigValue) {
    const size_t data_size = 10;
    uint8_t data[data_size] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t expected[data_size] = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x0, 0x0, 0x0};

    BitStreamWriter writer(data, data_size);
    writer.Append(0x11223344556677, 56);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, WriteBitsInAZeroStream) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0, 0, 0, 0};
    uint8_t expected[data_size] = {0x65, 0xA4, 0x02, 0x00};

    BitStreamWriter writer(data, data_size);
    writer.Write(0xA4, 8, 8);
    writer.Write(5, 3, 16);
    writer.Write(2, 3, 16);
    writer.Write(0x65, 8, 0);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, WriteBitsInANotZeroStream) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x11, 0x11, 0x11, 0x11};
    uint8_t expected[data_size] = {0x1E, 0x00, 0xEE, 0x11};

    BitStreamWriter writer(data, data_size);
    writer.Write(0xE, 4, 0);
    writer.Write(0xE, 4, 16);
    writer.Write(0xE, 4, 20);
    writer.Write(0x00, 8, 8);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, WriteABigValue) {
    const size_t data_size = 10;
    uint8_t data[data_size] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t expected[data_size] = {0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x0, 0x0, 0x0};

    BitStreamWriter writer(data, data_size);
    writer.Write(0x11223344556677, 56, 0);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, WriteBitsAfterAppending) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0, 0, 0, 0};
    uint8_t expected[data_size] = {0x00, 0xFF, 0x00, 0x0};

    BitStreamWriter writer(data, data_size);
    writer.Append(10, 4);
    writer.Append(1, 4);
    writer.Append(3, 2);
    writer.Write(0xFF00, 16, 0);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(BitStreamWriterTest, Get) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0, 0, 0, 0};

    BitStreamWriter writer(data, data_size);
    writer.Append(0x03, 8);

    BitStream bit_stream = writer.Get();
    EXPECT_EQ(data_size, bit_stream.allocated);
    EXPECT_EQ(static_cast<size_t>(8), bit_stream.size);

    writer.Append(0x3, 4);
    BitStream bit_stream2 = writer.Get();
    EXPECT_EQ(data_size, bit_stream2.allocated);
    EXPECT_EQ(static_cast<size_t>(12), bit_stream2.size);
}

TEST(BitStreamReaderTest, Read8BitValues) {
    const size_t data_size = 3;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11};
    uint8_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(3, 0, &result);
    EXPECT_EQ(result, 5);
    EXPECT_EQ(num_bits, 3);

    num_bits = reader.Read(3, 3, &result);
    EXPECT_EQ(result, 4);
    EXPECT_EQ(num_bits, 3);

    num_bits = reader.Read(2, 6, &result);
    EXPECT_EQ(result, 3);
    EXPECT_EQ(num_bits, 2);

    num_bits = reader.Read(8, 8, &result);
    EXPECT_EQ(result, 0xFF);
    EXPECT_EQ(num_bits, 8);

    num_bits = reader.Read(8, 16, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);
}

TEST(BitStreamReaderTest, Read16BitValues) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11, 0xAA};
    uint16_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(16, 0, &result);
    EXPECT_EQ(result, 0xFFE5);
    EXPECT_EQ(num_bits, 16);

    num_bits = reader.Read(16, 16, &result);
    EXPECT_EQ(result, 0xAA11);
    EXPECT_EQ(num_bits, 16);
}

TEST(BitStreamReaderTest, Read32BitValues) {
    const size_t data_size = 8;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11, 0xAA, 0xFF, 0x44, 0x67, 0x91};
    uint32_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(32, 0, &result);
    EXPECT_EQ(result, 0xAA11FFE5);
    EXPECT_EQ(num_bits, 32);

    num_bits = reader.Read(32, 32, &result);
    EXPECT_EQ(result, 0x916744FF);
    EXPECT_EQ(num_bits, 32);
}

TEST(BitStreamReaderTest, ReadNext8BitValues) {
    const size_t data_size = 3;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11};
    uint8_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.ReadNext(3, &result);
    EXPECT_EQ(result, 5);
    EXPECT_EQ(num_bits, 3);

    num_bits = reader.ReadNext(3, &result);
    EXPECT_EQ(result, 4);
    EXPECT_EQ(num_bits, 3);

    num_bits = reader.ReadNext(2, &result);
    EXPECT_EQ(result, 3);
    EXPECT_EQ(num_bits, 2);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0xFF);
    EXPECT_EQ(num_bits, 8);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);
}

TEST(BitStreamReaderTest, ReadNext16BitValues) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11, 0xAA};
    uint16_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.ReadNext(16, &result);
    EXPECT_EQ(result, 0xFFE5);
    EXPECT_EQ(num_bits, 16);

    num_bits = reader.ReadNext(16, &result);
    EXPECT_EQ(result, 0xAA11);
    EXPECT_EQ(num_bits, 16);
}

TEST(BitStreamReaderTest, ReadNext32BitValues) {
    const size_t data_size = 8;
    uint8_t data[data_size] = {0xE5, 0xFF, 0x11, 0xAA, 0xFF, 0x44, 0x67, 0x91};
    uint32_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.ReadNext(32, &result);
    EXPECT_EQ(result, 0xAA11FFE5);
    EXPECT_EQ(num_bits, 32);

    num_bits = reader.ReadNext(32, &result);
    EXPECT_EQ(result, 0x916744FF);
    EXPECT_EQ(num_bits, 32);
}

TEST(BitStreamReaderTest, ReadNextAndReset) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x11, 0x22, 0x33, 0x44};
    uint8_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x22);
    EXPECT_EQ(num_bits, 8);

    reader.Reset();

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);
}

TEST(BitStreamReaderTest, ReadNextAndMoveTo) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x11, 0x22, 0x33, 0x44};
    uint8_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);

    reader.MoveTo(16);
    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x33);
    EXPECT_EQ(num_bits, 8);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x44);
    EXPECT_EQ(num_bits, 8);

    reader.MoveTo(8);
    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x22);
    EXPECT_EQ(num_bits, 8);
}

TEST(BitStreamReaderTest, NextPos) {
    const size_t data_size = 4;
    uint8_t data[data_size] = {0x11, 0x22, 0x33, 0x44};
    uint8_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(8, 0, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);
    EXPECT_EQ(reader.NextPos(), 0u);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x11);
    EXPECT_EQ(num_bits, 8);
    EXPECT_EQ(reader.NextPos(), 8u);

    num_bits = reader.ReadNext(8, &result);
    EXPECT_EQ(result, 0x22);
    EXPECT_EQ(num_bits, 8);
    EXPECT_EQ(reader.NextPos(), 16u);
}

TEST(BitStreamReaderTest, ReadABigValue) {
    const size_t data_size = 7;
    uint8_t data[data_size] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    uint64_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(56, 0, &result);
    EXPECT_EQ(result, 0x77665544332211ULL);
    EXPECT_EQ(num_bits, 56);
}

TEST(BitStreamReaderTest, ReadABigValueWithAShift) {
    const size_t data_size = 8;
    uint8_t data[data_size] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint64_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(56, 4, &result);
    EXPECT_EQ(result, 0x87766554433221ULL);
    EXPECT_EQ(num_bits, 56);
}

TEST(BitStreamReaderTest, TryToReadAValueBiggerThanPossible) {
    const size_t data_size = 1;
    uint8_t data[data_size] = {0x11};
    uint64_t result;
    uint8_t num_bits;

    BitStreamReader reader(data, data_size);

    num_bits = reader.Read(16, 0, &result);
    EXPECT_EQ(result, 0x11ULL);
    EXPECT_EQ(num_bits, 8);
}

