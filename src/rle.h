#ifndef RLE_H_
#define RLE_H_

#include <cstddef>
#include <cstdint>

void Rle1(uint16_t* stream, size_t* stream_size);

void InverseRle1(const uint16_t* in,
                 const size_t in_size,
                 uint16_t* out,
                 size_t* out_size);

#endif // RLE_H_

