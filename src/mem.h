#ifndef MEM_H_
#define MEM_H_

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>

#include "constants.h"

template<typename T>
T* SecureAlloc(size_t size) {
    assert(size > 0);
    T *p = new (std::nothrow) T[size];
    if(p == nullptr) {
        exit(ZJUMP_ERROR_MEMORY_ALLOC);
    }
    return p;
}

template<typename T>
void SecureFree(T* address) {
    if(address != nullptr) {
        delete [] address;
        address = nullptr;
    }
}

#endif // MEM_H_

