#include "test.h"

#include <dictionary.h>

#include <stdio.h>

void p(char *name, void *value) { printf("n: %s, v: %p\n", name, value); }

void dump(dictionary *d) { dictionary_each(d, p); }

void test_dictionary(void **state) {
    // assert_true(false);

    dictionary *d = dictionary_new();
    assert_non_null(d);

    int v1 = 1000;
    dictionary_put(d, "int", &v1);

    char *v2 = "text";
    dictionary_put(d, "string", &v2);

    assert_true(dictionary_has(d, "int"));
    assert_true(dictionary_has(d, "string"));
    assert_false(dictionary_has(d, "something"));

    assert_non_null(dictionary_del(d, "int"));

    dump(d);

    assert_false(dictionary_has(d, "int"));

    dictionary_destroy(d, NULL);
}
