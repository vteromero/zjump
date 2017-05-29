/**
    Copyright (c) 2017 Vicente Romero. All rights reserved.
    Licensed under the MIT License.
    See LICENSE file in the project root for full license information.
*/

#include "huffman.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <vector>

#include "mem.h"

using namespace std;

struct HuffmanTreeNode {
    uint16_t symbol;
    uint32_t count;
    HuffmanTreeNode *left;
    HuffmanTreeNode *right;
};

struct HuffmanTreeNodeCompare {
    bool operator()(const HuffmanTreeNode& lhs, const HuffmanTreeNode& rhs) {
        return lhs.count > rhs.count;
    }
} HuffmanTreeNodeCompareObj;

struct HuffmanTreeNodePtrCompare {
    bool operator()(const HuffmanTreeNode* lhs, const HuffmanTreeNode* rhs) {
        return lhs->count > rhs->count;
    }
};

struct CanonicalOrderCompare {
    bool operator()(const EncodedSymbol& lhs, const EncodedSymbol& rhs) {
        return (lhs.enc_bit_length < rhs.enc_bit_length) ||
            (lhs.enc_bit_length == rhs.enc_bit_length && lhs.symbol < rhs.symbol);
    }
} CanonicalOrderCompareObj;

static size_t MaxTreeSize(const size_t num_symbols) {
    return num_symbols * 2;
}

static void CreateTree(const size_t num_symbols,
                       const uint16_t* symbols,
                       const uint32_t* freqs,
                       HuffmanTreeNode* tree,
                       size_t* tree_size) {
    priority_queue<HuffmanTreeNode*,
                   vector<HuffmanTreeNode*>,
                   HuffmanTreeNodePtrCompare> pq;

    for(size_t i=0; i<num_symbols; ++i) {
        tree[i] = {symbols[i], freqs[i], nullptr, nullptr};
    }

    sort(tree, tree + num_symbols, HuffmanTreeNodeCompareObj);

    for(size_t i=0; i<num_symbols; ++i) {
        pq.push(&tree[i]);
    }

    size_t n = num_symbols;

    while(!pq.empty()) {
        if(pq.size() == 1)
            break;

        HuffmanTreeNode *node1 = pq.top(); pq.pop();
        HuffmanTreeNode *node2 = pq.top(); pq.pop();
        tree[n] = {0, node1->count + node2->count, node1, node2};

        pq.push(&tree[n]);

        ++n;
    }

    *tree_size = n;
}

static void SetBitLengths(const size_t num_symbols,
                          const HuffmanTreeNode* tree,
                          const size_t tree_size,
                          EncodedSymbol* enc_symbols) {
    if(num_symbols == 1) {
        enc_symbols[0].enc_bit_length = 1;
        return;
    }

    uint8_t *depth = SecureAlloc<uint8_t>(tree_size);

    for(size_t i=0; i<tree_size; ++i) {
        depth[i] = 0;
    }

    for(size_t i=tree_size; i-->0; ) {
        if(tree[i].left != nullptr) {
            size_t j = static_cast<size_t>(tree[i].left - tree);
            depth[j] = depth[i] + 1;
        }

        if(tree[i].right != nullptr) {
            size_t j = static_cast<size_t>(tree[i].right - tree);
            depth[j] = depth[i] + 1;
        }
    }

    for(size_t i=0; i<num_symbols; ++i) {
        enc_symbols[i].enc_bit_length = depth[i];
    }

    SecureFree<uint8_t>(depth);
}

// Based on: http://cbloomrants.blogspot.co.uk/2010/07/07-03-10-length-limitted-huffman-codes.html
// To improve: http://fastcompression.blogspot.co.uk/2015/07/huffman-revisited-part-3-depth-limited.html
static void SetMaxBitLength(const size_t num_symbols,
                            const uint8_t max_bit_length,
                            EncodedSymbol* enc_symbols) {
    uint32_t k = 0;
    uint32_t max_k = 1 << max_bit_length;

    for(size_t i=0; i<num_symbols; ++i) {
        if(enc_symbols[i].enc_bit_length > max_bit_length) {
            enc_symbols[i].enc_bit_length = max_bit_length;
        }

        k += 1 << (max_bit_length - enc_symbols[i].enc_bit_length);
    }

    for(size_t i=0; i<num_symbols; ++i) {
        if(enc_symbols[i].enc_bit_length >= max_bit_length || k <= max_k) {
            break;
        }
        ++enc_symbols[i].enc_bit_length;
        k -= 1 << (max_bit_length - enc_symbols[i].enc_bit_length);
    }

    for(size_t i=num_symbols; i-->0; ) {
        uint32_t inc = 1 << (max_bit_length - enc_symbols[i].enc_bit_length);
        if(k + inc >= max_k) {
            break;
        }
        k += inc;
        --enc_symbols[i].enc_bit_length;
    }

    // TODO: check k == max_k
}

