module;

#include "../Macros.hpp"

export module Memory:ArenaAllocator;

import :Arena;
import std;

export template<typename T>
class ArenaAllocator {
public:
    static constexpr std::size_t ARENA_DEFAULT_SIZE = 4 * 1024;

    using pointer = std::remove_cvref_t<T>*;
    using void_pointer = std::nullptr_t;
    using const_void_pointer = std::add_const_t<std::nullptr_t>;
    using value_type = std::remove_cvref_t<T>;
    using size_type = std::size_t;

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
