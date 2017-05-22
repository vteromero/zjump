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