static void SetCanonicalOrder(const size_t num_symbols,
                              EncodedSymbol* enc_symbols) {
    sort(enc_symbols, enc_symbols + num_symbols, CanonicalOrderCompareObj);
}

static void SetEncodedValues(const size_t num_symbols,
                             const uint8_t max_bit_length,
                             EncodedSymbol* enc_symbols) {
    uint16_t bl_count[max_bit_length + 1];
    uint16_t next_code[max_bit_length + 1];
    uint32_t code = 0;

    fill_n(bl_count, max_bit_length + 1, 0);

    for(size_t i=0; i<num_symbols; ++i) {
        ++bl_count[enc_symbols[i].enc_bit_length];
    }

    bl_count[0] = 0;
    next_code[0] = 0;
    for(size_t i=1; i<=max_bit_length; ++i) {
        code = (code + bl_count[i - 1]) << 1;
        next_code[i] = static_cast<uint16_t>(code);
    }

    for(size_t i=0; i<num_symbols; ++i) {
        enc_symbols[i].enc_value = next_code[enc_symbols[i].enc_bit_length]++;
    }
}

// HuffmanEncoding -------------------------------------------------------------

HuffmanEncoding::HuffmanEncoding(const uint16_t max_symbols,
                                 const uint8_t max_bit_length) {
    enc_symbols_ = SecureAlloc<EncodedSymbol>(max_symbols);
    for(size_t i=0; i<max_symbols; ++i) {
        enc_symbols_[i] = EncodedSymbol(i);
    }

    max_symbols_ = max_symbols;
    max_bit_length_ = max_bit_length;
}

HuffmanEncoding::~HuffmanEncoding() {
    SecureFree<EncodedSymbol>(enc_symbols_);
}

void HuffmanEncoding::SetEncodedSymbols(const EncodedSymbol* enc_symbols,
                                        const size_t length) {
    for(size_t i=0; i<length; ++i) {
        assert(enc_symbols[i].symbol < max_symbols_);
        assert(enc_symbols[i].enc_bit_length <= max_bit_length_);
        enc_symbols_[enc_symbols[i].symbol] = enc_symbols[i];
    }
}

EncodedSymbol* HuffmanEncoding::GetEncodedSymbol(const uint16_t symbol) const {
    assert(symbol < max_symbols_);
    if(enc_symbols_[symbol].enc_bit_length == 0) {
        return nullptr;
    }
    return &(enc_symbols_[symbol]);
}
    
uint16_t HuffmanEncoding::MaxSymbols() const {
    return max_symbols_;
}

uint8_t HuffmanEncoding::MaxBitLength() const {
    return max_bit_length_;
}

// HuffmanFrequencyBuilder -----------------------------------------------------

HuffmanFrequencyBuilder::HuffmanFrequencyBuilder(const uint16_t max_symbols,
                                                 const uint8_t max_bit_length) {
    symbol_freqs_ = SecureAlloc<uint32_t>(max_symbols);

    for(size_t i=0; i<max_symbols; ++i) {
        symbol_freqs_[i] = 0;
    }

    max_symbols_ = max_symbols;
    max_bit_length_ = max_bit_length;
}

HuffmanFrequencyBuilder::~HuffmanFrequencyBuilder() {
    SecureFree<uint32_t>(symbol_freqs_);
}

void HuffmanFrequencyBuilder::SetSymbolFrequency(const uint16_t symbol,
                                                 const uint32_t freq) {
    assert(symbol < max_symbols_);
    symbol_freqs_[symbol] = freq;
}

void HuffmanFrequencyBuilder::AddSymbolFrequency(const uint16_t symbol,
                                                 const uint32_t freq) {
    assert(symbol < max_symbols_);
    symbol_freqs_[symbol] += freq;
}

