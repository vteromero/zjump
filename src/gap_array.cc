#include "gap_array.h"

using namespace std;

GapArray::GapArray(uint8_t* array, size_t array_length) {
    array_ = array;
    array_length_ = array_length;

    Clear();
}

bool GapArray::Set(size_t index, uint8_t value) {
    GapArraySet::iterator it = gaps_.find({index, 0, 1});
    if(it == gaps_.end()) {
        return false;
    }

    size_t offset = index - it->start;
    if(offset >= it->length) {
        return false;
    }

    array_[it->orig_start + offset] = value;

    index_set_.insert(index);

    return true;
}

void GapArray::Reconstruct() {
    GapArraySet new_gaps;
    GapArraySet::iterator gap_it = gaps_.begin();
    set<size_t>::iterator idx_it = index_set_.begin();
    size_t new_start_index = 0;

    for(; gap_it!=gaps_.end(); ++gap_it) {
        GapArrayItem gap = *gap_it;
        size_t start = gap.start;
        size_t orig_start = gap.orig_start;
        size_t end = gap.start + gap.length;

        for(; idx_it!=index_set_.end(); ++idx_it) {
            size_t cur_idx = *idx_it;
            if(cur_idx >= end) {
                break;
            }

            size_t len = cur_idx - start;
            if(len > 0) {
                new_gaps.insert({new_start_index, orig_start, len});
                new_start_index += len;
            }

            orig_start += len + 1;
            start = cur_idx + 1;
        }

        size_t len = end - start;
        if(len > 0) {
            new_gaps.insert({new_start_index, orig_start, len});
            new_start_index += len;
        }
    }

    size_ = new_start_index;

    gaps_ = new_gaps;

    index_set_.clear();
}

size_t GapArray::Size() const {
    return size_;
}

void GapArray::Clear() {
    size_ = array_length_;

    gaps_.clear();
    gaps_.insert({0, 0, array_length_});

    index_set_.clear();
}

