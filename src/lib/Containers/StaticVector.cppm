export module Containers:StaticVector;

import std;
import Traits;

#define CHECK_CAPACITY_FAIL(error)       \
    if (m_size == m_capacity) {          \
        return std::unexpected((error)); \
    }

#define CHECK_INDEX_FAIL(index, error)   \
    if ((index) > m_size) {              \
        return std::unexpected((error)); \
    }

// TODO: contraints on types
// TODO: std containers compliant
// TODO: std iterators complaint
// TODO: fix const correctness
export template<typename T, std::size_t Capacity>
class StaticVector : public std::ranges::view_interface<StaticVector<T, Capacity>> {
public:
    enum class Error {
        FAILED_TO_PUSH_MAX_SIZE_REACHED,
        FAILED_INDEX_OUT_OF_BOUNDS
    };

    using ValueType = std::remove_cvref_t<T>;

    constexpr StaticVector() = default;
    ~StaticVector() = default;

    constexpr StaticVector(StaticVector const& rhs) {
        m_size = rhs.m_size;
        std::ranges::copy(rhs.m_data, this->m_data);
    }

    constexpr StaticVector operator=(StaticVector const& rhs) {
        m_size = rhs.m_size;
        std::ranges::copy(rhs.m_data, this->m_data);
        return *this;
    }

    constexpr StaticVector(StaticVector&& rhs) noexcept {
        m_size = rhs.m_size;
        std::ranges::copy(rhs.m_data, this->m_data);
    }

    constexpr StaticVector& operator=(StaticVector&& rhs) noexcept {
        m_size = rhs.m_size;
        std::ranges::copy(rhs.m_data, this->m_data);
        return *this;
    }

    template<typename V, typename... Rest>
    constexpr StaticVector(V first, Rest... rest)
        : m_data{first, rest...}
        , m_size{sizeof...(rest) + 1}
    {}

    template<typename... Args>
    constexpr std::expected<void, Error> tryEmplaceBack(Args&&... arguments) noexcept {
        CHECK_CAPACITY_FAIL(Error::FAILED_TO_PUSH_MAX_SIZE_REACHED);
        m_data[m_size++] = { std::forward<Args>(arguments)... };
        return {};
    }

    template<typename... Args>
    constexpr std::expected<void, Error> tryPushBack(ValueType const& value) noexcept {
        CHECK_CAPACITY_FAIL(Error::FAILED_TO_PUSH_MAX_SIZE_REACHED);
        m_data[m_size++] = value;
        return {};
    }

    constexpr std::optional<ValueType> popBack() noexcept {
        if (m_size > 0) {
            auto const tmp = m_data[m_size--];
            // TODO: fill with zeroes?
            return tmp;
        }

        return std::nullopt;
    }

    constexpr ValueType& operator[](std::size_t index) {
        return m_data[index];
    }

    constexpr ValueType operator[](std::size_t index) const {
        std::println("begin");
        return m_data[index];
    }

    constexpr std::expected<ValueType&, Error> at(std::size_t index) noexcept {
        return m_data[index];
    }

    constexpr std::expected<ValueType, Error> at(std::size_t index) const noexcept {
        CHECK_INDEX_FAIL(index, Error::FAILED_INDEX_OUT_OF_BOUNDS);
        return m_data[index];
    }

    constexpr std::size_t size() const noexcept {
        return m_size;
    }

    constexpr std::size_t capacity() const noexcept {
        return m_capacity;
    }

    struct Iterator {
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;

        value_type* data;
        difference_type index;

        value_type& operator*() const {
            return data[index];
        }

        Iterator& operator++() noexcept {
            index += 1;
            return *this;
        }

        Iterator operator++(int) noexcept {
            auto const tmp = *this;
            index += 1;
            return tmp;
        }

        Iterator& operator--() noexcept {
            index -= 1;
            return *this;
        }

        Iterator operator--(int) noexcept {
            auto const tmp = *this;
            index -= 1;
            return tmp;
        }

        friend bool operator==(Iterator const& lhs, Iterator const& rhs) {
            bool const same_data = lhs.data == rhs.data;
            bool const same_index = lhs.index == rhs.index;
            return same_data && same_index;
        }

        friend bool operator!=(Iterator const& lhs, Iterator const& rhs) {
            return !(lhs == rhs);
        }

        friend bool operator<(Iterator const& lhs, Iterator const& rhs) {
            bool const same_data = lhs.data == rhs.data;
            bool const index_lt = lhs.index < rhs.index;
            return same_data && index_lt;
        }

        friend bool operator>(Iterator const& lhs, Iterator const& rhs) {
            bool const same_data = lhs.data == rhs.data;
            bool const index_gt = lhs.index > rhs.index;
            return same_data && index_gt;
        }

        friend bool operator<=(Iterator const& lhs, Iterator const& rhs) {
            bool const same_data = lhs.data == rhs.data;
            bool const index_lte = lhs.index <= rhs.index;
            return same_data && index_lte;
        }

        friend bool operator>=(Iterator const& lhs, Iterator const& rhs) {
            bool const same_data = lhs.data == rhs.data;
            bool const index_gte = lhs.index >= rhs.index;
            return same_data && index_gte;
        }

        friend difference_type operator+(Iterator const& lhs, Iterator const& rhs) {
            return lhs.index - rhs.index;
        }

        friend difference_type operator-(Iterator const& lhs, Iterator const& rhs) {
            return lhs.index - rhs.index;
        }

        friend Iterator operator+(Iterator const& lhs, difference_type const n) {
            return lhs.index += n;
            return lhs;
        }

        friend Iterator operator-(Iterator const& lhs, difference_type const n) {
            return lhs.index -= n;
            return lhs;
        }

        friend Iterator operator+(difference_type const n, Iterator const& lhs) {
            return lhs + n;
        }

        friend Iterator operator-(difference_type const n, Iterator const& lhs) {
            return lhs - n;
        }

        friend Iterator& operator+=(Iterator& lhs, difference_type const n) {
            lhs.index += n;
            return lhs;
        }

        friend Iterator& operator-=(Iterator& lhs, difference_type const n) {
            lhs.index -= n;
            return lhs;
        }

        value_type& operator[](difference_type const index) const {
            return data[index];
        }
    };

    static_assert(std::random_access_iterator<Iterator>, "StaticVector::Iterator does not implement concept 'random_access_iterator'");

    Iterator begin() {
        return Iterator{m_data, typename Iterator::difference_type(0)};
    }

    Iterator end() {
        return Iterator{m_data, typename Iterator::difference_type(m_size)};
    }

    template<typename F>
    friend constexpr decltype(auto) operator|(StaticVector&& vector, F&& function) {
        return std::forward<F>(function)(Iterator{vector.m_data, typename Iterator::difference_type(0)});
    }

private:
    ValueType m_data[Capacity];
    std::size_t m_size = 0;
    std::size_t const m_capacity = Capacity;
};

// Deduction Guides

template<typename T, typename... U>
StaticVector(T, U...) -> StaticVector<T, sizeof...(U) + 1>;
