#ifndef FLAGHASHMAP_H_
#define FLAGHASHMAP_H_
#include <exception>
#include <memory>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstdio>
#include <cassert>
#include <memory.h>
#include <functional>
#include "HashmapUtil.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE  64 // 64 byte cache line on x86 and x86-64
#endif

#define DEFAULT_HM_CAPACITY 2

struct prime_hash_policy {
    size_t index(uint64_t hash) const {
        return hash % cap_;
    }
    size_t capacity() const {
        return cap_;
    }
    prime_hash_policy(size_t cap, bool predicted = false){
        if (predicted == false) {
            cap_ = nextPrime(cap * 2);
        } else {
            cap_ = nextPrime(cap);
        }
    }
private:
    size_t nextPrime(size_t n) {
        size_t x = n;
        while (true) {
            x++;
            size_t i = 2;
            bool flag = true;
            while (i * i <= x && flag) {
                if (x % i == 0) {
                    flag = false;
                }
                ++i;
            }
            if (flag)
                return x;
        }
    }
    size_t cap_;
};

struct power2_hash_policy {
    size_t cap_; // # of slots
    static size_t next_power2(size_t n) {
        if (n == 0)
            return 1;
        size_t x = 1;
        while (x <= n) {
            x *= 2;
        }
        return x;
    }
    power2_hash_policy(size_t requiredCap, bool predicted = false): cap_(1){
        cap_ = next_power2(requiredCap);
        assert(cap_ == 0 || (cap_ & (cap_ - 1)) == 0);
    }
    size_t index(uint64_t hash) const {
        return hash & (cap_ - 1);
    }
    size_t capacity() const {
        return cap_;
    }
};

#ifdef __has_builtin
#define HAVE_BUILTIN(x) __has_builtin(x)
#else
#define HAVE_BUILTIN(x) 0
#endif

#ifdef __has_attribute
#define HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAVE_ATTRIBUTE(x) 0
#endif

#if HAVE_BUILTIN(__builtin_expect) || (defined(__GNUC__) && !defined(__clang__))
#define PREDICT_FALSE(x) (__builtin_expect(false || (x), false))
#define PREDICT_TRUE(x) (__builtin_expect(false || (x), true))
#else
#define PREDICT_FALSE(x) (x)
#define PREDICT_TRUE(x) (x)
#endif

#if defined(__GNUC__) && !defined(__clang__)
// GCC
#define NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(x) 1
#else
#define NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(x) HAVE_BUILTIN(x)
#endif

#if !defined(NDEBUG)
#define INTERNAL_ASSUME(cond) assert(cond)
#elif HAVE_BUILTIN(__builtin_assume)
#define INTERNAL_ASSUME(cond) __builtin_assume(cond)
#elif defined(__GNUC__) || HAVE_BUILTIN(__builtin_unreachable)
#define INTERNAL_ASSUME(cond)        \
  do {                                    \
    if (!(cond)) __builtin_unreachable(); \
  } while (0)
#elif defined(_MSC_VER)
#define INTERNAL_ASSUME(cond) __assume(cond)
#else
#define INTERNAL_ASSUME(cond)      \
  do {                                  \
    static_cast<void>(false && (cond)); \
  } while (0)
#endif

// Forces functions to either inline or not inline. Introduced in gcc 3.1.
#if HAVE_ATTRIBUTE(always_inline) || \
    (defined(__GNUC__) && !defined(__clang__))
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#define HAVE_ATTRIBUTE_ALWAYS_INLINE 1
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountLeadingZeroes32(uint32_t x) {
#if NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(__builtin_clz)
  // Use __builtin_clz, which uses the following instructions:
  //  x86: bsr, lzcnt
  //  ARM64: clz
  //  PPC: cntlzd

  static_assert(sizeof(unsigned int) == sizeof(x),
                "__builtin_clz does not take 32-bit arg");
  // Handle 0 as a special case because __builtin_clz(0) is undefined.
  return x == 0 ? 32 : __builtin_clz(x);
#elif defined(_MSC_VER) && !defined(__clang__)
  unsigned long result = 0;  // NOLINT(runtime/int)
  if (_BitScanReverse(&result, x)) {
    return 31 - result;
  }
  return 32;
#else
  int zeroes = 28;
  if (x >> 16) {
    zeroes -= 16;
    x >>= 16;
  }
  if (x >> 8) {
    zeroes -= 8;
    x >>= 8;
  }
  if (x >> 4) {
    zeroes -= 4;
    x >>= 4;
  }
  return "\4\3\2\2\1\1\1\1\0\0\0\0\0\0\0"[x] + zeroes;
#endif
}

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountLeadingZeroes16(uint16_t x) {
#if HAVE_BUILTIN(__builtin_clzs)
  static_assert(sizeof(unsigned short) == sizeof(x),  // NOLINT(runtime/int)
                "__builtin_clzs does not take 16-bit arg");
  return x == 0 ? 16 : __builtin_clzs(x);
#else
  return CountLeadingZeroes32(x) - 16;
#endif
}

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountLeadingZeroes64(uint64_t x) {
#if NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(__builtin_clzll)
  // Use __builtin_clzll, which uses the following instructions:
  //  x86: bsr, lzcnt
  //  ARM64: clz
  //  PPC: cntlzd
  static_assert(sizeof(unsigned long long) == sizeof(x),  // NOLINT(runtime/int)
                "__builtin_clzll does not take 64-bit arg");

  // Handle 0 as a special case because __builtin_clzll(0) is undefined.
  return x == 0 ? 64 : __builtin_clzll(x);
#elif defined(_MSC_VER) && !defined(__clang__) && \
    (defined(_M_X64) || defined(_M_ARM64))
  // MSVC does not have __buitin_clzll. Use _BitScanReverse64.
  unsigned long result = 0;  // NOLINT(runtime/int)
  if (_BitScanReverse64(&result, x)) {
    return 63 - result;
  }
  return 64;
#elif defined(_MSC_VER) && !defined(__clang__)
  // MSVC does not have __buitin_clzll. Compose two calls to _BitScanReverse
  unsigned long result = 0;  // NOLINT(runtime/int)
  if ((x >> 32) &&
      _BitScanReverse(&result, static_cast<unsigned long>(x >> 32))) {
    return 31 - result;
  }
  if (_BitScanReverse(&result, static_cast<unsigned long>(x))) {
    return 63 - result;
  }
  return 64;
#else
  int zeroes = 60;
  if (x >> 32) {
    zeroes -= 32;
    x >>= 32;
  }
  if (x >> 16) {
    zeroes -= 16;
    x >>= 16;
  }
  if (x >> 8) {
    zeroes -= 8;
    x >>= 8;
  }
  if (x >> 4) {
    zeroes -= 4;
    x >>= 4;
  }
  return "\4\3\2\2\1\1\1\1\0\0\0\0\0\0\0"[x] + zeroes;
#endif
}

constexpr bool IsPowerOf2(unsigned int x) noexcept {
  return x != 0 && (x & (x - 1)) == 0;
}

template <typename T>
ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountLeadingZeroes(T x) {
  static_assert(std::is_unsigned<T>::value, "T must be unsigned");
  static_assert(IsPowerOf2(std::numeric_limits<T>::digits),
                "T must have a power-of-2 size");
  static_assert(sizeof(T) <= sizeof(uint64_t), "T too large");
  return sizeof(T) <= sizeof(uint16_t)
             ? CountLeadingZeroes16(static_cast<uint16_t>(x)) -
                   (std::numeric_limits<uint16_t>::digits -
                    std::numeric_limits<T>::digits)
             : (sizeof(T) <= sizeof(uint32_t)
                    ? CountLeadingZeroes32(static_cast<uint32_t>(x)) -
                          (std::numeric_limits<uint32_t>::digits -
                           std::numeric_limits<T>::digits)
                    : CountLeadingZeroes64(x));
}

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountTrailingZeroesNonzero32(uint32_t x) {
#if NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(__builtin_ctz)
  static_assert(sizeof(unsigned int) == sizeof(x),
                "__builtin_ctz does not take 32-bit arg");
  return __builtin_ctz(x);
#elif defined(_MSC_VER) && !defined(__clang__)
  unsigned long result = 0;  // NOLINT(runtime/int)
  _BitScanForward(&result, x);
  return result;
#else
  int c = 31;
  x &= ~x + 1;
  if (x & 0x0000FFFF) c -= 16;
  if (x & 0x00FF00FF) c -= 8;
  if (x & 0x0F0F0F0F) c -= 4;
  if (x & 0x33333333) c -= 2;
  if (x & 0x55555555) c -= 1;
  return c;
#endif
}

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountTrailingZeroesNonzero64(uint64_t x) {
#if NUMERIC_INTERNAL_HAVE_BUILTIN_OR_GCC(__builtin_ctzll)
  static_assert(sizeof(unsigned long long) == sizeof(x),  // NOLINT(runtime/int)
                "__builtin_ctzll does not take 64-bit arg");
  return __builtin_ctzll(x);
#elif defined(_MSC_VER) && !defined(__clang__) && \
    (defined(_M_X64) || defined(_M_ARM64))
  unsigned long result = 0;  // NOLINT(runtime/int)
  _BitScanForward64(&result, x);
  return result;
#elif defined(_MSC_VER) && !defined(__clang__)
  unsigned long result = 0;  // NOLINT(runtime/int)
  if (static_cast<uint32_t>(x) == 0) {
    _BitScanForward(&result, static_cast<unsigned long>(x >> 32));
    return result + 32;
  }
  _BitScanForward(&result, static_cast<unsigned long>(x));
  return result;
#else
  int c = 63;
  x &= ~x + 1;
  if (x & 0x00000000FFFFFFFF) c -= 32;
  if (x & 0x0000FFFF0000FFFF) c -= 16;
  if (x & 0x00FF00FF00FF00FF) c -= 8;
  if (x & 0x0F0F0F0F0F0F0F0F) c -= 4;
  if (x & 0x3333333333333333) c -= 2;
  if (x & 0x5555555555555555) c -= 1;
  return c;
#endif
}

ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountTrailingZeroesNonzero16(uint16_t x) {
#if HAVE_BUILTIN(__builtin_ctzs)
  static_assert(sizeof(unsigned short) == sizeof(x),  // NOLINT(runtime/int)
                "__builtin_ctzs does not take 16-bit arg");
  return __builtin_ctzs(x);
#else
  return CountTrailingZeroesNonzero32(x);
#endif
}

template <class T>
ATTRIBUTE_ALWAYS_INLINE constexpr inline int
CountTrailingZeroes(T x) noexcept {
  static_assert(std::is_unsigned<T>::value, "T must be unsigned");
  static_assert(IsPowerOf2(std::numeric_limits<T>::digits),
                "T must have a power-of-2 size");
  static_assert(sizeof(T) <= sizeof(uint64_t), "T too large");
  return x == 0 ? std::numeric_limits<T>::digits
                : (sizeof(T) <= sizeof(uint16_t)
                       ? CountTrailingZeroesNonzero16(static_cast<uint16_t>(x))
                       : (sizeof(T) <= sizeof(uint32_t)
                              ? CountTrailingZeroesNonzero32(
                                    static_cast<uint32_t>(x))
                              : CountTrailingZeroesNonzero64(x)));
}

template <size_t Width>
class probe_seq {
   public:
    probe_seq(size_t hash, size_t mask) {
        assert(((mask + 1) & mask) == 0 && "not a mask");
        mask_ = mask;
        offset_ = hash & mask_;
    }
    size_t offset() const { return offset_; }
    size_t offset(size_t i) const { return (offset_ + i) & mask_; }

