module;

#include "../Macros.hpp"

export module Memory:MemoryTreeImpl;

import :MemoryTree;
import std;

void MemoryTree::insert(void* data, std::size_t size) {
    if (!data) {
        // TODO: handle errors
        return;
    }

    // The case when we're empty
    if (!m_data) {
        m_data = data;
        m_size = size;
    }

    TODO("not implemented");
}
