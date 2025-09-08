#pragma once

#include <type_traits>
#include <cstddef>

template<typename T>
class Arena {
public:
    using ValueType = std::remove_cvref_t<T>;
    using PointerType = ValueType*;
    using VoidPointer = void*;

    Arena() = default;

    Arena(size_t size)
        : m_capacity{size}
        , m_offset{0}
        , m_data_start{new ValueType[size]}
    {}

    ~Arena() {
        if (m_data_start) {
            delete[] static_cast<PointerType>(m_data_start);
        }
    }

    PointerType allocateN(size_t n) {
        auto const new_offset = m_offset + sizeof(ValueType) * n;

        if (!(new_offset < m_capacity)) {
            TODO("not implemented");
        }

        m_offset = new_offset;

        return static_cast<PointerType>(m_data_start) + m_offset;
    }

    PointerType deallocate(PointerType ptr, size_t n) {
        auto const new_offset = m_offset + sizeof(ValueType) * n;

        if (!(new_offset < m_capacity)) {
            TODO("not implemented");
        }

        m_offset = new_offset;

        return static_cast<PointerType>(m_data_start) + m_offset;
    }

private:
    size_t m_capacity;
    size_t m_offset;
    VoidPointer m_data_start;
};

template<typename T>
class ArenaAllocator {
public:
    static constexpr size_t ARENA_DEFAULT_SIZE = 4 * 1024;

    using pointer = std::remove_cvref_t<T>*;
    using void_pointer = std::nullptr_t;
    using const_void_pointer = std::add_const_t<std::nullptr_t>;
    using value_type = std::remove_cvref_t<T>;
    using size_type = size_t;

    using ArenaType = Arena<T>;

    ArenaAllocator()
        : m_arena{ARENA_DEFAULT_SIZE}
    {}

    ArenaAllocator(ArenaAllocator const& a1) {
        TODO("not implemented");
    }

    ArenaAllocator& operator=(ArenaAllocator const&) {
        TODO("not implemented");
    }

    ArenaAllocator(ArenaAllocator&&) noexcept {
        TODO("not implemented");
    }

    ArenaAllocator& operator=(ArenaAllocator&&) noexcept {
        TODO("not implemented");
    }

    value_type& operator*() {
        TODO("not implemented");
    }

    value_type const& operator*() const {
        TODO("not implemented");
    }

    operator void_pointer() {
        TODO("not implemented");
    }

    operator const_void_pointer() {
        TODO("not implemented");
    }

    pointer allocate(size_type size) {
        return m_arena.allocateN(size);
    }

    pointer deallocate(pointer ptr, size_type size) {
        TODO("not implemented");
    }

    friend bool operator==(ArenaAllocator const& a1, ArenaAllocator const& a2) {
        TODO("not implemented");
    }

    friend bool operator!=(ArenaAllocator const& a1, ArenaAllocator const& a2) {
        TODO("not implemented");
    }

private:
    ArenaType m_arena;
};