    void next() {
        index_ += Width;
        offset_ += index_;
        offset_ &= mask_;
    }
    // 0-based probe index. The i-th probe in the probe sequence.
    size_t index() const { return index_; }

   public:
    size_t mask_;
    size_t offset_;
    size_t index_ = 0;
};

template <typename T>
uint32_t TrailingZeros(T x) {
  INTERNAL_ASSUME(x != 0);
  return CountTrailingZeroes(x);
}

// Returns: If x == 0, 0; otherwise one plus the base-2 logarithm of x, with any
// fractional part discarded.
template <class T>
constexpr inline
    typename std::enable_if<std::is_unsigned<T>::value, T>::type
    bit_width(T x) noexcept {
  return std::numeric_limits<T>::digits - CountLeadingZeroes(x);
}

// Counting functions
//
// While these functions are typically constexpr, on some platforms, they may
// not be marked as constexpr due to constraints of the compiler/available
// intrinsics.
template <class T>
constexpr inline
    typename std::enable_if<std::is_unsigned<T>::value, int>::type
    countl_zero(T x) noexcept {
  return CountLeadingZeroes(x);
}
// An abstraction over a bitmask. It provides an easy way to iterate through the
// indexes of the set bits of a bitmask.  When Shift=0 (platforms with SSE),
// this is a true bitmask.  On non-SSE, platforms the arithematic used to
// emulate the SSE behavior works in bytes (Shift=3) and leaves each bytes as
// either 0x00 or 0x80.
//
// For example:
//   for (int i : BitMask<uint32_t, 16>(0x5)) -> yields 0, 2
//   for (int i : BitMask<uint64_t, 8, 3>(0x0000000080800000)) -> yields 2, 3
template <class T, int SignificantBits, int Shift = 0>
class BitMask {
  static_assert(std::is_unsigned<T>::value, "");
  static_assert(Shift == 0 || Shift == 3, "");

 public:
  // These are useful for unit tests (gunit).
  using value_type = int;
  using iterator = BitMask;
  using const_iterator = BitMask;

  explicit BitMask(T mask) : mask_(mask) {}
  BitMask& operator++() {
    mask_ &= (mask_ - 1);
    return *this;
  }
  explicit operator bool() const { return mask_ != 0; }
  int operator*() const { return LowestBitSet(); }
  uint32_t LowestBitSet() const {
    return CountTrailingZeroes(mask_) >> Shift;
  }
  


  uint32_t HighestBitSet() const {
    return static_cast<uint32_t>((bit_width(mask_) - 1) >> Shift);
  }

  BitMask begin() const { return *this; }
  BitMask end() const { return BitMask(0); }

  uint32_t TrailingZeros() const {
    return ::TrailingZeros(mask_) >> Shift;
  }

  uint32_t LeadingZeros() const {
    constexpr int total_significant_bits = SignificantBits << Shift;
    constexpr int extra_bits = sizeof(T) * 8 - total_significant_bits;
    return countl_zero(mask_ << extra_bits) >> Shift;
  }

 private:
  friend bool operator==(const BitMask& a, const BitMask& b) {
    return a.mask_ == b.mask_;
  }
  friend bool operator!=(const BitMask& a, const BitMask& b) {
    return a.mask_ != b.mask_;
  }

  T mask_;
};

using ctrl_t = signed char;
using h2_t = uint8_t;

enum Ctrl : ctrl_t {
    kEmpty = -128,   // 0b10000000
    kDeleted = -2,   // 0b11111110
    kSentinel = -1,  // 0b11111111
                     // kFull >= 0     // 0b0xxxxxxx
};

// A single block of empty control bytes for tables without any slots allocated.
// This enables removing a branch in the hot path of find().
inline ctrl_t* EmptyGroup() {
  alignas(16) static constexpr ctrl_t empty_group[] = {
      kSentinel, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,
      kEmpty,    kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty};
  return const_cast<ctrl_t*>(empty_group);
}

// Returns a hash seed.
//
// The seed consists of the ctrl_ pointer, which adds enough entropy to ensure
// non-determinism of iteration order in most cases.
inline size_t HashSeed(const ctrl_t* ctrl) {
  // The low bits of the pointer have little or no entropy because of
  // alignment. We shift the pointer to try to use higher entropy bits. A
  // good number seems to be 12 bits, because that aligns with page size.
  return reinterpret_cast<uintptr_t>(ctrl) >> 12;
}

inline size_t H1(size_t hash, const ctrl_t* ctrl) {
  return (hash >> 7) ^ HashSeed(ctrl);
}
inline ctrl_t H2(size_t hash) { return hash & 0x7F; }

inline bool IsEmpty(ctrl_t c) { return c == kEmpty; }
inline bool IsFull(ctrl_t c) { return c >= 0; }
inline bool IsDeleted(ctrl_t c) { return c == kDeleted; }
inline bool IsEmptyOrDeleted(ctrl_t c) { return c < kSentinel; }

// Returns "random" seed.
inline size_t RandomSeed() {
#ifdef HAVE_THREAD_LOCAL
  static thread_local size_t counter = 0;
  size_t value = ++counter;
#else   // HAVE_THREAD_LOCAL
  static std::atomic<size_t> counter(0);
  size_t value = counter.fetch_add(1, std::memory_order_relaxed);
#endif  // HAVE_THREAD_LOCAL
  return value ^ static_cast<size_t>(reinterpret_cast<uintptr_t>(&counter));
}

#if !defined(NDEBUG)
static bool ShouldInsertBackwards(size_t hash, ctrl_t* ctrl) {
  // To avoid problems with weak hashes and single bit tests, we use % 13.
  // TODO(kfm,sbenza): revisit after we do unconditional mixing
  return (H1(hash, ctrl) ^ RandomSeed()) % 13 > 6;
}
#endif


#if HAVE_SSE2
#include <wmmintrin.h>

// https://github.com/abseil/abseil-cpp/issues/209
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87853
// _mm_cmpgt_epi8 is broken under GCC with -funsigned-char
// Work around this by using the portable implementation of Group
// when using -funsigned-char under GCC.
inline __m128i _mm_cmpgt_epi8_fixed(__m128i a, __m128i b) {
#if defined(__GNUC__) && !defined(__clang__)
    if (std::is_unsigned<char>::value) {
        const __m128i mask = _mm_set1_epi8(0x80);
        const __m128i diff = _mm_subs_epi8(b, a);
        return _mm_cmpeq_epi8(_mm_and_si128(diff, mask), mask);
    }
#endif
    return _mm_cmpgt_epi8(a, b);
}

struct GroupSse2Impl {
    static constexpr size_t kWidth = 16;  // the number of slots per group

    explicit GroupSse2Impl(const ctrl_t* pos) { ctrl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pos)); }

    // Returns a bitmask representing the positions of slots that match hash.
    BitMask<uint32_t, kWidth> Match(h2_t hash) const {
        auto match = _mm_set1_epi8(hash);
        return BitMask<uint32_t, kWidth>(_mm_movemask_epi8(_mm_cmpeq_epi8(match, ctrl)));
    }

    // Returns a bitmask representing the positions of empty slots.
    BitMask<uint32_t, kWidth> MatchEmpty() const {
#if HAVE_SSSE3
        // This only works because kEmpty is -128.
        return BitMask<uint32_t, kWidth>(_mm_movemask_epi8(_mm_sign_epi8(ctrl, ctrl)));
#else
        return Match(static_cast<h2_t>(kEmpty));
#endif
    }

    // Returns a bitmask representing the positions of empty or deleted slots.
    BitMask<uint32_t, kWidth> MatchEmptyOrDeleted() const {
        auto special = _mm_set1_epi8(kSentinel);
        return BitMask<uint32_t, kWidth>(_mm_movemask_epi8(_mm_cmpgt_epi8_fixed(special, ctrl)));
    }

    // Returns the number of trailing empty or deleted elements in the group.
    uint32_t CountLeadingEmptyOrDeleted() const {
        auto special = _mm_set1_epi8(kSentinel);
        return TrailingZeros(static_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpgt_epi8_fixed(special, ctrl)) + 1));
    }

    void ConvertSpecialToEmptyAndFullToDeleted(ctrl_t* dst) const {
        auto msbs = _mm_set1_epi8(static_cast<char>(-128));
        auto x126 = _mm_set1_epi8(126);
#if HAVE_SSSE3
        auto res = _mm_or_si128(_mm_shuffle_epi8(x126, ctrl), msbs);
#else
        auto zero = _mm_setzero_si128();
        auto special_mask = _mm_cmpgt_epi8_fixed(zero, ctrl);
        auto res = _mm_or_si128(msbs, _mm_andnot_si128(special_mask, x126));
#endif
        _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), res);
    }

    __m128i ctrl;
};
#endif  // HAVE_SSE2

inline uint64_t ToHost64(uint64_t x) { return x; }

inline uint64_t UnalignedLoad64(const void *p) {
  uint64_t t;
  memcpy(&t, p, sizeof t);
  return t;
}

struct GroupPortableImpl {
    static constexpr size_t kWidth = 8;

    explicit GroupPortableImpl(const ctrl_t* pos) : ctrl(ToHost64(UnalignedLoad64(pos))){}

    BitMask<uint64_t, kWidth, 3> Match(h2_t hash) const {
        // For the technique, see:
        // http://graphics.stanford.edu/~seander/bithacks.html##ValueInWord
        // (Determine if a word has a byte equal to n).
        //
        // Caveat: there are false positives but:
        // - they only occur if there is a real match
        // - they never occur on kEmpty, kDeleted, kSentinel
        // - they will be handled gracefully by subsequent checks in code
        //
        // Example:
        //   v = 0x1716151413121110
        //   hash = 0x12
        //   retval = (v - lsbs) & ~v & msbs = 0x0000000080800000
        constexpr uint64_t msbs = 0x8080808080808080ULL;
        constexpr uint64_t lsbs = 0x0101010101010101ULL;
        auto x = ctrl ^ (lsbs * hash);
        return BitMask<uint64_t, kWidth, 3>((x - lsbs) & ~x & msbs);
    }

    BitMask<uint64_t, kWidth, 3> MatchEmpty() const {
        constexpr uint64_t msbs = 0x8080808080808080ULL;
        return BitMask<uint64_t, kWidth, 3>((ctrl & (~ctrl << 6)) & msbs);
    }

    BitMask<uint64_t, kWidth, 3> MatchEmptyOrDeleted() const {
        constexpr uint64_t msbs = 0x8080808080808080ULL;
        return BitMask<uint64_t, kWidth, 3>((ctrl & (~ctrl << 7)) & msbs);
    }

    uint32_t CountLeadingEmptyOrDeleted() const {
        constexpr uint64_t gaps = 0x00FEFEFEFEFEFEFEULL;
        return (TrailingZeros(((~ctrl & (ctrl >> 7)) | gaps) + 1) + 7) >> 3;
    }

    void ConvertSpecialToEmptyAndFullToDeleted(ctrl_t* dst) const {
      constexpr uint64_t msbs = 0x8080808080808080ULL;
      constexpr uint64_t lsbs = 0x0101010101010101ULL;
      auto x = ctrl & msbs;
      auto res = (~x + (x >> 7)) & ~lsbs;
      memcpy(dst, &res, sizeof(res));
    }

    uint64_t ctrl;
};

#if HAVE_SSE2
using Group = GroupSse2Impl;
#else
using Group = GroupPortableImpl;
#endif

// The number of cloned control bytes that we copy from the beginning to the
// end of the control bytes array.
constexpr size_t NumClonedBytes() { return Group::kWidth - 1; }

template <class Policy, class Hash, class Eq, class Alloc>
class raw_hash_set;