HuffmanEncoding* HuffmanFrequencyBuilder::Build() {
    size_t num_symbols = 0;
    uint16_t *symbols = SecureAlloc<uint16_t>(max_symbols_);
    uint32_t *freqs = SecureAlloc<uint32_t>(max_symbols_);

    for(uint16_t i=0; i<max_symbols_; ++i) {
        if(symbol_freqs_[i]) {
            symbols[num_symbols] = i;
            freqs[num_symbols] = symbol_freqs_[i];
            ++num_symbols;
        }
    }

    HuffmanEncoding *encoding = new HuffmanEncoding(max_symbols_, max_bit_length_);

    if(num_symbols > 0) {
        HuffmanTreeNode *tree = SecureAlloc<HuffmanTreeNode>(MaxTreeSize(max_symbols_));
        EncodedSymbol *enc_symbols = SecureAlloc<EncodedSymbol>(num_symbols);
        size_t tree_size;

        CreateTree(num_symbols, symbols, freqs, tree, &tree_size);

        for(size_t i=0; i<num_symbols; ++i) {
            enc_symbols[i] = EncodedSymbol(tree[i].symbol);
        }

        SetBitLengths(num_symbols, tree, tree_size, enc_symbols);

        SetMaxBitLength(num_symbols, max_bit_length_, enc_symbols);

        SetCanonicalOrder(num_symbols, enc_symbols);

        SetEncodedValues(num_symbols, max_bit_length_, enc_symbols);

        encoding->SetEncodedSymbols(enc_symbols, num_symbols);

        SecureFree<HuffmanTreeNode>(tree);
        SecureFree<EncodedSymbol>(enc_symbols);
    }

    SecureFree<uint16_t>(symbols);
    SecureFree<uint32_t>(freqs);

    return encoding;
}

// HuffmanBitLengthBuilder -----------------------------------------------------

HuffmanBitLengthBuilder::HuffmanBitLengthBuilder(const uint16_t max_symbols,
                                                 const uint8_t max_bit_length) {
    bit_lengths_ = SecureAlloc<uint8_t>(max_symbols);

    for(size_t i=0; i<max_symbols; ++i) {
        bit_lengths_[i] = 0;
    }

    max_symbols_ = max_symbols;
    max_bit_length_ = max_bit_length;
}

HuffmanBitLengthBuilder::~HuffmanBitLengthBuilder() {
    SecureFree<uint8_t>(bit_lengths_);
}

void HuffmanBitLengthBuilder::SetSymbolBitLength(const uint16_t symbol,
                                                 const uint8_t bit_length) {
    assert(symbol < max_symbols_);
    assert(bit_length <= max_bit_length_);
    bit_lengths_[symbol] = bit_length;
}

HuffmanEncoding* HuffmanBitLengthBuilder::Build() {
    size_t num_symbols = 0;
    uint16_t *symbols = SecureAlloc<uint16_t>(max_symbols_);
    uint8_t *lengths = SecureAlloc<uint8_t>(max_symbols_);

    for(uint16_t i=0; i<max_symbols_; ++i) {
        if(bit_lengths_[i]) {
            symbols[num_symbols] = i;
            lengths[num_symbols] = bit_lengths_[i];
            ++num_symbols;
        }
    }

    HuffmanEncoding *encoding = new HuffmanEncoding(max_symbols_, max_bit_length_);

    if(num_symbols > 0) {
        EncodedSymbol *enc_symbols = SecureAlloc<EncodedSymbol>(num_symbols);

        for(size_t i=0; i<num_symbols; ++i) {
            enc_symbols[i].symbol = symbols[i];
            enc_symbols[i].enc_bit_length = lengths[i];
        }

        SetEncodedValues(num_symbols, max_bit_length_, enc_symbols);

        encoding->SetEncodedSymbols(enc_symbols, num_symbols);

        SecureFree<EncodedSymbol>(enc_symbols);
    }

    SecureFree<uint16_t>(symbols);
    SecureFree<uint8_t>(lengths);

    return encoding;
}

// HuffmanWriter ---------------------------------------------------------------

HuffmanWriter::HuffmanWriter(const HuffmanEncoding& huff_tree) :
    huff_tree_(huff_tree) {
    writer_ = nullptr;
    encoding_type_ = 0;
    range_size_ = huff_tree.MaxSymbols();
    range_flags_ = SecureAlloc<uint8_t>(huff_tree.MaxSymbols());
    range_flags_size_ = 0;
}

