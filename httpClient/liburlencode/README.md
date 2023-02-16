# liburlencode

C library for urlencode.

> Inspired from https://github.com/dnmfarrell/URI-Encode-C.

## Preparation

+ [make](https://www.gnu.org/software/make/)
+ [Python](https://www.python.org/)

## Build

### Static Library

```shell
$ make
$ make BUILDTYPE=Debug
```

The static library will be at `out/$(BUILDTYPE)/libencode.a`.

### Example

```shell
$ make example
$ make BUILDTYPE=Debug example
```

The example executable binary will be at `out/$(BUILDTYPE)/example`.

## Usage

```cpp
size_t Encode(const char* src, const size_t len, char* dst, bool space_to_plus);
size_t Decode(const char* src, const size_t len, char* dst, bool plus_to_space);
```

**If you're encoding, make sure that the `dst`'s total length be 3 times as `src`'s.** You may refers to `example/main.cc`.