inline bool IsValidCapacity(size_t n) { return ((n + 1) & n) == 0 && n > 0; }

// PRECONDITION:
//   IsValidCapacity(capacity)
//   ctrl[capacity] == kSentinel
//   ctrl[i] != kSentinel for all i < capacity
// Applies mapping for every byte in ctrl:
//   DELETED -> EMPTY
//   EMPTY -> EMPTY
//   FULL -> DELETED
static void ConvertDeletedToEmptyAndFullToDeleted(ctrl_t* ctrl, size_t capacity){
  assert(ctrl[capacity] == kSentinel);
  assert(IsValidCapacity(capacity));
  for (ctrl_t* pos = ctrl; pos < ctrl + capacity; pos += Group::kWidth) {
    Group{pos}.ConvertSpecialToEmptyAndFullToDeleted(pos);
  }
  // Copy the cloned ctrl bytes.
  std::memcpy(ctrl + capacity + 1, ctrl, NumClonedBytes());
  ctrl[capacity] = kSentinel;
}

// Rounds up the capacity to the next power of 2 minus 1, with a minimum of 1.
inline size_t NormalizeCapacity(size_t n) { return n ? ~size_t{} >> CountLeadingZeroes64(n) : 1; }

// General notes on capacity/growth methods below:
// - We use 7/8th as maximum load factor. For 16-wide groups, that gives an
//   average of two empty slots per group.
// - For (capacity+1) >= Group::kWidth, growth is 7/8*capacity.
// - For (capacity+1) < Group::kWidth, growth == capacity. In this case, we
//   never need to probe (the whole table fits in one group) so we don't need a
//   load factor less than 1.

// Given `capacity` of the table, returns the size (i.e. number of full slots)
// at which we should grow the capacity.
inline size_t CapacityToGrowth(size_t capacity) {
    assert(IsValidCapacity(capacity));
    // `capacity*7/8`
    if (Group::kWidth == 8 && capacity == 7) {
        // x-x/8 does not work when x==7.
        return 6;
    }
    return capacity - capacity / 8;
}
// From desired "growth" to a lowerbound of the necessary capacity.
// Might not be a valid one and requires NormalizeCapacity().
inline size_t GrowthToLowerboundCapacity(size_t growth) {
    // `growth*8/7`
    if (Group::kWidth == 8 && growth == 7) {
        // x+(x-1)/7 does not work when x==7.
        return 8;
    }
    return growth + static_cast<size_t>((static_cast<int64_t>(growth) - 1) / 7);
}

inline void AssertIsFull(ctrl_t* ctrl) {
    assert(ctrl != nullptr && IsFull(*ctrl));
}

inline void AssertIsValid(ctrl_t* ctrl) {
    assert(ctrl == nullptr || IsFull(*ctrl));
}

struct FindInfo {
    size_t offset;
    size_t probe_length;
};

// The representation of the object has two modes:
//  - small: For capacities < kWidth-1
//  - large: For the rest.
//
// Differences:
//  - In small mode we are able to use the whole capacity. The extra control
//  bytes give us at least one "empty" control byte to stop the iteration.
//  This is important to make 1 a valid capacity.
//
//  - In small mode only the first `capacity()` control bytes after the
//  sentinel are valid. The rest contain dummy kEmpty values that do not
//  represent a real slot. This is important to take into account on
//  find_first_non_full(), where we never try ShouldInsertBackwards() for
//  small tables.
inline bool is_small(size_t capacity) { return capacity < Group::kWidth - 1;}

inline probe_seq<Group::kWidth> probe(ctrl_t* ctrl, size_t hash, size_t capacity) {
    return probe_seq<Group::kWidth>(H1(hash, ctrl), capacity);
}

// Probes the raw_hash_set with the probe sequence for hash and returns the
// pointer to the first empty or deleted slot.
// NOTE: this function must work with tables having both kEmpty and kDelete
// in one group. Such tables appears during drop_deletes_without_resize.
//
// This function is very useful when insertions happen and:
// - the input is already a set
// - there are enough slots
// - the element with the hash is not in the table

inline FindInfo find_first_non_full(ctrl_t* ctrl, size_t hash, size_t capacity) {
    auto seq = probe(ctrl, hash, capacity);
    while (true) {
        Group g{ctrl + seq.offset()};
        auto mask = g.MatchEmptyOrDeleted();
        if (mask) {
#if !defined(NDEBUG)
            // We want to add entropy even when ASLR is not enabled.
            // In debug build we will randomly insert in either the front or back of
            // the group.
            // TODO(kfm,sbenza): revisit after we do unconditional mixing
            if (!is_small(capacity) && ShouldInsertBackwards(hash, ctrl)) {
                return {seq.offset(mask.HighestBitSet()), seq.index()};
            }
#endif
            return {seq.offset(mask.LowestBitSet()), seq.index()};
        }
        seq.next();
        assert(seq.index() <= capacity && "full table!");
    }
}

#define INSERT_GOOD        0
#define INSERT_NO_VACANCY  1
#define INSERT_KEY_EXISTS  2

template <class Key, class T, class Hasher, class KeyEqual>
class SwissTableImpl {
public:
    using key_type = Key;
    using mapped_type = T;
    using key_hasher = Hasher;
    using key_equal = KeyEqual;

    SwissTableImpl():ctrl_(EmptyGroup()){
      initialize();
      endIterator = iterator(this, capacity_);
    }

    ~SwissTableImpl() {
        if(capacity_ > 0){
            for (size_t i = 0; i < capacity_; ++i) {
                if (!IsFull(ctrl_[i])){
                    continue;
                }
                set_ctrl(i, kEmpty);
                keys_[i].~key_type();
                values_[i].~mapped_type();
            }
            size_ = 0;

            myFree(ctrl_unaligned_);
            myFree(keys_unaligned_);
            myFree(values_unaligned_);
        }
    }
    // Insert <key, value>
    // This version is used by hashmap that supports erase.
    // Return Values:
    //  INSERT_NO_VACANCY : the insertion procedure fails to find a empty slot within probe_limit_ away from the initial
    //  hashing position. This suggests a rehash with larger capacity is needed to spread out the keys. 
    //  INSERT_GOOD: the insertion is successful.
    //  INSERT_KEY_EXISTS : the insertion proceudre fails because the key already exists
    int insert(const key_type& key, const mapped_type& value) {
        uint64_t hash = key_hasher_(key);
        auto seq = probe(ctrl_, hash, capacity_);
        // check if key already exists
        while (true) {
            Group g{ctrl_ + seq.offset()};
            for (int i : g.Match(H2(hash))) {
                if (PREDICT_TRUE(key_equal_(key, keys_[seq.offset(i)]))) {
                    return INSERT_KEY_EXISTS;
                }
            }
            if (PREDICT_TRUE(g.MatchEmpty())) 
                break;
            seq.next();
            assert(seq.index() <= capacity_ && "full table!");
        }
        // prepare insert
        auto target = find_first_non_full(ctrl_, hash, capacity_);
        if (PREDICT_FALSE(grow_left_ == 0 && !IsDeleted(ctrl_[target.offset]))) {
            rehash_and_grow_if_necessary();
            target = find_first_non_full(ctrl_, hash, capacity_);
            assert(target.offset < capacity_);
        }
        size_t new_i = target.offset;
        // insert
        new ((char*)&keys_[new_i]) key_type(key);
        try {
            new ((char*)&values_[new_i]) mapped_type(value);
        } catch (...) {
            keys_[new_i].~key_type();
            throw;
        }
        ++size_;
        grow_left_ -= IsEmpty(ctrl_[new_i]);
        set_ctrl(new_i, H2(hash));
        return INSERT_GOOD;
    }

    // Insert <key, value>
    // Similar to the "int insert(const key_type & key, const mapped_type & value)",
    // but additionally returns a pointer to :
    //  1. the address that stores the value if the insertion is successful
    //  2. the addres that stores the existing value if the key already exists in the table
    int insert(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        uint64_t hash = key_hasher_(key);
        auto seq = probe(ctrl_, hash, capacity_);
        // check if key already exists
        while (true) {
            Group g{ctrl_ + seq.offset()};
            for (int i : g.Match(H2(hash))) {
                if (PREDICT_TRUE(key_equal_(key, keys_[seq.offset(i)]))) {
                    *recvPtr = &values_[seq.offset(i)];
                    return INSERT_KEY_EXISTS;
                }
            }
            if (PREDICT_TRUE(g.MatchEmpty())) 
                break;
            seq.next();
            assert(seq.index() <= capacity_ && "full table!");
        }
        // prepare insert
        auto target = find_first_non_full(ctrl_, hash, capacity_);
        if (PREDICT_FALSE(grow_left_ == 0 && !IsDeleted(ctrl_[target.offset]))) {
            rehash_and_grow_if_necessary();
            target = find_first_non_full(ctrl_, hash, capacity_);
            assert(target.offset < capacity_);
        }
        size_t new_i = target.offset;
        // insert
        new ((char*)&keys_[new_i]) key_type(key);
        try {
            new ((char*)&values_[new_i]) mapped_type(value);
        } catch (...) {
            keys_[new_i].~key_type();
            throw;
        }
        ++size_;
        grow_left_ -= IsEmpty(ctrl_[new_i]);
        set_ctrl(new_i, H2(hash));
        *recvPtr = &values_[new_i];
        return INSERT_GOOD;
    }

    // Probe at most probe_limit_ slots to find a key.
    // Because the <key,value> entry is always probe_limit_ away from the initial
    // hashing position, this procedure will find the key if it exists.
    bool find(const key_type& key, mapped_type& recv) {
        uint64_t hash = key_hasher_(key);
        auto seq = probe(ctrl_, hash, capacity_);
        while (true) {
            Group g{ctrl_ + seq.offset()};
            for (int i : g.Match(H2(hash))) {
                size_t new_i = seq.offset(i);
                if (PREDICT_TRUE(key_equal_(key, keys_[new_i]))){
                    recv = values_[new_i];
                    return true;
                }
            }
            if (PREDICT_TRUE(g.MatchEmpty())) 
                return false;
            seq.next();
            assert(seq.index() < capacity_ && "full table!");
        }
    }

    bool findPointer(const key_type& key, mapped_type** recvPtr) {
        uint64_t hash = key_hasher_(key);
        auto seq = probe(ctrl_, hash, capacity_);
        while (true) {
            Group g{ctrl_ + seq.offset()};
            for (int i : g.Match(H2(hash))) {
                size_t new_i = seq.offset(i);
                if (PREDICT_TRUE(key_equal_(key, keys_[new_i]))){
                    *recvPtr = &values_[new_i];
                    return true;
                }
            }
            if (PREDICT_TRUE(g.MatchEmpty())) 
                return false;
            seq.next();
            assert(seq.index() < capacity_ && "full table!");
        }
    }

    bool findIndex(const key_type& key, size_t &index){
        uint64_t hash = key_hasher_(key);
        auto seq = probe(ctrl_, hash, capacity_);
        while (true) {
            Group g{ctrl_ + seq.offset()};
            for (int i : g.Match(H2(hash))) {
                size_t new_i = seq.offset(i);
                if (PREDICT_TRUE(key_equal_(key, keys_[new_i]))){
                    index = new_i;
                    return true;
                }
            }
            if (PREDICT_TRUE(g.MatchEmpty())) 
                return false;
            seq.next();
            assert(seq.index() < capacity_ && "full table!");
        }
    }

