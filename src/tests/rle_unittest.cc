#include "gtest/gtest.h"

#include "../constants.h"
#include "../mem.h"
#include "../rle.h"

TEST(Rle1Test, Rle1WithNoOnes) {
    size_t data_size = 10;
    uint16_t data[data_size] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    const size_t expected_size = data_size;
    const uint16_t expected[expected_size] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 0};

    Rle1(data, data_size, data, &data_size);

    EXPECT_EQ(data_size, expected_size);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(Rle1Test, Rle1WithOnesOnly) {
    size_t data_size = 25;
    uint16_t data[data_size] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const size_t expected_size = 4;
    const uint16_t expected[expected_size] = {kRUNASymbol, kRUNBSymbol, kRUNASymbol, kRUNBSymbol};

    Rle1(data, data_size, data, &data_size);

    EXPECT_EQ(data_size, expected_size);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(Rle1Test, Rle1WithMixedData) {
    size_t data_size = 60;
    uint16_t data[data_size] = {
         1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        10,  5, 20,  1, 11,  1,  1,  1,  1,  3,  3,  5, 45,  1,  1,  9,  1,  8, 22, 13,
         1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  7,  9,  1,  1,  1
    };
    const size_t expected_size = 30;
    const uint16_t expected[expected_size] = {
         kRUNBSymbol,  kRUNASymbol,  kRUNBSymbol,  kRUNASymbol, 10,  5, 20,  kRUNASymbol, 11,
         kRUNBSymbol,  kRUNASymbol,  3,  3,  5, 45,  kRUNBSymbol,  9,  kRUNASymbol,  8, 22,
        13,  kRUNASymbol,  kRUNBSymbol,  kRUNBSymbol,  3,  kRUNASymbol,  7,  9,  kRUNASymbol,
         kRUNASymbol
    };

    Rle1(data, data_size, data, &data_size);

    EXPECT_EQ(data_size, expected_size);

    for(size_t i=0; i<data_size; ++i) {
        EXPECT_EQ(expected[i], data[i]);
    }
}

TEST(Rle1Test, InverseRle1WithNoRle) {
    const size_t in_data_size = 10;
    const uint16_t in_data[in_data_size] = {3, 4, 5, 6, 7, 8, 9, 3, 4, 5};
    const size_t expected_size = in_data_size;
    const uint16_t expected[expected_size] = {3, 4, 5, 6, 7, 8, 9, 3, 4, 5};
    uint16_t *out_data = SecureAlloc<uint16_t>(expected_size);
    size_t out_data_size = 0;

    InverseRle1(in_data, in_data_size, out_data, &out_data_size);

    EXPECT_EQ(out_data_size, expected_size);

    for(size_t i=0; i<out_data_size; ++i) {
        EXPECT_EQ(expected[i], out_data[i]);
    }

    SecureFree<uint16_t>(out_data);
}

TEST(Rle1Test, InverseRle1WithRleOnly) {
    const size_t in_data_size = 4;
    const uint16_t in_data[in_data_size] = {kRUNASymbol, kRUNBSymbol, kRUNASymbol, kRUNBSymbol};
    const size_t expected_size = 25;
    const uint16_t expected[expected_size] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    uint16_t *out_data = SecureAlloc<uint16_t>(expected_size);
    size_t out_data_size = 0;

    InverseRle1(in_data, in_data_size, out_data, &out_data_size);

    EXPECT_EQ(out_data_size, expected_size);

    for(size_t i=0; i<out_data_size; ++i) {
        EXPECT_EQ(expected[i], out_data[i]);
    }

    SecureFree<uint16_t>(out_data);
}

TEST(Rle1Test, InverseRle1WithMixedData) {
    const size_t in_data_size = 30;
    const uint16_t in_data[in_data_size] = {
         kRUNBSymbol,  kRUNASymbol,  kRUNBSymbol,  kRUNASymbol, 10,  5, 20,  kRUNASymbol, 11,
         kRUNBSymbol,  kRUNASymbol,  3,  3,  5, 45,  kRUNBSymbol,  9,  kRUNASymbol,  8, 22,
        13,  kRUNASymbol,  kRUNBSymbol,  kRUNBSymbol,  3,  kRUNASymbol,  7,  9,  kRUNASymbol,
         kRUNASymbol
    };
    const size_t expected_size = 60;
    const uint16_t expected[expected_size] = {
         1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        10,  5, 20,  1, 11,  1,  1,  1,  1,  3,  3,  5, 45,  1,  1,  9,  1,  8, 22, 13,
         1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  1,  7,  9,  1,  1,  1
    };
    uint16_t *out_data = SecureAlloc<uint16_t>(expected_size);
    size_t out_data_size = 0;

    InverseRle1(in_data, in_data_size, out_data, &out_data_size);

    EXPECT_EQ(out_data_size, expected_size);

    for(size_t i=0; i<out_data_size; ++i) {
        EXPECT_EQ(expected[i], out_data[i]);
    }

    SecureFree<uint16_t>(out_data);
}

