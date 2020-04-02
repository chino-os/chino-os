#include "test.h"

#include <math.h>
#include <parser.h>
#include <stdbool.h>
#include <stdio.h>

extern void test_dictionary(void **state);

extern void test_lines(void **state);
extern int lines_setup(void **state);
extern int lines_teardown(void **state);

static void test_BASIC(void **state)
{
  assert_true( true );
} 

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_BASIC),
        cmocka_unit_test(test_dictionary),
        // cmocka_unit_test(test_lines)
        cmocka_unit_test_setup_teardown(test_lines, lines_setup, lines_teardown)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