    bool erase(const key_type & key) {
      size_t index;
      if(findIndex(key, index)){
        keys_[index].~key_type();
        values_[index].~mapped_type();
        --size_;
        
        const size_t index_before = (index - Group::kWidth) & capacity_;
        const auto empty_after = Group(ctrl_ + index).MatchEmpty();
        const auto empty_before = Group(ctrl_ + index_before).MatchEmpty();

        // We count how many consecutive non empties we have to the right and to the
        // left of `it`. If the sum is >= kWidth then there is at least one probe
        // window that might have seen a full group.
        bool was_never_full =
            empty_before && empty_after &&
            static_cast<size_t>(empty_after.TrailingZeros() +
                                empty_before.LeadingZeros()) < Group::kWidth;

        set_ctrl(index, was_never_full ? kEmpty : kDeleted);
        grow_left_ += was_never_full;
        return true;
      }
      return false;
    }

    void clear() {
        bool needFree = capacity_ > 0 ? true : false;

        for (size_t i = 0; i < capacity_; ++i) {
            if (!IsFull(ctrl_[i])){
                continue;
            }
            set_ctrl(i, kEmpty);
            keys_[i].~key_type();
            values_[i].~mapped_type();
        }
        size_ = 0;

        // destructor
        if (needFree) {
            myFree(ctrl_unaligned_);
            myFree(keys_unaligned_);
            myFree(values_unaligned_);
        }

        // constructor
        endIterator = iterator();

        old_ctrl_unaligned_ = nullptr;
        old_keys_unaligned_ = nullptr;
        old_values_unaligned_ = nullptr;

        ctrl_unaligned_ = nullptr;
        keys_unaligned_ = nullptr;
        values_unaligned_ = nullptr;
    
        ctrl_ = EmptyGroup();
        keys_ = nullptr;
        values_ = nullptr;

        size_ = 0;
        capacity_ = 0;
        grow_left_ = 0;

        initialize();
        endIterator = iterator(this, capacity_);
    }

    inline size_t size() {
        return size_;
    }

    inline size_t capacity() {
        return capacity_;
    }

    void drop_deletes_without_resize() {
        assert(IsValidCapacity(capacity_));
        assert(!is_small(capacity_));
        // Algorithm:
        // - mark all DELETED slots as EMPTY
        // - mark all FULL slots as DELETED
        // - for each slot marked as DELETED
        //     hash = Hash(element)
        //     target = find_first_non_full(hash)
        //     if target is in the same group
        //       mark slot as FULL
        //     else if target is EMPTY
        //       transfer element to target
        //       mark slot as EMPTY
        //       mark target as FULL
        //     else if target is DELETED
        //       swap current element with target element
        //       mark target as FULL
        //       repeat procedure for current slot with moved from element (target)


        // back_up ctrl_, keys_, values_
        size_t ctrl_sz = (capacity_ + Group::kWidth) * sizeof(ctrl_t);
        size_t keys_sz = (capacity_ + Group::kWidth) * sizeof(key_type); 
        size_t vals_sz = (capacity_ + Group::kWidth) * sizeof(mapped_type);
        void *temp_ctrl_unaligned, *temp_keys_unaligned, *temp_values_unaligned;
        try{
            temp_ctrl_unaligned = myAlloc(ctrl_sz + CACHE_LINE_SIZE - 1);
        } catch(...){
            throw;
        }
        try {
            temp_keys_unaligned = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
        } catch(...) {
            myFree(temp_ctrl_unaligned);
            throw;
        }
        try {
            temp_values_unaligned = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
        } catch(...) {
            myFree(temp_ctrl_unaligned);
            myFree(temp_keys_unaligned);
            throw;
        }
        void* temp_ctrl_aligned = (void *)(((size_t)temp_ctrl_unaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* temp_keys_aligned = (void *)(((size_t)temp_keys_unaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* temp_values_aligned = (void *)(((size_t)temp_values_unaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        ctrl_t* temp_ctrl = (ctrl_t *)temp_ctrl_aligned;
        key_type*  temp_keys = (key_type *)temp_keys_aligned;
        mapped_type*  temp_values = (mapped_type *)temp_values_aligned;
        assert((size_t)temp_ctrl % CACHE_LINE_SIZE == 0);
        assert((size_t)temp_keys % CACHE_LINE_SIZE == 0);
        assert((size_t)temp_values % CACHE_LINE_SIZE == 0);
        memcpy(temp_ctrl, ctrl_, sizeof(ctrl_t) * (capacity_ + Group::kWidth));
        try{
            for (size_t i = 0; i != capacity_; ++i) {
                if (IsFull(ctrl_[i])) {
                    new ((char*)&temp_keys[i]) key_type(keys_[i]);
                    try {
                        new ((char*)&temp_values[i]) mapped_type(values_[i]);
                    } catch (...) {
                        temp_keys[i].~key_type();
                        throw;
                    }
                }
            }
        } catch(...){
            for(size_t i = 0; i < capacity_; i++){
                if (IsFull(ctrl_[i])) {
                    temp_keys[i].~key_type();
                    temp_values[i].~mapped_type();
                }
            }
            myFree(temp_ctrl_unaligned);
            myFree(temp_keys_unaligned);
            myFree(temp_values_unaligned);
            throw; 
        }

        ConvertDeletedToEmptyAndFullToDeleted(ctrl_, capacity_);

        try{
            for (size_t i = 0; i != capacity_; ++i) {
                if (!IsDeleted(ctrl_[i])) 
                    continue;
                const size_t hash = key_hasher_(keys_[i]);
                const FindInfo target = find_first_non_full(ctrl_, hash, capacity_);
                const size_t new_i = target.offset;

                // Verify if the old and new i fall within the same group wrt the hash.
                // If they do, we don't need to move the object as it falls already in the
                // best probe we can.
                const size_t probe_offset = probe(ctrl_, hash, capacity_).offset();
                const auto probe_index = [probe_offset, this](size_t pos) {
                    return ((pos - probe_offset) & capacity_) / Group::kWidth;
                };

                // Element doesn't move.
                if (PREDICT_TRUE(probe_index(new_i) == probe_index(i))) {
                    set_ctrl(i, H2(hash));
                    continue;
                }
                if (IsEmpty(ctrl_[new_i])) {
                    // Transfer element to the empty spot.
                    // set_ctrl poisons/unpoisons the slots so we have to call it at the
                    // right time.
                    new ((char*)&keys_[new_i]) key_type(keys_[i]);
                    try {
                        new ((char*)&values_[new_i]) mapped_type(values_[i]);
                    } catch (...) {
                        keys_[new_i].~key_type();
                        throw;
                    }
                    set_ctrl(new_i, H2(hash));
                    keys_[i].~key_type();
                    values_[i].~mapped_type();
                    set_ctrl(i, kEmpty);
                } else {
                    assert(IsDeleted(ctrl_[new_i]));        
                    // Until we are done rehashing, DELETED marks previously FULL slots.
                    // Swap i and new_i elements.
                    std::swap(keys_[i], keys_[new_i]);
                    std::swap(values_[i], values_[new_i]);
                    set_ctrl(new_i, H2(hash));
                    --i;  // repeat
                }
            }
        }catch(...){
            for(size_t i = 0; i < capacity_; i++){
                if (IsFull(ctrl_[i])) {
                    keys_[i].~key_type();
                    values_[i].~mapped_type();
                }
            }
            myFree(keys_unaligned_);
            myFree(values_unaligned_);
            memcpy(ctrl_, temp_ctrl,  sizeof(ctrl_t) * (capacity_ + Group::kWidth));
            myFree(temp_ctrl_unaligned);
            keys_ = temp_keys;
            values_ = temp_values;
            keys_unaligned_ = temp_keys_unaligned;
            values_unaligned_ = temp_values_unaligned;
            throw;
        }
        

        for(size_t i = 0; i < capacity_; i++){
            if (IsFull(temp_ctrl[i])) {
                temp_keys[i].~key_type();
                temp_values[i].~mapped_type();
            }
        }

        myFree(temp_ctrl_unaligned);
        myFree(temp_keys_unaligned);
        myFree(temp_values_unaligned);
        grow_left_ = CapacityToGrowth(capacity_) - size_;
    }

    void resize(size_t new_capacity) {
        assert(IsValidCapacity(new_capacity));
        ctrl_t* old_ctrl = ctrl_;
        key_type* old_keys = keys_;
        mapped_type* old_values = values_; 
        const size_t old_capacity = capacity_;
        int old_grow_left = grow_left_;
        capacity_ = new_capacity;

        // realloc memory
        size_t ctrl_sz = (capacity_ + Group::kWidth) * sizeof(ctrl_t);
        size_t keys_sz = (capacity_ + Group::kWidth) * sizeof(key_type); 
        size_t vals_sz = (capacity_ + Group::kWidth) * sizeof(mapped_type);
        try{
            ctrl_unaligned_ = myAlloc(ctrl_sz + CACHE_LINE_SIZE - 1);
            try {
                keys_unaligned_ = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
            } catch(...) {
                myFree(ctrl_unaligned_);
                throw;
            }
            try {
                values_unaligned_ = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
            } catch(...) {
                myFree(ctrl_unaligned_);
                myFree(keys_unaligned_);
                throw;
            }
        }catch(...){
            ctrl_unaligned_ = old_ctrl_unaligned_;
            keys_unaligned_ = old_keys_unaligned_;
            values_unaligned_ = old_values_unaligned_;
            capacity_ = old_capacity;
            throw;
        }
        void* ctrl_aligned = (void *)(((size_t)ctrl_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* keys_aligned = (void *)(((size_t)keys_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* values_aligned = (void *)(((size_t)values_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        ctrl_ = (ctrl_t *)ctrl_aligned;
        keys_ = (key_type *)keys_aligned;
        values_ = (mapped_type *)values_aligned;
        assert((size_t)ctrl_ % CACHE_LINE_SIZE == 0);
        assert((size_t)keys_ % CACHE_LINE_SIZE == 0);
        assert((size_t)values_ % CACHE_LINE_SIZE == 0);

        std::memset(ctrl_, kEmpty, capacity_ + Group::kWidth);
        ctrl_[capacity_] = kSentinel;
        grow_left_ = CapacityToGrowth(capacity_) - size_;
        
        // copy memory from old to new
        try{
            for (size_t i = 0; i != old_capacity; ++i) {
                if (IsFull(old_ctrl[i])) {
                    uint64_t hash = key_hasher_(old_keys[i]);
                    auto target = find_first_non_full(ctrl_, hash, capacity_);
                    size_t new_i = target.offset;
                    new ((char*)&keys_[new_i]) key_type(old_keys[i]);
                    try {
                        new ((char*)&values_[new_i]) mapped_type(old_values[i]);
                    } catch (...) {
                        keys_[new_i].~key_type();
                        throw;
                    }
                    set_ctrl(new_i, H2(hash));
                }
            }
        }catch(...){
            for(size_t i = 0; i < capacity_; i++){
                if (IsFull(ctrl_[i])) {
                    keys_[i].~key_type();
                    values_[i].~mapped_type();
                }
            }
            myFree(ctrl_unaligned_);
            myFree(keys_unaligned_);
            myFree(values_unaligned_);
            ctrl_ = old_ctrl;
            keys_ = old_keys;
            values_ = old_values;
            capacity_ = old_capacity;
            ctrl_unaligned_ = old_ctrl_unaligned_;
            keys_unaligned_ = old_keys_unaligned_;
            values_unaligned_ = old_values_unaligned_;
            grow_left_ = old_grow_left;
            throw; 
        }

        // release old memory
        if (old_capacity) {
            for (size_t i = 0; i < old_capacity; ++i) {
              if (IsFull(old_ctrl[i])){
                  old_keys[i].~key_type();
                  old_values[i].~mapped_type();
              }
            }
            myFree(old_ctrl_unaligned_);
            myFree(old_keys_unaligned_);
            myFree(old_values_unaligned_);
            // auto layout = MakeLayout(old_capacity);
            // Deallocate<Layout::Alignment()>(&alloc_ref(), old_ctrl, layout.AllocSize());
        }
        old_ctrl_unaligned_ = ctrl_unaligned_;
        old_keys_unaligned_ = keys_unaligned_;
        old_values_unaligned_ = values_unaligned_;
        
        endIterator.setStartIdx(capacity_);
    }

    void rehash_and_grow_if_necessary() {
        if (capacity_ == 0) {
            resize(1);
        } else if (size_ <= CapacityToGrowth(capacity_) / 2) {
            // Squash DELETED without growing if there is enough capacity.
            drop_deletes_without_resize();
        } else {
            // Otherwise grow the container.
            resize(capacity_ * 2 + 1);
        }
    }

    // Sets the control byte, and if `i < Group::kWidth - 1`, set the cloned byte at the end too.
    void set_ctrl(size_t i, ctrl_t h) {
        assert(i < capacity_);
        ctrl_[i] = h;
        ctrl_[((i - NumClonedBytes()) & capacity_) +
            (NumClonedBytes() & capacity_)] = h;
    }

    void initialize(){
        size_ = 0;
        capacity_ = 0;
    }

    inline bool keyPresent(size_t idx) const {
        return IsFull(ctrl_[idx]);
    }

    struct iterator {
        const key_type & key() {
            return impl->keys_[idx];
        }
        const mapped_type & value() {
            return impl->values_[idx];
        }
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(SwissTableImpl<Key, T,  Hasher, KeyEqual>* impl_, int startIdx = -1):idx(startIdx), impl(impl_) {
            end = impl_->capacity();
            this->next();
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
        void setStartIdx(const int startIdx){
            idx = startIdx;
        }
    private:
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        SwissTableImpl<Key, T, Hasher, KeyEqual>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }

private:
    iterator endIterator;
    static key_hasher key_hasher_;
    static key_equal key_equal_;

    void* old_ctrl_unaligned_;     // [(capacity + 1 + NumClonedBytes()) * ctrl_t]
    void* old_keys_unaligned_;     // [capacity * key_type]
    void* old_values_unaligned_;   // [capacity * value+type]

    void* ctrl_unaligned_;         // [(capacity + 1 + NumClonedBytes()) * ctrl_t]
    void* keys_unaligned_;         // [capacity * key_type]
    void* values_unaligned_;       // [capacity * value+type]
    
    ctrl_t* ctrl_ = EmptyGroup();  // [(capacity + 1 + NumClonedBytes()) * ctrl_t]
    key_type* keys_;               // [capacity * key_type]
    mapped_type* values_;          // [capacity * value+type]

    size_t size_ = 0;              // number of full slots
    size_t capacity_ = 0;          // total number of slots
    int grow_left_ = 0;
};

template<typename Key,
         typename T,
         typename Hasher,
         typename KeyEqual>
Hasher SwissTableImpl<Key, T, Hasher, KeyEqual>::key_hasher_;

template<typename Key,
         typename T,
         typename Hasher,
         typename KeyEqual>
KeyEqual SwissTableImpl<Key, T, Hasher, KeyEqual>::key_equal_;

template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
class FlatHashmapImpl {
public:
	typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;

    FlatHashmapImpl(const FlatHashmapImpl&)=delete;
    FlatHashmapImpl& operator=(const FlatHashmapImpl&)=delete;
    
    friend struct iterator;
    FlatHashmapImpl(size_t cap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f, bool predictedCap = false, bool log2_problimit = true)
        : hash_policy_(cap, predictedCap) {
        initialize(hash_policy_.capacity(), probeLimitScalingFactor, log2_problimit);
        endIterator = iterator(this, this->hash_policy_.capacity() + this->probe_limit_);
    }

    ~FlatHashmapImpl() {
        clear();
        myFree(headers_ptr_unaligned_);
        myFree(keys_ptr_unaligned_);
        myFree(values_ptr_unaligned_);
    }
    inline bool keyPresent(size_t idx) const {
        return this->headers[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->headers[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->headers[idx >> 5] &= ~(1 << (idx & 31));
    }

    int insertNoErase(const key_type & key, const mapped_type & value) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
                //key unmatched
                continue;
            }
            new ((char*)&this->keys[i]) key_type(key);
            try {
                new ((char*)&this->values[i]) mapped_type(value);
            } catch (...) {
                this->keys[i].~key_type();
                throw;
            }
            setKeyPresent(i);
            ++this->count_;
            ret = INSERT_GOOD;
            break;
        }
        return ret;
    }

    // Insert <key, value>.
    // This version is used by hashmap that does not support erase.
    // Insertion procedure starts at initial hashing position and stops either:
    //  1. a empty slot is found -> INSERT_GOOD
    //  2. a non empty slot having the same key -> INSERT_KEY_EXISTS
    //  3. probe_limit_ positions are traversed and still no vacant slots -> INSERT_NO_VACANCY
    // Return Values:
    //  INSERT_NO_VACANCY : the insertion procedure fails to find a empty slot within probe_limit_ away from the initial hashing position. This suggests a rehash with larger capacity is needed to spread out the keys.
    //  INSERT_GOOD: the insertion is successful.
    //  INSERT_KEY_EXISTS : the insertion proceudre fails because the key already exists
    int insertNoErase(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    if (recvPtr != nullptr)
                        *recvPtr = &this->values[i];
                    break;
                }
                //key unmatched
                continue;
            }
            new ((char*)&this->keys[i]) key_type(key);
            try {
                new ((char*)&this->values[i]) mapped_type(value);
            } catch (...) {
                this->keys[i].~key_type();
                throw;
            }
            setKeyPresent(i);
            ++this->count_;
            ret = INSERT_GOOD;
            if (recvPtr != nullptr)
                *recvPtr = &this->values[i];
            break;
        }
        return ret;
    }

    // Insert <key, value>
    // This version is used by hashmap that supports erase.
    // Return Values:
    //  INSERT_NO_VACANCY : the insertion procedure fails to find a empty slot within probe_limit_ away from the initial hashing position. This suggests a rehash with larger capacity is needed to spread out the keys.
    //  INSERT_GOOD: the insertion is successful.
    //  INSERT_KEY_EXISTS : the insertion proceudre fails because the key already exists
    int insert(const key_type & key, const mapped_type & value) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        // It's required to examine all probe_limit_ slots to finding the right insert position.
        // For example:
        // insert 1 => 1 _ _ _
        // insert 2 => 1 2 _ _
        // erase 1  => _ 2 _ _
        // insert 2 => 2 2 _ _  // wrong!
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
            } else if (insertSlot == -1) {
                // insert at the first free slot
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            new ((char*)&this->keys[insertSlot]) key_type(key);
            try {
                new ((char*)&this->values[insertSlot]) mapped_type(value);
            } catch (...) {
                this->keys[insertSlot].~key_type();
                throw;
            }
            setKeyPresent(insertSlot);
            ++this->count_;
            ret = INSERT_GOOD;
        }
        return ret;
    }

    // Insert <key, value>
    // Similar to the "int insert(const key_type & key, const mapped_type & value)",
    // but additionally returns a pointer to :
    //  1. the address that stores the value if the insertion is successful
    //  2. the addres that stores the existing value if the key already exists in the table
    int insert(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    if (recvPtr != nullptr)
                        *recvPtr = &this->values[i];
                    break;
                }
            } else if (insertSlot == -1) {
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            new ((char*)&this->keys[insertSlot]) key_type(key);
            try {
                new ((char*)&this->values[insertSlot]) mapped_type(value);
            } catch (...) {
                this->keys[insertSlot].~key_type();
                throw;
            }
            setKeyPresent(insertSlot);
            ++this->count_;
            ret = INSERT_GOOD;
            if (recvPtr != nullptr)
                *recvPtr = &this->values[insertSlot];
        }
        return ret;
    }

    // Probe at most probe_limit_ slots to find a key.
    // Because the <key,value> entry is always probe_limit_ away from the initial
    // hashing position, this procedure will find the key if it exists.
    bool find(const key_type & key, mapped_type & recv) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                recv = this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    // Similar to bool find(const key_type & key, mapped_type & recv),
    // but returns a address that stores the value if the key exists.
    bool findPointer(const key_type & key, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                *recvPtr = &this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }


    // Probe at most probe_limit_ slots to find a key.
    // This version is used by hashmap that does not support erase.
    // Since no erasure is supported, the probing stops whenever a empty slots is found.
    bool findNoErase(const key_type & key, mapped_type & recv) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->keys[i] == key) {
                recv = this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool findPointerNoErase(const key_type & key, mapped_type ** recvPtr) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->key_equal_(this->keys[i], key)) {
                *recvPtr = &this->values[i];
                found = true;
                break;
            }
        }
        return found;
    }

    bool erase(const key_type & key) {
        uint64_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool erased = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                setKeyUnpresent(i);
                this->keys[i].~key_type();
                this->values[i].~mapped_type();
                --this->count_;
                erased = true;
                break;
            }
        }
        return erased;
    }

