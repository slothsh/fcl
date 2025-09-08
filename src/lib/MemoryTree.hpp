#pragma once

#include <cstddef>

class MemoryTree {
public:
    void insert(void* data, size_t size);
    MemoryTree* descendLeft();
    MemoryTree* descendRight();

private:
    void* m_data = nullptr;
    size_t m_size = 0;
    MemoryTree* m_left = nullptr;
    MemoryTree* m_right = nullptr;
};
