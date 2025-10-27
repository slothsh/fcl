module;

#include "../Macros.hpp"

export module Memory:Arena;

import std;

export template<typename T>
class Arena {
public:
    using ValueType = std::remove_cvref_t<T>;
    using PointerType = ValueType*;
    using VoidPointer = void*;

    Arena() = default;

    Arena(std::size_t size)
        : m_capacity{size}
        , m_offset{0}
        , m_data_start{new ValueType[size]}
    {}

    ~Arena() {
        if (m_data_start) {
            delete[] static_cast<PointerType>(m_data_start);
        }
    }

    PointerType allocateN(std::size_t n) {
        auto const new_offset = m_offset + sizeof(ValueType) * n;

        if (!(new_offset < m_capacity)) {
            TODO("not implemented");
        }

        m_offset = new_offset;

        return static_cast<PointerType>(m_data_start) + m_offset;
    }

    PointerType deallocate(PointerType ptr, std::size_t n) {
        auto const new_offset = m_offset + sizeof(ValueType) * n;

        if (!(new_offset < m_capacity)) {
            TODO("not implemented");
        }

        m_offset = new_offset;

        return static_cast<PointerType>(m_data_start) + m_offset;
    }

private:
    std::size_t m_capacity;
    std::size_t m_offset;
    VoidPointer m_data_start;
};
