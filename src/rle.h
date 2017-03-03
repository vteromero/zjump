/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#ifndef RLE_H_
#define RLE_H_

#include <cstddef>
#include <cstdint>

void Rle1(const uint16_t* in,
          const size_t in_size,
          uint16_t* out,
          size_t* out_size);

void InverseRle1(const uint16_t* in,
                 const size_t in_size,
                 uint16_t* out,
                 size_t* out_size);

#endif // RLE_H_

