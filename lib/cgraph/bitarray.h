/// \file
/// \brief API for compacted arrays of booleans
///
/// The straightforward way to construct a dynamic array of booleans is to
/// `calloc` an array of `bool` values. However, this wastes a lot of memory.
/// Typically 8 bits per byte, which really adds up for large arrays.
///
/// The following implements an alternative that stores 8 array elements per
/// byte. Using this over the `bool` implementation described above decreases
/// heap pressure and increases locality of reference, at the cost of a few
/// (inexpensive) shifts and masks.
///
/// This is the same optimization C++’s `std::vector<bool>` does.
///
/// This is deliberately implemented header-only so even Graphviz components
/// that do not link against cgraph can use it.

#pragma once

#include <assert.h>
#include <cgraph/likely.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// a compressed array of boolean values
///
/// Note that this complies with the zero-is-initialization idiom. That is, C99
/// zero initializing one of these (`bitarray_t b = {0}`) or `memset`ing one of
/// these to zero gives you a valid zero-length bit array.
typedef struct {
  uint8_t *base; ///< start of the underlying allocated buffer
  size_t size_bits; ///< extent in bits
} bitarray_t;

/// create an array of the given element length
static inline int bitarray_new(bitarray_t *self, size_t size_bits) {
  assert(self != NULL);
  assert(self->size_bits == 0);

  size_t capacity = size_bits / 8 + (size_bits % 8 == 0 ? 0 : 1);
  uint8_t *base = calloc(capacity, sizeof(self->base[0]));
  if (UNLIKELY(base == NULL))
    return ENOMEM;

  self->base = base;
  self->size_bits = size_bits;

  return 0;
}

/// `bitarray_new` for callers who cannot handle failure
static inline bitarray_t bitarray_new_or_exit(size_t size_bits) {

  bitarray_t ba = {0};

  int error = bitarray_new(&ba, size_bits);
  if (UNLIKELY(error != 0)) {
    fprintf(stderr, "out of memory\n");
    exit(EXIT_FAILURE);
  }

  return ba;
}

/// get the value of the given element
static inline bool bitarray_get(bitarray_t self, size_t index) {
  assert(index < self.size_bits && "out of bounds access");

  return (self.base[index / 8] >> (index % 8)) & 1;
}

/// set or clear the value of the given element
static inline void bitarray_set(bitarray_t self, size_t index, bool value) {
  assert(index < self.size_bits && "out of bounds access");

  if (value) {
    self.base[index / 8] |= (uint8_t)(UINT8_C(1) << (index % 8));
  } else {
    self.base[index / 8] ^= (uint8_t)(UINT8_C(1) << (index % 8));
  }
}

/// free underlying resources and leave a bit array empty
static inline void bitarray_reset(bitarray_t *self) {
  assert(self != NULL);

  free(self->base);
  memset(self, 0, sizeof(*self));
}