HuffmanWriter::~HuffmanWriter() {
    SecureFree<uint8_t>(range_flags_);
}

bool HuffmanWriter::Write(BitStreamWriter* writer) {
    assert(writer != nullptr);

    writer_ = writer;
    range_flags_size_ = 0;

    encoding_type_ = EncodingType();

    return WriteEncodingType() &&
           WriteFlags() &&
           WriteSymbolBitLengths();
}

uint8_t HuffmanWriter::EncodingType() {
    const size_t symbols = huff_tree_.MaxSymbols();
    size_t sum[symbols + 1] = {0};

    for(size_t i=0; i<symbols; ++i) {
        sum[i + 1] = sum[i] + (huff_tree_.GetEncodedSymbol(i) != nullptr);
    }

    size_t enc_type = 0;
    size_t enc_len = symbols;

    size_t enc_len_1 = EncodingLength(symbols, 8, sum);
    if(enc_len_1 < enc_len) {
        enc_type = 1;
        enc_len = enc_len_1;
    }

    size_t enc_len_2 = EncodingLength(symbols, 16, sum);
    if(enc_len_2 < enc_len) {
        enc_type = 2;
        enc_len = enc_len_2;
    }

    size_t enc_len_3 = EncodingLength(symbols, 32, sum);
    if(enc_len_3 < enc_len) {
        enc_type = 3;
        enc_len = enc_len_3;
    }

    return enc_type;
}

size_t HuffmanWriter::EncodingLength(size_t num_symbols,
                                     size_t range_size,
                                     size_t* sum_symbols) {
    size_t enc_len = (num_symbols / range_size) + ((num_symbols % range_size) > 0);

    for(size_t i=0; i<=num_symbols; i+=range_size) {
        size_t remaining = (i + range_size > num_symbols) ? (num_symbols - i) : range_size;
        size_t symbols_in_range = sum_symbols[i + remaining] - sum_symbols[i];
        enc_len += remaining * (symbols_in_range > 0);
    }

    return enc_len;
}

bool HuffmanWriter::WriteEncodingType() {
    if(writer_->Append(encoding_type_, 2) == 2) {
        return true;
    } else {
        return false;
    }
}

bool HuffmanWriter::WriteFlags() {
    switch(encoding_type_) {
        case 0:
            return WriteSymbolFlags(0, huff_tree_.MaxSymbols());
        case 1:
            range_size_ = 8;
            return WriteRangeFlags() &&
                   WriteSymbolFlags();
        case 2:
            range_size_ = 16;
            return WriteRangeFlags() &&
                   WriteSymbolFlags();
        case 3:
            range_size_ = 32;
            return WriteRangeFlags() &&
                   WriteSymbolFlags();
    }

    return false;
}

bool HuffmanWriter::WriteRangeFlags() {
    const size_t n_symbols = huff_tree_.MaxSymbols();

    for(size_t i=0; i<n_symbols; ) {
        uint8_t flag = 0;

        for(size_t j=0; (i<n_symbols && j<range_size_); ++j, ++i) {
            flag = flag | static_cast<uint8_t>(huff_tree_.GetEncodedSymbol(i) != nullptr);
        }

        if(writer_->Append(flag, 1) != 1) {
            return false;
        }

        range_flags_[range_flags_size_++] = flag;
    }

    return true;
}

bool HuffmanWriter::WriteSymbolFlags() {
    for(size_t i=0; i<range_flags_size_; ++i) {
        if(range_flags_[i]) {
            if(!WriteSymbolFlags(i * range_size_, range_size_)) {
                return false;
            }
        }
    }

    return true;
}

bool HuffmanWriter::WriteSymbolFlags(size_t start,
                                     size_t length) {
    size_t end = start + length;
    if(end > huff_tree_.MaxSymbols()) {
        end = huff_tree_.MaxSymbols();
    }

    for(size_t s=start; s<end; ++s) {
        uint8_t flag = static_cast<uint8_t>(huff_tree_.GetEncodedSymbol(s) != nullptr);
        if(writer_->Append(flag, 1) != 1) {
            return false;
        }
    }

    return true;
}