    void clear() {
        size_t realCapacity = this->hash_policy_.capacity() + this->probe_limit_;
        for (size_t i = 0; i < realCapacity; ++i) {
            if (keyPresent(i) == false)
                continue;
            setKeyUnpresent(i);
            this->keys[i].~key_type();
            this->values[i].~mapped_type();
        }
        count_ = 0;
    }

    static FlatHashmapImpl* growFrom(const FlatHashmapImpl & from, bool noErase, float probeLimitScalingFactor, const uint64_t maxItemsHint, const uint64_t inserts) {
        uint64_t oldCap = from.hash_policy_.capacity();
        uint64_t newCap = oldCap + 1;
        bool predicted = false;
        if (maxItemsHint && inserts >= 50000) { // take 50000 samples
            assert(from.size() <= inserts);
            double uniqueKeysInsertsRatio = from.size() / (inserts + 0.0); 
            double expectedCapacity = (uniqueKeysInsertsRatio * maxItemsHint) / (from.size() / (oldCap + 0.0)) * 1.3; // expected # unique keys / load_factor
            newCap = std::max(expectedCapacity, newCap+0.0);
            predicted = true;
        }
        std::unique_ptr<FlatHashmapImpl> to;
        double load_factor = from.size() / (oldCap + 0.0);
        bool log2_problimit = load_factor > 0.1;
        while (true) {
            to.reset(new FlatHashmapImpl(newCap, probeLimitScalingFactor, predicted, log2_problimit));
            if (noErase) {
                if (rehashNoErase(from, *to))
                    break;
            } else {
                if (rehash(from, *to))
                    break;
            }
            assert (to->hash_policy_.capacity() + 1 > newCap);
            newCap = to->hash_policy_.capacity() + 1;
        }
        return to.release();
    }

    inline size_t size() const {
        return count_;
    }

    inline size_t capacity() const {
        return this->hash_policy_.capacity();
    }
    
