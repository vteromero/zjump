#ifndef GAP_ARRAY_H_
#define GAP_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <set>

struct GapArrayItem {
    size_t start;
    size_t orig_start;
    size_t length;
};

struct GapArrayItemCompare {
    bool operator()(const GapArrayItem& lhs, const GapArrayItem& rhs) const {
        return (lhs.start + lhs.length) <= rhs.start;
    }
};

typedef std::set<GapArrayItem, GapArrayItemCompare> GapArraySet;

class GapArray {
public:
    GapArray(uint8_t* array, size_t array_length);

    bool Set(size_t index, uint8_t value);

    void Reconstruct();

    size_t Size() const;

    void Clear();

private:
    uint8_t *array_;
    size_t array_length_;
    size_t size_;
    GapArraySet gaps_;
    std::set<size_t> index_set_;
};

#endif // GAP_ARRAY_H_

