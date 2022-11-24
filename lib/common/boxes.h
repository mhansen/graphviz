#pragma once

#include <assert.h>
#include <common/geom.h>
#include <common/memory.h>
#include <stdlib.h>
#include <string.h>

// a dynamically expanding array of boxes
typedef struct {
  boxf *data;
  size_t size;
  size_t capacity;
} boxes_t;

/** Add an entry to the end of a boxes array.
 *
 * This may expand the array if it is not already large enough to contain the
 * new element.
 *
 * \param boxes Array to append to.
 * \param item Element to append.
 */
static inline void boxes_append(boxes_t *boxes, boxf item) {

  assert(boxes != NULL);

  // do we need to expand the array?
  if (boxes->size == boxes->capacity) {
    size_t c = boxes->capacity == 0 ? 128 : (boxes->capacity * 2);
    boxes->data = grealloc(boxes->data, c * sizeof(boxes->data[0]));
    boxes->capacity = c;
  }

  boxes->data[boxes->size] = item;
  ++boxes->size;
}

/** Remove all entries from a boxes array.
 *
 * \param boxes Array to clear.
 */
static inline void boxes_clear(boxes_t *boxes) {
  assert(boxes != NULL);
  boxes->size = 0;
}

/** Deallocate memory associated with a boxes array.
 *
 * Following a call to this function, the array is reusable as if it had just
 * been initialized.
 *
 * \param boxes Array to deallocate.
 */
static inline void boxes_free(boxes_t *boxes) {
  assert(boxes != NULL);
  free(boxes->data);
  memset(boxes, 0, sizeof(*boxes));
}
