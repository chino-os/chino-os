#include "test.h"

#include <lines.h>

#include <stdio.h>

static char __memory[1024];
static size_t __memory_size = sizeof(__memory);

int lines_setup(void **state)
{
  lines_init(__memory, __memory_size);
  return 0;
}

int lines_teardown(void **state)
{
  return 0;
}

static void out(uint16_t number, char* contents)
{
  static int calls = 0;
  
  printf("(%d) '%s'\n", number, contents);
  
  calls++;

  // assert_int_equal( number, 10 * calls );
}


void test_lines(void **state)
{
  /*
  assert_true
  (
    lines_insert(10, "XXX")
  );

  line* l10 = lines_get_by_number(10);
  
  assert_int_equal
  (
    l10->number, 10
  );  
  assert_string_equal
  (
    l10->contents, "XXX"
  ); 

  assert_true
  (
    lines_store(20, "YYY")
  );

  line* l20 = lines_get_by_number(20);
  
  assert_int_equal
  (
    l20->number, 20
  );  
  assert_string_equal
  (
    l20->contents, "YYY"
  ); 
 
  lines_list(out); 
  */

  lines_store(10, "XXXX 10");
  lines_store(20, "XXXX 20");
  lines_store(30, "XXXX 30");
  lines_list(out);

  lines_store(15, "XXXX 15");
  lines_list(out);

  lines_store(20, "XXXX 20.2");
  lines_list(out);

  lines_store(20, "XXXX 20");
  lines_list(out);

  lines_store(5, "XXXX 5");
  lines_list(out);

  lines_store(40, "XXXX 40");
  lines_list(out);

  char* l15 = lines_get_contents( 15 );
  assert_string_equal( l15, "XXXX 15" );

  printf("iterate lines:\n");
  uint16_t line = lines_first();
  while (true)
  {
    printf(" %d\n", line);
    line = lines_next( line );
    if ( line == 0 )
    {
      break;
    }
  } 
  printf("-- done\n");

  lines_delete( 11 );
  lines_list(out);

  lines_delete( 5 );
  lines_list(out);

  lines_delete( 40 );
  lines_list(out);

  lines_delete( 15 );
  lines_list(out);

  lines_clear();
  lines_list(out);
}