bool HuffmanWriter::WriteSymbolBitLengths() {
    const size_t n_symbols = huff_tree_.MaxSymbols();

    // calculate the minimum size to encode a symbol bit length
    size_t max_bit_length = huff_tree_.MaxBitLength();
    size_t bit_length_field_size = 0;
    while(max_bit_length) {
        ++bit_length_field_size;
        max_bit_length >>= 1;
    }

    for(size_t s=0; s<n_symbols; ++s) {
        const EncodedSymbol *enc = huff_tree_.GetEncodedSymbol(s);
        if(enc != nullptr) {
            uint8_t written = writer_->Append(enc->enc_bit_length, bit_length_field_size);
            if(written != bit_length_field_size) {
                return false;
            }
        }
    }

    return true;
}

// HuffmanReader ---------------------------------------------------------------

constexpr int HuffmanReader::NO_ERROR;
constexpr int HuffmanReader::ERROR_BIT_STREAM;
constexpr int HuffmanReader::ERROR_HUFFMAN;

HuffmanReader::HuffmanReader(BitStreamReader& reader,
                             const uint16_t max_symbols,
                             const uint8_t max_bit_length) :
    reader_(reader), max_symbols_(max_symbols), max_bit_length_(max_bit_length) {
    assert(max_symbols > 0);
    assert(max_bit_length > 0);
    encoding_type_ = 0;
    symbols_ = SecureAlloc<uint16_t>(max_symbols);
    bit_lengths_ = SecureAlloc<uint8_t>(max_symbols);
    num_symbols_ = 0;
}

HuffmanReader::~HuffmanReader() {
    SecureFree<uint16_t>(symbols_);
    SecureFree<uint8_t>(bit_lengths_);
}

int HuffmanReader::Read(HuffmanEncoding** huff_tree) {
    num_symbols_ = 0;

    if(!ReadEncodingType()) {
        return ERROR_BIT_STREAM;
    }

    if(!ReadFlags()) {
        return ERROR_BIT_STREAM;
    }

    if(!ReadSymbolBitLengths()) {
        return ERROR_BIT_STREAM;
    }

    HuffmanBitLengthBuilder builder(max_symbols_, max_bit_length_);

    for(size_t i=0; i<num_symbols_; ++i) {
        builder.SetSymbolBitLength(symbols_[i], bit_lengths_[i]);
    }

    *huff_tree = builder.Build();
    if(*huff_tree == nullptr) {
        return ERROR_HUFFMAN;
    }

    return NO_ERROR;
}

bool HuffmanReader::ReadEncodingType() {
    return (reader_.ReadNext(2, &encoding_type_) == 2);
}

bool HuffmanReader::ReadFlags() {
    switch(encoding_type_) {
        case 0:
            return ReadSymbolFlags(0, max_symbols_);
        case 1:
            return ReadRangeFlags(8);
        case 2:
            return ReadRangeFlags(16);
        case 3:
            return ReadRangeFlags(32);
    }

    return false;
}

bool HuffmanReader::ReadRangeFlags(size_t range_size) {
    const size_t num_range_flags = (max_symbols_ / range_size) + ((max_symbols_ % range_size) > 0);
    size_t range_flags_pos = reader_.NextPos();

    // move the next-position-to-read to the position from where symbol flags
    // are going to be read.
    reader_.MoveTo(range_flags_pos + num_range_flags);

    for(size_t i=0, j=0; i<num_range_flags; ++i, j+=range_size) {
        uint8_t flag = 0;
        if(reader_.Read(1, range_flags_pos, &flag) != 1) {
            return false;
        }

        if((flag == 1) && (!ReadSymbolFlags(j, range_size))) {
            return false;
        }

        ++range_flags_pos;
    }

    return true;
}

bool HuffmanReader::ReadSymbolFlags(size_t start,
                                    size_t length) {
    size_t end = start + length;
    if(end > max_symbols_) {
        end = max_symbols_;
    }

    for(size_t s=start; s<end; ++s) {
        uint8_t flag;

        if(reader_.ReadNext(1, &flag) != 1) {
            return false;
        }

        if(flag == 1) {
            symbols_[num_symbols_++] = s;
        }
    }

    return true;
}

bool HuffmanReader::ReadSymbolBitLengths() {
    size_t bit_length_field_size = 0;
    size_t x = max_bit_length_;
    while(x) {
        ++bit_length_field_size;
        x >>= 1;
    }

    for(size_t i=0; i<num_symbols_; ++i) {
        if(reader_.ReadNext(bit_length_field_size, &bit_lengths_[i]) != bit_length_field_size) {
            return false;
        }
    }

    return true;
}

