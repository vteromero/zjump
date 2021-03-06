/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "rle.h"

#include <cassert>

#include "constants.h"

static void AppendRle1(uint32_t length, uint16_t* stream, size_t* stream_size) {
    uint32_t run_a = 1;
    uint32_t run_b = 2;
    size_t n = *stream_size;

    while(length) {
        if((length % run_b) == 0) {
            stream[n++] = kRUNBSymbol;
            length -= run_b;
        } else {
            stream[n++] = kRUNASymbol;
            length -= run_a;
        }

        run_a = run_b;
        run_b <<= 1;
    }

    *stream_size = n;
}

static void AppendOnes(const uint32_t length,
                       uint16_t* stream,
                       size_t* stream_size) {
    size_t n = *stream_size;

    for(size_t i=0; i<length; ++i) {
        stream[n++] = 1;
    }

    *stream_size = n;
}

static uint32_t EncodeOnes(const uint16_t* in,
                           const size_t in_size,
                           uint16_t* out,
                           size_t* out_size) {
    uint32_t length = 0;

    for(size_t i=0; ((i < in_size) && (in[i] == 1)); ++i) {
        ++length;
    }

    AppendRle1(length, out, out_size);

    return length;
}

static uint32_t DecodeRle1(const uint16_t* in,
                           const size_t in_size,
                           uint16_t* out,
                           size_t* out_size) {
    uint32_t p = 1;
    uint32_t length = 0;
    uint32_t i = 0;

    for(; i<in_size; ++i) {
        if(in[i] == kRUNASymbol) {
            length += p;
        } else if(in[i] == kRUNBSymbol) {
            length += (p << 1);
        } else {
            break;
        }
        p <<= 1;
    }

    AppendOnes(length, out, out_size);

    return i;
}

void Rle1(const uint16_t* in,
          const size_t in_size,
          uint16_t* out,
          size_t* out_size) {
    assert(in != nullptr);
    assert(out != nullptr);
    assert(in_size > 0);

    size_t n = 0;

    for(size_t i=0; i<in_size; ) {
        uint32_t len = EncodeOnes(&(in[i]), (in_size - i), out, &n);
        if(len) {
            i += len;
        } else {
            out[n++] = in[i++];
        }
    }

    *out_size = n;
}

void InverseRle1(const uint16_t* in,
                 const size_t in_size,
                 uint16_t* out,
                 size_t* out_size) {
    assert(in != nullptr);
    assert(out != nullptr);
    assert(in_size > 0);
    assert(in != out);

    size_t n = 0;

    for(size_t i=0; i<in_size; ) {
        uint32_t len = DecodeRle1(&(in[i]), (in_size - i), out, &n);
        if(len) {
            i += len;
        } else {
            out[n++] = in[i++];
        }
    }

    *out_size = n;
}

