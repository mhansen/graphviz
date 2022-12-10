// basic unit tester for list.h

#ifdef NDEBUG
#error this is not intended to be compiled with assertions off
#endif

#include <assert.h>
#include <cgraph/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_LIST(ints, int)

// test construction and destruction, with nothing in-between
static void test_create_reset(void) {
  ints_t i = ints_new();
  ints_free(&i);
}

// repeated append
static void test_append(void) {
  ints_t xs = {0};
  assert(ints_is_empty(&xs));

  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
    assert(ints_size(&xs) == i + 1);
  }

  ints_free(&xs);
}

static void test_get(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(ints_get(&xs, i) == (int)i);
  }
  for (size_t i = 9;; --i) {
    assert(ints_get(&xs, i) == (int)i);
    if (i == 0) {
      break;
    }
  }

  ints_free(&xs);
}

static void test_set(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    ints_set(&xs, i, (int)(i + 1));
    assert(ints_get(&xs, i) == (int)i + 1);
  }
  for (size_t i = 9;; --i) {
    ints_set(&xs, i, (int)i - 1);
    assert(ints_get(&xs, i) == (int)i - 1);
    if (i == 0) {
      break;
    }
  }

  ints_free(&xs);
}

static void test_at(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(ints_get(&xs, i) == *ints_at(&xs, i));
  }

  for (size_t i = 0; i < 10; ++i) {
    int *j = ints_at(&xs, i);
    *j = (int)i + 1;
    assert(ints_get(&xs, i) == (int)i + 1);
  }

  ints_free(&xs);
}

static void test_clear_empty(void) {
  ints_t xs = {0};
  ints_clear(&xs);
  assert(ints_is_empty(&xs));

  ints_free(&xs);
}

static void test_clear(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  assert(!ints_is_empty(&xs));
  ints_clear(&xs);
  assert(ints_is_empty(&xs));

  ints_free(&xs);
}

static void test_resize_empty_1(void) {
  ints_t xs = {0};
  ints_resize(&xs, 0, 0);
  assert(ints_is_empty(&xs));
  ints_free(&xs);
}

static void test_resize_empty_2(void) {
  ints_t xs = {0};
  ints_resize(&xs, 10, 42);
  for (size_t i = 0; i < 10; ++i) {
    assert(ints_get(&xs, i) == 42);
  }
  ints_free(&xs);
}

static void test_resize_down(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  ints_resize(&xs, 4, 0);
  assert(ints_size(&xs) == 4);

  ints_free(&xs);
}

static void test_resize_to_0(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  ints_resize(&xs, 0, 0);
  assert(ints_is_empty(&xs));

  ints_free(&xs);
}

static void test_shrink(void) {
  ints_t xs = {0};

  // to test this one we need to access the list internals
  while (ints_size(&xs) == xs.capacity) {
    ints_append(&xs, 42);
  }

  assert(xs.capacity > ints_size(&xs));
  ints_shrink_to_fit(&xs);
  assert(xs.capacity == ints_size(&xs));

  ints_free(&xs);
}

static void test_shrink_empty(void) {
  ints_t xs = {0};
  ints_shrink_to_fit(&xs);
  assert(xs.capacity == 0);
  ints_free(&xs);
}

static void test_free(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  ints_free(&xs);
  assert(ints_size(&xs) == 0);
  assert(xs.capacity == 0);
}

static void test_push(void) {
  ints_t xs = {0};
  ints_t ys = {0};

  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
    ints_push(&ys, (int)i);
    assert(ints_size(&xs) == ints_size(&ys));
    for (size_t j = 0; j <= i; ++j) {
      assert(ints_get(&xs, j) == ints_get(&ys, j));
    }
  }

  ints_free(&xs);
}

static void test_pop(void) {
  ints_t xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    ints_push(&xs, (int)i);
  }
  for (size_t i = 0; i < 10; ++i) {
    assert(ints_size(&xs) == 10 - i);
    int x = ints_pop(&xs);
    assert(x == 10 - (int)i - 1);
  }

  for (size_t i = 0; i < 10; ++i) {
    ints_push(&xs, (int)i);
    (void)ints_pop(&xs);
    assert(ints_is_empty(&xs));
  }

  ints_free(&xs);
}

static void test_large(void) {
  ints_t xs = {0};

  for (int i = 0; i < 5000; ++i) {
    ints_append(&xs, i);
  }
  for (size_t i = 0; i < 5000; ++i) {
    assert(ints_get(&xs, i) == (int)i);
  }

  ints_free(&xs);
}

static void test_attach_detach(void) {
  ints_t xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    ints_append(&xs, (int)i);
  }

  int *ys = ints_detach(&xs);
  assert(ys != NULL);
  assert(ints_is_empty(&xs));

  for (size_t i = 0; i < 10; ++i) {
    assert(ys[i] == (int)i);
  }

  ints_t zs = ints_attach(ys, 10);
  assert(ints_size(&zs) == 10);

  for (size_t i = 0; i < 10; ++i) {
    assert(ints_get(&zs, i) == (int)i);
  }

  ints_free(&zs);
}

DEFINE_LIST(strs, char *)

static void test_dtor(void) {

  // setup a list with a non-trivial destructor
  strs_t xs = {.dtor = (void (*)(char *))free};

  for (size_t i = 0; i < 10; ++i) {
    char *hello = strdup("hello");
    assert(hello != NULL);
    strs_append(&xs, hello);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(strcmp(strs_get(&xs, i), "hello") == 0);
  }

  strs_free(&xs);
}

int main(void) {

#define RUN(t)                                                                 \
  do {                                                                         \
    printf("running test_%s... ", #t);                                         \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    printf("OK\n");                                                            \
  } while (0)

  RUN(create_reset);
  RUN(append);
  RUN(get);
  RUN(set);
  RUN(at);
  RUN(clear_empty);
  RUN(clear);
  RUN(resize_empty_1);
  RUN(resize_empty_2);
  RUN(resize_down);
  RUN(resize_to_0);
  RUN(shrink);
  RUN(shrink_empty);
  RUN(free);
  RUN(push);
  RUN(pop);
  RUN(large);
  RUN(attach_detach);
  RUN(dtor);

#undef RUN

  return EXIT_SUCCESS;
}