    struct iterator {
        const key_type & key() {
            return impl->keys[idx];
        }
        const mapped_type & value() {
            return impl->values[idx];
        }
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>* impl_, int startIdx = -1):idx(startIdx), impl(impl_) {
            end = impl->hash_policy_.capacity() + impl->probe_limit_;
            this->next();
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
    private:
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }

private:
    iterator endIterator;
    static bool rehash(const FlatHashmapImpl & from, FlatHashmapImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insert(from.keys[i], from.values[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    static bool rehashNoErase(const FlatHashmapImpl & from, FlatHashmapImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insertNoErase(from.keys[i], from.values[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    void initialize(size_t cap, float probeLimitScalingFactor, bool log2_problimit = true) {
        assert(probeLimitScalingFactor > 0);
        // probe_limit = log2(n)
        if (log2_problimit) {
            this->probe_limit_ = std::ceil(std::log2(cap)) * probeLimitScalingFactor;
        } else {
            this->probe_limit_ = cap;
        }
        cap += probe_limit_; //allocate probe_limit_ more slots to avoid bound-checking
        size_t headers_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make headers_sz multiple of sizeof(uint32_t)
        assert(headers_sz % sizeof(uint32_t) == 0);
        size_t keys_sz = cap * sizeof(key_type);
        size_t vals_sz = cap * sizeof(mapped_type);
        this->count_ = 0;
        this->headers_ptr_unaligned_ = this->keys_ptr_unaligned_ = this->values_ptr_unaligned_ = nullptr;
        this->headers_ptr_unaligned_ = myAlloc(headers_sz + CACHE_LINE_SIZE - 1);
        try {
            this->keys_ptr_unaligned_ = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
        } catch(...) {
            myFree(this->headers_ptr_unaligned_);
            throw;
        }

        try {
            this->values_ptr_unaligned_ = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
        } catch(...) {
            myFree(this->headers_ptr_unaligned_);
            myFree(this->keys_ptr_unaligned_);
            throw;
        }
        // align slots at cacheline granularity for better speed
        void* headers_ptr_aligned = (void *)(((size_t)this->headers_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* keys_ptr_aligned = (void *)(((size_t)this->keys_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* values_ptr_aligned = (void *)(((size_t)this->values_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->headers = new(headers_ptr_aligned) uint32_t[cap / 8];
        this->keys = (key_type *)keys_ptr_aligned;
        this->values = (mapped_type *)values_ptr_aligned;
        assert((size_t)headers_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)keys_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)values_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->headers_ptr_unaligned_, 0, headers_sz + CACHE_LINE_SIZE - 1);
    }

    static key_hasher key_hasher_;
    static key_equal key_equal_;
    hash_policy hash_policy_;
    void* headers_ptr_unaligned_;
    void* keys_ptr_unaligned_;
    void* values_ptr_unaligned_;
    uint32_t* headers;
    key_type* keys;
    mapped_type* values;
    size_t probe_limit_;
    size_t count_; // # elements in the table
};

template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
Hasher FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::key_hasher_;
template<typename Key,
         typename T,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
KeyEqual FlatHashmapImpl<Key, T, HashPolicy, Hasher, KeyEqual>::key_equal_;

/*
 * A hashmap implementation that is based on the idea of bounded linear-probing.
 * Insertion/Lookup probes at most probe_limit_ of slots before resorting to rehash.
 * probe_limit_ is set to log2(table capacity).
 */
template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = XXHasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class FlatHashmap {
public:
    typedef Key key_type;
	typedef T mapped_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    FlatHashmap(size_t initialCap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f)
        : impl(new SwissTableImpl<Key, T, Hasher, KeyEqual>()) {
        impl->resize(NormalizeCapacity(initialCap));
    }
    
    inline bool find(const key_type & key, mapped_type & recv) {
        return impl->find(key, recv);
    }

    inline bool findPointer(const key_type& key, mapped_type** recvPtr) {
        return impl->findPointer(key, recvPtr);
    }

    bool insert(const key_type & key, const mapped_type & value) {
        int ret = impl->insert(key, value);
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        return false;
    }

    mapped_type& operator[] (const key_type & key) {
        mapped_type *recvPtr;
        bool found = impl->findPointer(key, &recvPtr);
        if (found == true)
            return *recvPtr;
        impl->insert(key, mapped_type(), &recvPtr);
        return *recvPtr;
    }

    bool upsert(const key_type & key, const mapped_type & value) {
        mapped_type *recvPtr;
        bool found = impl->findPointer(key, &recvPtr);
        if (found == true) {
            *recvPtr = value;
            return false;
        }
        int ret = impl->insert(key, value);
        if (ret == INSERT_GOOD || ret == INSERT_KEY_EXISTS)
            return true;
        return false;
    }

    inline bool erase(const key_type & key) {
        return impl->erase(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }
    struct const_iterator {
        const key_type & key() {
            return it.key();
        }
        const mapped_type & value() {
            return it.value();
        }
        
        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator& operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(SwissTableImpl<Key, T, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename SwissTableImpl<Key, T, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename SwissTableImpl<Key, T, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<SwissTableImpl<Key, T, Hasher, KeyEqual>> impl;
};


template<typename Key,
         typename T,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = XXHasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
using IrremovableFlatHashmap = FlatHashmap<Key, T, HashPolicy, Hasher, KeyEqual>;

/* Use bitmap to speed up hashmap for dense integral keys */
template<typename Key,
         typename T>
class FlatBitmap {
public:
    static_assert(std::is_integral<Key>::value, "Key must be one of integral types");
	typedef Key key_type;
	typedef T mapped_type;
	typedef size_t size_type;

    FlatBitmap(const FlatBitmap&)=delete;
    FlatBitmap& operator=(const FlatBitmap&)=delete;

    friend struct iterator;

    FlatBitmap(const key_type & minKey_, const key_type & maxKey_): minKey(minKey_), maxKey(maxKey_) {
        if (maxKey < minKey) {
            throw std::runtime_error("maxKey must be greater than or equal to minKey");
        }
        initialize();
        endIterator = iterator(this, this->cap);
    }

    ~FlatBitmap() {
        clear();
        myFree(keyBitmapPtrUnaligned);
        myFree(valuesPtrUnaligned);
    }

    inline bool keyPresent(size_t idx) const {
        return this->keyBitmap[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->keyBitmap[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->keyBitmap[idx >> 5] &= ~(1 << (idx & 31));
    }

    inline size_t key2Idx(const key_type & key) {
        return (int64_t)key - minKey;
    }

    inline const key_type idx2Key(int64_t idx) {
        return idx + minKey;
    }

    inline bool insert(const key_type & key, const mapped_type & value) {
        int64_t idx = key2Idx(key);
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            new((char*)&this->values[idx]) mapped_type(value);
            ++this->count;
            return true;
        }
        return false;
    }

    inline bool find(const key_type & key, mapped_type & recv) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            recv = this->values[idx];
            return true;
        }
        return false;
    }

    inline mapped_type& operator[] (const key_type & key) {
        mapped_type *recvPtr;
        if (findPointer(key, &recvPtr) == false) {
            insert(key, mapped_type(), &recvPtr);
        }
        // tail-recursive call
        return *recvPtr;
    }

    inline bool erase(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            setKeyUnpresent(idx);
            this->values[idx].~mapped_type();
            --this->count;
            return true;
        }
        return false;
    }

    inline void clear() {
        size_t capcity = this->cap;
        for (size_t i = 0; i < capcity; ++i) {
            if (keyPresent(i) == false)
                continue;
            this->values[i].~mapped_type();
            setKeyUnpresent(i);
        }
        count = 0;
    }

    inline size_t size() const {
        return this->count;
    }

    inline size_t capacity() const {
        return this->cap;
    }

    struct iterator {
        const key_type key() {
            return impl->idx2Key(idx);
        }
        const mapped_type & value() {
            return impl->values[idx];
        }
        
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
        friend FlatBitmap<Key,T>;
    private:
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatBitmap<Key, T>* impl_, int startIdx = -1): idx(startIdx), impl(impl_) {
            end = impl->cap;
            this->next();
        }
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatBitmap<Key, T>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }
private:
    bool findPointer(const key_type & key, mapped_type ** recvPtr) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            *recvPtr = &this->values[idx];
            return true;
        }
        return false;
    }

    int insert(const key_type & key, const mapped_type & value, mapped_type ** recvPtr) {
        int64_t idx = key2Idx(key);
        *recvPtr = &this->values[idx];
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            new((char*)&this->values[idx]) mapped_type(value);
            ++this->count;
            return true;
        }
        return false;
    }


    void initialize() {
        cap = (int64_t)this->maxKey - (int64_t)this->minKey + 1;
        size_t bitmap_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make bitmap_sz multiple of sizeof(uint32_t)
        assert(bitmap_sz % sizeof(uint32_t) == 0);
        size_t vals_sz = cap * sizeof(mapped_type);
        this->count = 0;
        this->keyBitmapPtrUnaligned = this->valuesPtrUnaligned = nullptr;
        this->keyBitmapPtrUnaligned = myAlloc(bitmap_sz + CACHE_LINE_SIZE - 1);
        try {
            this->valuesPtrUnaligned = myAlloc(vals_sz + CACHE_LINE_SIZE - 1);
        } catch(...) {
            myFree(this->keyBitmapPtrUnaligned);
            throw;
        }
        void* bitmap_ptr_aligned = (void *)(((size_t)this->keyBitmapPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* values_ptr_aligned = (void *)(((size_t)this->valuesPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->keyBitmap = new(bitmap_ptr_aligned) uint32_t[cap / 8];
        this->values = (mapped_type *)values_ptr_aligned;
        assert((size_t)bitmap_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)values_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->keyBitmapPtrUnaligned, 0, bitmap_sz + CACHE_LINE_SIZE - 1);
        //cache warmup
        memset(this->valuesPtrUnaligned, 0, vals_sz + CACHE_LINE_SIZE - 1);
    }

    iterator endIterator;
    void* keyBitmapPtrUnaligned;
    void* valuesPtrUnaligned;
    uint32_t* keyBitmap;
    mapped_type* values;
    size_t count; // # elements in the table
    size_t cap;
    key_type minKey;
    key_type maxKey;
};

/* Use bitmap to speed up hashset and for dense integral types*/
template<typename Key>
class FlatBitset {
public:
    static_assert(std::is_integral<Key>::value, "Key must be one of integral types");
	typedef Key key_type;
	typedef size_t size_type;

    FlatBitset(const FlatBitset&)=delete;
    FlatBitset& operator=(const FlatBitset&)=delete;

    friend struct iterator;

    FlatBitset(const key_type & minKey_, const key_type & maxKey_): minKey(minKey_), maxKey(maxKey_) {
        if (!std::is_integral<Key>::value) {
            throw std::runtime_error("Only integral key types are supported");
        }
        initialize();
        endIterator = iterator(this, this->cap);
    }

    ~FlatBitset() {
        myFree(keyBitmapPtrUnaligned);
    }

    inline bool keyPresent(int idx) const {
        return this->keyBitmap[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(int idx) {
        assert(keyPresent(idx) == false);
        this->keyBitmap[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(int idx) {
        int scaledIdx = idx >> 5;
        int bitShiftInInt = idx & 31;
        this->keyBitmap[scaledIdx] &= ~(1 << bitShiftInInt);
    }

    inline int64_t key2Idx(const key_type & key) {
        return key - minKey;
    }

    inline const key_type idx2Key(int64_t idx) {
        return idx + minKey;
    }

    inline bool insert(const key_type & key) {
        int64_t idx = key2Idx(key);
        if (!keyPresent(idx)) {
            setKeyPresent(idx);
            ++this->count;
            return true;
        }
        return false;
    }

    inline bool find(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            return true;
        }
        return false;
    }

    inline bool erase(const key_type & key) {
        if (key < this->minKey || key > this->maxKey)
            return false;
        int64_t idx = key2Idx(key);
        if (keyPresent(idx)) {
            setKeyUnpresent(idx);
            --this->count;
            return true;
        }
        return false;
    }

    inline void clear() {
        size_t capcity = this->cap;
        for (size_t i = 0; i < capcity; ++i) {
            setKeyUnpresent(i);
        }
        count = 0;
    }

    inline size_t size() const {
        return this->count;
    }

    inline size_t capacity() const {
        return this->cap;
    }

    struct iterator {
        const key_type key() {
            return impl->idx2Key(idx);
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
        friend FlatBitset<Key>;
    private:
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatBitset<Key>* impl_, int startIdx = -1): idx(startIdx), impl(impl_) {
            end = impl->cap;
            this->next();
        }
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatBitset<Key>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }
private:
    void initialize() {
        cap = this->maxKey - this->minKey + 1;
        size_t bitmap_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make bitmap_sz multiple of sizeof(uint32_t)
        assert(bitmap_sz % sizeof(uint32_t) == 0);
        this->count = 0;
        this->keyBitmapPtrUnaligned = myAlloc(bitmap_sz + CACHE_LINE_SIZE - 1);
        void* bitmap_ptr_aligned = (void *)(((size_t)this->keyBitmapPtrUnaligned + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->keyBitmap = new(bitmap_ptr_aligned) uint32_t[cap / 8];
        assert((size_t)bitmap_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->keyBitmapPtrUnaligned, 0, bitmap_sz + CACHE_LINE_SIZE - 1);
    }

    iterator endIterator;
    void* keyBitmapPtrUnaligned;
    uint32_t* keyBitmap;
    size_t count; // # elements in the table
    size_t cap;
    key_type minKey;
    key_type maxKey;
};


template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
class FlatHashsetImpl {
public:
	typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;

    FlatHashsetImpl(const FlatHashsetImpl&)=delete;
    FlatHashsetImpl& operator=(const FlatHashsetImpl&)=delete;
    
    friend struct iterator;
    FlatHashsetImpl(size_t cap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0, bool predictedCap = false, bool log2_problimit = true)
        : hash_policy_(cap, predictedCap){
        initialize(hash_policy_.capacity(), probeLimitScalingFactor, log2_problimit);
        endIterator = iterator(this, this->hash_policy_.capacity() + this->probe_limit_);
    }

    ~FlatHashsetImpl() {
        clear();
        myFree(headers_ptr_unaligned_);
        myFree(keys_ptr_unaligned_);
    }
    inline bool keyPresent(size_t idx) const {
        return this->headers[idx >> 5] & (1 << (idx & 31));
    }

    inline void setKeyPresent(size_t idx) {
        assert(keyPresent(idx) == false);
        this->headers[idx >> 5] |= (1 << (idx & 31));
    }

    inline void setKeyUnpresent(size_t idx) {
        this->headers[idx >> 5] &= ~(1 << (idx & 31));
    }

    int insertNoErase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
                //key unmatched
                continue;
            }
            setKeyPresent(i);
            new((char*)&this->keys[i]) key_type(key);
            ++this->count_;
            ret = INSERT_GOOD;
            break;
        }
        return ret;
    }

    int insert(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        int ret = INSERT_NO_VACANCY;
        int insertSlot = -1;
        // It's required to examine all probe_limit_ slots to finding the right insert position.
        // For example:
        // insert 1 => 1 _ _ _
        // insert 2 => 1 2 _ _
        // erase 1  => _ 2 _ _
        // insert 2 => 2 2 _ _  // wrong!
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i)) {
                if (this->key_equal_(this->keys[i], key)) {
                    ret = INSERT_KEY_EXISTS;
                    break;
                }
            } else if (insertSlot == -1) {
                // insert at the first free slot
                insertSlot = i;
            }
        }
        if (insertSlot != -1 && ret != INSERT_KEY_EXISTS) {
            setKeyPresent(insertSlot);
            new((char*)&this->keys[insertSlot]) key_type(key);
            ++this->count_;
            ret = INSERT_GOOD;
        }
        return ret;
    }

    bool find(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                found = true;
                break;
            }
        }
        return found;
    }

    bool findNoErase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool found = false;
        for(size_t i = idx; i < end && keyPresent(i); ++i) {
            if (this->keys[i] == key) {
                found = true;
                break;
            }
        }
        return found;
    }

    bool erase(const key_type & key) {
        uint32_t hash = this->key_hasher_(key);
        size_t idx = this->hash_policy_.index(hash);
        size_t end = idx + this->probe_limit_;
        bool erased = false;
        for(size_t i = idx; i < end; ++i) {
            if (keyPresent(i) &&
                this->key_equal_(this->keys[i], key)) {
                this->keys[i].~key_type();
                setKeyUnpresent(i);
                --this->count_;
                erased = true;
                break;
            }
        }
        return erased;
    }

    void clear() {
        size_t realCapacity = this->hash_policy_.capacity() + this->probe_limit_;
        for (size_t i = 0; i < realCapacity; ++i) {
            if (keyPresent(i) == false)
                continue;
            this->keys[i].~key_type();
            setKeyUnpresent(i);
        }
        count_ = 0;
    }

    static FlatHashsetImpl* growFrom(const FlatHashsetImpl & from, bool noErase, float probeLimitScalingFactor, const uint64_t maxItemsHint, const uint64_t inserts) {
        size_t oldCap = from.hash_policy_.capacity();
        size_t newCap = oldCap + 1;
        bool predicted = false;
        if (maxItemsHint && inserts >= 50000) { // take 50000 samples
            assert(from.size() <= inserts);
            double uniqueKeysInsertsRatio = from.size() / (inserts + 0.0); 
            double expectedCapacity = (uniqueKeysInsertsRatio * maxItemsHint) / (from.size() / (oldCap + 0.0)); // expected # unique keys / load_factor
            newCap = std::max(expectedCapacity, newCap+0.0);
            //printf("keys %lu, inserts %lu, load_factor %0.2f, uniqueKeysInsertsRatio %0.2lf, predicted Capacity %lu\n", from.size(), inserts,from.size() / (oldCap + 0.0), uniqueKeysInsertsRatio, newCap);
            predicted = true;
        }
        double load_factor = from.size() / (oldCap + 0.0);
        bool log2_problimit = load_factor > 0.1;
        std::unique_ptr<FlatHashsetImpl> to;
        //int failures = 0;
        while (true) {
            to.reset(new FlatHashsetImpl(newCap, probeLimitScalingFactor, predicted, log2_problimit));
            if (noErase) {
                if (rehashNoErase(from, *to))
                    break;
            } else {
                if (rehash(from, *to))
                    break;
            }
            assert (to->hash_policy_.capacity() + 1 > newCap);
            //printf("%d, failed to grow at newCap %d\n",failures++, newCap);
            newCap = to->hash_policy_.capacity() + 1;
        }
        //printf("grew at size %lu from capcity %lu => %lu\n", from.size(), oldCap, to->hash_policy_.capacity());
        return to.release();
    }

    inline size_t size() const {
        return count_;
    }

    inline size_t capacity() const {
        return this->hash_policy_.capacity();
    }
    
    struct iterator {
        const key_type & key() {
            return impl->keys[idx];
        }
        iterator() : idx(-1), end(-1), impl(0){}
        iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>* impl_, int startIdx = -1):idx(startIdx), impl(impl_) {
            end = impl->hash_policy_.capacity() + impl->probe_limit_;
            this->next();
        }
        iterator & operator++() {
            this->next();
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            this->next();
            return old;
        }
        bool operator!=(const iterator & rhs) {
            return idx != rhs.idx;
        }
        bool operator==(const iterator & rhs) {
            return idx == rhs.idx;
        }
    private:
        void next() {
            while (idx < end && impl->keyPresent(++idx) == false);
        }
        int idx;
        int end;
        FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>* impl;
    };

    inline iterator begin() {
        return iterator(this);
    }
    inline iterator end() {
        return endIterator;
    }

private:
    iterator endIterator;
    static bool rehash(const FlatHashsetImpl & from, FlatHashsetImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insert(from.keys[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    static bool rehashNoErase(const FlatHashsetImpl & from, FlatHashsetImpl & to) {
        size_t fromCapacity = from.hash_policy_.capacity() + from.probe_limit_;
        assert(fromCapacity < to.hash_policy_.capacity() + to.probe_limit_);
        bool good = true;
        for (size_t i = 0; i < fromCapacity; ++i) {
            if (from.keyPresent(i) == false)
                continue;
            int ret = to.insertNoErase(from.keys[i]);
            assert(ret != INSERT_KEY_EXISTS);
            good = ret == INSERT_GOOD;
            if (good == false)
                break;
        }
        return good;
    }
    void initialize(size_t cap, float probeLimitScalingFactor, bool log2_problimit = true) {
        assert(probeLimitScalingFactor > 0);
        // probe_limit = log2(n)
        if (log2_problimit) {
            this->probe_limit_ = std::ceil(std::log2(cap)) * probeLimitScalingFactor;
        } else {
            this->probe_limit_ = cap;
        }
        cap += probe_limit_; //allocate probe_limit_ more slots to avoid bound-checking
        size_t headers_sz = ((int)std::ceil(cap / 8.0) + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1); // make headers_sz multiple of sizeof(uint32_t)
        assert(headers_sz % sizeof(uint32_t) == 0);
        size_t keys_sz = cap * sizeof(key_type);
        this->count_ = 0;
        this->headers_ptr_unaligned_ = nullptr;
        this->keys_ptr_unaligned_ = nullptr;
        this->headers_ptr_unaligned_ = myAlloc(headers_sz + CACHE_LINE_SIZE - 1);
        try {
            this->keys_ptr_unaligned_ = myAlloc(keys_sz + CACHE_LINE_SIZE - 1);
        } catch (...) {
            myFree(this->headers_ptr_unaligned_);
            throw;
        }
        void* headers_ptr_aligned = (void *)(((size_t)this->headers_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        void* keys_ptr_aligned = (void *)(((size_t)this->keys_ptr_unaligned_ + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1));
        this->headers = new(headers_ptr_aligned) uint32_t[cap / 8];
        this->keys = (key_type *)keys_ptr_aligned;
        assert((size_t)headers_ptr_aligned % CACHE_LINE_SIZE == 0);
        assert((size_t)keys_ptr_aligned % CACHE_LINE_SIZE == 0);
        memset(this->headers_ptr_unaligned_, 0, headers_sz + CACHE_LINE_SIZE - 1);
    }

    static key_hasher key_hasher_;
    static key_equal key_equal_;
    hash_policy hash_policy_;
    void* headers_ptr_unaligned_;
    void* keys_ptr_unaligned_;
    uint32_t* headers;
    key_type* keys;
    size_t probe_limit_;
    size_t count_; // # elements in the table
};

template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
Hasher FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::key_hasher_;
template<typename Key,
         typename HashPolicy,
         typename Hasher,
         typename KeyEqual>
KeyEqual FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::key_equal_;

template<typename Key,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class FlatHashset {
public:
    typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    FlatHashset(size_t initialCap = DEFAULT_HM_CAPACITY, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>(initialCap)) {
            probeLimitScalingFactor_ = probeLimitScalingFactor;
        }
    FlatHashset(float probeLimitScalingFactor)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>(DEFAULT_HM_CAPACITY, probeLimitScalingFactor)) {
            probeLimitScalingFactor_ = probeLimitScalingFactor;
        }

    inline bool find(const key_type & key) {
        return impl->find(key);
    }
    
    bool insert(const key_type & key) {
        int ret = impl->insert(key);
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, false, probeLimitScalingFactor_, 0, 0);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        // tail-recursive call
        return insert(key);
    }

    inline bool erase(const key_type & key) {
        return impl->erase(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }

    struct const_iterator {
        const key_type & key() {
            return it.key();
        }
        
        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator& operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>> impl;
    float probeLimitScalingFactor_;
};


template<typename Key,
         typename HashPolicy = power2_hash_policy,
         typename Hasher = murmur_hasher<Key>,
         typename KeyEqual = std::equal_to<Key>>
class IrremovableFlatHashset {
public:
    typedef Key key_type;
    typedef Hasher key_hasher;
    typedef KeyEqual key_equal;
	typedef size_t size_type;
    typedef HashPolicy hash_policy;
    IrremovableFlatHashset()
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, 1.0f)),
         inserts_(0), maxItemsHint_(0), probeLimitScalingFactor_(1.0f)
        {}
    IrremovableFlatHashset(uint64_t maxItemsHint, float probeLimitScalingFactor = 1.0f)
        : impl(new FlatHashsetImpl<Key, HashPolicy, Hasher,  KeyEqual>(DEFAULT_HM_CAPACITY, probeLimitScalingFactor)),
         inserts_(0), maxItemsHint_(maxItemsHint) , probeLimitScalingFactor_(probeLimitScalingFactor)
        {}

    inline bool find(const key_type & key) {
        return impl->findNoErase(key);
    }

    bool insert(const key_type & key) {
        int ret = impl->insertNoErase(key);
        ++inserts_;
        if (ret == INSERT_GOOD)
            return true;
        else if (ret == INSERT_KEY_EXISTS)
            return false;
        // ret == INSERT_NO_VACANCY
        auto to = impl->growFrom(*impl, true, probeLimitScalingFactor_, maxItemsHint_, inserts_);
        if (to == nullptr) {
            throw std::runtime_error("failed to grow hashmap");
        }
        impl.reset(to);
        --inserts_;
        // tail-recursive call
        return insert(key);
    }

    inline void clear() {
        impl->clear();
    }

    inline size_t size() {
        return impl->size();
    }

    inline size_t capacity() {
        return impl->capacity();
    }

    struct const_iterator {
        const key_type & key() {
            return it.key();
        }

        bool operator!=(const const_iterator & rhs) {
            return it != rhs.it;
        }
        bool operator==(const const_iterator & rhs) {
            return it == rhs.it;
        }
        const_iterator & operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator old = *this;
            ++it;
            return old;
        }
        const_iterator(FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual> * impl): it(impl->begin()) {}
        const_iterator(const typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator & implIterator): it(implIterator) {}
    private:
        typename FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>::iterator it;
    };

    inline const_iterator begin() {
        return const_iterator(impl.get());
    }
    inline const_iterator end() {
        return const_iterator(impl->end());
    }
private:
    std::unique_ptr<FlatHashsetImpl<Key, HashPolicy, Hasher, KeyEqual>> impl;
    uint64_t inserts_;
    const uint64_t maxItemsHint_;
    float probeLimitScalingFactor_;
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKey {
    static_assert(std::is_integral<RawKeyType>::value, "RawKeyType must be one of integral types");
    RawKeyType keys[keyCounts];
    uint32_t idx;
    MultiCombinedKey():idx(0) {}

    template<typename Key, typename... Rest>
    MultiCombinedKey(const Key & key, Rest... rest):idx(0) {
        addKeys(key, rest...);
    }

    bool operator==(const MultiCombinedKey<keyCounts, RawKeyType> & rhs) const {
        return memcmp(keys, rhs.keys, sizeof(RawKeyType) * idx) == 0;
    }

    inline void addKeys() {}
    template<typename Key, typename... Rest>
    inline void addKeys(const Key & key, Rest... rest) {
        keys[idx++] = key;
        addKeys(rest...);
    }
    
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKeyHasher{ 
    uint64_t operator()(const MultiCombinedKey<keyCounts, RawKeyType> & lhs) {
        return XXHash64((const char *)&lhs.keys, sizeof(lhs.keys));
    }
};

template<int keyCounts, typename RawKeyType>
struct MultiCombinedKeyEqual{ 
    bool operator()(const MultiCombinedKey<keyCounts, RawKeyType> & lhs, const MultiCombinedKey<keyCounts, RawKeyType> & rhs) {
        return memcmp(lhs.keys, rhs.keys, sizeof(lhs.keys)) == 0;
    }
};


template<typename RawKeyType>
struct MultiCombinedKey<2, RawKeyType> {
    RawKeyType keys[2];

    MultiCombinedKey(){}

    template<typename Key1, typename Key2>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2) {
        addKeys(key1, key2);
    }

    template<typename Key1, typename Key2>
    inline void addKeys(const Key1 & key1, const Key2 & key2) {
        keys[0] = key1;
        keys[1] = key2;
    }
    bool operator==(const MultiCombinedKey<2, RawKeyType> & rhs) const {
        return keys[0] == rhs.keys[0] && keys[1] == rhs.keys[1];
    }
};


template<>
struct MultiCombinedKey<2, uint32_t> {
    union{
        uint32_t keys[2];
        uint64_t bits64;
    };
    

    MultiCombinedKey(){}

    template<typename Key1, typename Key2>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2) {
        addKeys(key1, key2);
    }

    template<typename Key1, typename Key2>
    inline void addKeys(const Key1 & key1, const Key2 & key2) {
        keys[0] = key1;
        keys[1] = key2;
    }
    bool operator==(const MultiCombinedKey<2, uint32_t> & rhs) const {
        return bits64 == rhs.bits64;
    }
};

template<typename RawKeyType>
struct MultiCombinedKey<3, RawKeyType> {
    RawKeyType keys[3];

    MultiCombinedKey() {}

    template<typename Key1, typename Key2, typename Key3>
    MultiCombinedKey(const Key1 & key1, const Key2 & key2, const Key3 & key3) {
        addKeys(key1, key2, key3);
    }

    template<typename Key1, typename Key2, typename Key3>
    inline void addKeys(const Key1 & key1, const Key2 & key2, const Key3 & key3) {
        keys[0] = key1;
        keys[1] = key2;
        keys[2] = key3;
    }
    bool operator==(const MultiCombinedKey<3, RawKeyType> & rhs) const {
        return memcmp(keys, rhs.keys, sizeof(keys)) == 0;
    }
};

typedef MultiCombinedKey<2, uint32_t> Double4BKey;
typedef MultiCombinedKey<3, uint32_t> Triple4BKey;
typedef MultiCombinedKeyHasher<2, uint32_t> Double4BKeyHasher;
typedef MultiCombinedKeyEqual<2, uint32_t> Double4BKeyEqual;
typedef MultiCombinedKeyHasher<3, uint32_t> Triple4BKeyHasher;
typedef MultiCombinedKeyEqual<3, uint32_t> Triple4BKeyEqual;
typedef MultiCombinedKey<2, DolphinString> DoubleDStrKey;
typedef MultiCombinedKeyHasher<2, DolphinString> DoubleDStrKeyHasher;
typedef MultiCombinedKeyEqual<2, DolphinString> DoubleDStrKeyEqual;
typedef MultiCombinedKey<2, Guid> DoubleGuidKey;
typedef MultiCombinedKeyHasher<2, Guid> DoubleGuidKeyHasher;
typedef MultiCombinedKeyEqual<2, Guid> DoubleGuidKeyEqual;

template<>
struct murmur_hasher<Double4BKey> {
    uint64_t operator()(const Double4BKey & key) {
        return MultiCombinedKeyHasher<2, uint32_t>{}(key);
    }
};

template<>
struct murmur_hasher<Triple4BKey> {
    uint64_t operator()(const Triple4BKey & key) {
        return MultiCombinedKeyHasher<3, uint32_t>{}(key);
    }
};

template<>
struct XXHasher<Double4BKey> {
    uint64_t operator()(const Double4BKey & key) {
        return MultiCombinedKeyHasher<2, uint32_t>{}(key);
    }
};

template<>
struct XXHasher<Triple4BKey> {
    uint64_t operator()(const Triple4BKey & key) {
        return MultiCombinedKeyHasher<3, uint32_t>{}(key);
    }
};

namespace std
{
    template<> struct hash<Double4BKey>
    {
        typedef Double4BKey argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            return MultiCombinedKeyHasher<2, uint32_t>{}(s);
        }
    };
}

namespace std {
template<>
struct equal_to<Double4BKey> {
    bool operator()(const Double4BKey & key1, const Double4BKey & key2) const {
        return key1 == key2;
    }
};

template<>
struct equal_to<Triple4BKey> {
    bool operator()(const Triple4BKey & key1, const Triple4BKey & key2) const {
        return key1 == key2;
    }
};
}


typedef MultiCombinedKey<2, uint64_t> Double8BKey;
typedef MultiCombinedKey<3, uint64_t> Triple8BKey;
typedef MultiCombinedKeyHasher<2, uint64_t> Double8BKeyHasher;
typedef MultiCombinedKeyEqual<2, uint64_t> Double8BKeyEqual;
typedef MultiCombinedKeyHasher<3, uint64_t> Triple8BKeyHasher;
typedef MultiCombinedKeyEqual<3, uint64_t> Triple8BKeyEqual;

template<>
struct murmur_hasher<Double8BKey> {
    uint64_t operator()(const Double8BKey & key) {
        return MultiCombinedKeyHasher<2, uint64_t>{}(key);
    }
};

template<>
struct murmur_hasher<Triple8BKey> {
    uint64_t operator()(const Triple8BKey & key) {
        return MultiCombinedKeyHasher<3, uint64_t>{}(key);
    }
};

template<>
struct XXHasher<Double8BKey> {
    uint64_t operator()(const Double8BKey & key) {
        return MultiCombinedKeyHasher<2, uint64_t>{}(key);
    }
};

template<>
struct XXHasher<Triple8BKey> {
    uint64_t operator()(const Triple8BKey & key) {
        return MultiCombinedKeyHasher<3, uint64_t>{}(key);
    }
};

namespace std
{
    template<> struct hash<Double8BKey>
    {
        typedef Double8BKey argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            return MultiCombinedKeyHasher<2, uint64_t>{}(s);
        }
    };
}

namespace std {
template<>
struct equal_to<Double8BKey> {
    bool operator()(const Double8BKey & key1, const Double8BKey & key2) const {
        return key1 == key2;
    }
};

template<>
struct equal_to<Triple8BKey> {
    bool operator()(const Triple8BKey & key1, const Triple8BKey & key2) const {
        return key1 == key2;
    }
};
}


#define DEF_MULTI_COMBINED_KEY(count, type, prefix, bytes) \
typedef MultiCombinedKey<count, type> prefix##bytes##BKey; \
typedef MultiCombinedKeyHasher<count, type> prefix##bytes##BKeyHasher; \
typedef MultiCombinedKeyEqual<count, type> prefix##bytes##BKeyEqual; \
\
template<> struct murmur_hasher<prefix##bytes##BKey> { \
    uint64_t operator()(const prefix##bytes##BKey & key) { \
        return prefix##bytes##BKeyHasher{}(key); \
    } \
}; \
template<> struct XXHasher<prefix##bytes##BKey> { \
    uint64_t operator()(const prefix##bytes##BKey & key) { \
        return prefix##bytes##BKeyHasher{}(key); \
    } \
}; \
\
namespace std \
{ \
template<> struct hash<prefix##bytes##BKey> { \
    typedef prefix##bytes##BKey argument_type; \
    typedef std::size_t result_type; \
    result_type operator()(argument_type const& s) const { \
        return prefix##bytes##BKeyHasher{}(s); \
    } \
}; \
template<> struct equal_to<prefix##bytes##BKey> { \
    bool operator()(const prefix##bytes##BKey & key1, const prefix##bytes##BKey & key2) const { \
        return key1 == key2; \
    } \
}; \
} \
//======

DEF_MULTI_COMBINED_KEY(2, wide_integer::int128, Double, 16);  // Double16BKey
DEF_MULTI_COMBINED_KEY(3, wide_integer::int128, Triple, 16);  // Triple16BKey

#undef DEF_MULTI_COMBINED_KEY

#endif
