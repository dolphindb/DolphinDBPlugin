#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#if __cplusplus >= 201703L
#include <optional>
#endif

#if __cplusplus < 201402L
namespace std {

template <typename T>
struct make_unique_result {
    typedef unique_ptr<T> single_object;
};

template <typename T>
struct make_unique_result<T[]> {
    typedef unique_ptr<T[]> unbounded_array;
};

template <typename T, size_t N>
struct make_unique_result<T[N]> {
    struct invalid_type {};
};

template <typename T, typename... Args>
typename make_unique_result<T>::single_object make_unique(Args&&... args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
typename make_unique_result<T>::unbounded_array make_unique(size_t n) {
    typedef typename remove_extent<T>::type Element;
    return unique_ptr<T>(new Element[n]());
}

template <typename T, typename... Args>
typename make_unique_result<T>::invalid_type make_unique(Args&&...) = delete;

}  // namespace std
#endif

#if __cplusplus < 201703L
namespace std {

struct nullopt_t {
    explicit constexpr nullopt_t(int) {}
};

constexpr nullopt_t nullopt{0};

template <typename T>
class optional {
  public:
    optional() noexcept : hasValue_(false) {}

    optional(nullopt_t) noexcept : hasValue_(false) {}

    optional(const T &value) : hasValue_(false) {
        construct(value);
    }

    optional(T &&value) : hasValue_(false) {
        construct(std::move(value));
    }

    optional(const optional &other) : hasValue_(false) {
        if (other.hasValue_) {
            construct(*other);
        }
    }

    optional(optional &&other) : hasValue_(false) {
        if (other.hasValue_) {
            construct(std::move(*other));
        }
    }

    ~optional() {
        reset();
    }

    optional &operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    optional &operator=(const optional &other) {
        if (this == &other) {
            return *this;
        }
        if (other.hasValue_) {
            assign_or_construct(*other);
        } else {
            reset();
        }
        return *this;
    }

    optional &operator=(optional &&other) {
        if (this == &other) {
            return *this;
        }
        if (other.hasValue_) {
            assign_or_construct(std::move(*other));
        } else {
            reset();
        }
        return *this;
    }

    optional &operator=(const T &value) {
        assign_or_construct(value);
        return *this;
    }

    optional &operator=(T &&value) {
        assign_or_construct(std::move(value));
        return *this;
    }

    explicit operator bool() const noexcept {
        return hasValue_;
    }

    bool has_value() const noexcept {
        return hasValue_;
    }

    T &operator*() {
        return *ptr();
    }

    const T &operator*() const {
        return *ptr();
    }

    T *operator->() {
        return ptr();
    }

    const T *operator->() const {
        return ptr();
    }

    template <typename U>
    T value_or(U &&defaultValue) const {
        return hasValue_ ? **this : static_cast<T>(std::forward<U>(defaultValue));
    }

    void reset() noexcept {
        if (hasValue_) {
            ptr()->~T();
            hasValue_ = false;
        }
    }

  private:
    T *ptr() {
        return reinterpret_cast<T *>(&storage_);
    }

    const T *ptr() const {
        return reinterpret_cast<const T *>(&storage_);
    }

    template <typename U>
    void construct(U &&value) {
        new (&storage_) T(std::forward<U>(value));
        hasValue_ = true;
    }

    template <typename U>
    void assign_or_construct(U &&value) {
        if (hasValue_) {
            **this = std::forward<U>(value);
        } else {
            construct(std::forward<U>(value));
        }
    }

    bool hasValue_;
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type storage_;
};

}  // namespace std
#endif
