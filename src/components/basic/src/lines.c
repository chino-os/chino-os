// -- Lines BASIC program storage

// TODO:
//  v delete line
//  v wipe
//  v get_line
//  . clean up code
//  v integrate with parser

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hexdump.h"
#include "lines.h"

static char* __memory;
static char* __memory_end;
static size_t __memory_size;

  static line*
_next(line* l)
{
  char* p = (char*) l;
  p += sizeof(line) - 1 + l->length;
  return (line*) p;
}

  static bool
_is_end(line* l)
{
  return l && l->number == 0 && l->length == 0;
}

  static line*
_find_end(line* l)
{
  line* n = l;
  while ( ! _is_end( n ) )
  {
    n = _next( n );
  }
  return n;  
}

  void
lines_init(char *memory, size_t memory_size)
{
  __memory = memory;
  __memory_end = memory;
  __memory_size = memory_size;
 
  // Signal end 
  line* l = (line*) __memory;
  l->number = 0;
  l->length = 0;
}

  size_t
lines_memory_used(void)
{
  char *p = __memory;
  line* start = (line*) p;
  line* end = _find_end(start);
  end = _next(end);
      
  char* m_start = (char*) start;
  char* m_end = (char*) end;
   
   return m_end - m_start;
}

  size_t
lines_memory_available(void)
{
  return __memory_size - lines_memory_used();
}

  bool
lines_store(uint16_t number, char* contents)
{
  // support insert


  char *p = __memory;
  line* l = (line*) p;
  while ( ! _is_end( l ) )
  {
    line* next = _next( l );

    // Find line that is to be insert after. That line has a line number < insert and the next line has a >
    if ( l->number < number && next->number > number )
    {
      // We need to insert
      // printf("insert %d\n", number);
      
      // The address of the insert is the same as the next line
      line* insert = next;
     
      // But we need to move the memory block holding the rest to the right.
      line* end = _find_end( insert );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // We have the end*,  calculate size to move
      char* m_src = (char*) insert;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;

      // Calculate offset to move 
      size_t insert_size = sizeof(line) - 1 + strlen(contents) + 1;
      char* m_dst = m_src + insert_size;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the insert
      insert->number = number;
      insert->length = strlen(contents) + 1;
      strcpy( &(insert->contents), contents );

      // hexdump( "insert", __memory, 256 );
      
      return true;
    }
    // Replace
    if ( l->number == number )
    {
      // printf("replace %d\n", number);
      
      // We need to shift the memory to the new offset determined by the size of the line to be inserted
      
      line* end = _find_end( l );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // Calculate size of bloack
      char* m_src = (char*) next;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;

      // Calculate offset to move 
      size_t replace_size = sizeof(line) - 1 + strlen(contents) + 1;
      size_t actual_size = sizeof(line) - 1 + strlen(&(l->contents)) + 1;
      int offset = replace_size - actual_size ;
      char* m_dst = m_src + offset;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the replace
      l->length = strlen(contents) + 1;
      strcpy( &(l->contents), contents );

      // hexdump( "replace", __memory, 256 );
      
      return true;
    }
    // Prepend
    if ( l->number > number )
    {
      // printf("prepend %d\n", number);
      
      // The address of the insert is the same as the actual line
      line* insert = l;
     
      // But we need to move the memory block holding the rest to the right.
      line* end = _find_end( insert );
      end = _next(end); // Move to next empty slot (we keep the sentinel in the copy)
      
      // We have the end*,  calculate size to move
      char* m_src = (char*) insert;
      char* m_end = (char*) end;
      size_t m_size = m_end - m_src;
     
      // Calculate offset to move 
      size_t insert_size = sizeof(line) - 1 + strlen(contents) + 1;
      char* m_dst = m_src + insert_size;
      
      // Move the memory block
      memmove( m_dst, m_src, m_size );

      // Set the data of the insert
      insert->number = number;
      insert->length = strlen(contents) + 1;
      strcpy( &(insert->contents), contents );

      // hexdump( "prepend", __memory, 256 );
 
      return true; 
    }

    l = next;
  }
  
  l->number = number;
  l->length = strlen(contents) + 1; // Length is offset to next line
  strcpy( &(l->contents), contents );
 
  line* end = _next( l );
  end->number = 0;
  end->length = 0;
 
  // hexdump( "append", __memory, 256 );

  return true;
}

  bool
lines_delete(uint16_t number)
{
  // printf("delete line %d\n", number);

  // find the line
  line* l = (line*) __memory;
  while( ! _is_end( l ) && l->number != number)
  {
    l = _next( l );
  }

  if ( _is_end( l ) )
  {
    // printf("line %d not found\n", number);
    return false;
  }

  // l is the line to delete
  // check if this is the last line
  line* next = _next(l);
  if ( _is_end( next ) )
  {
    // printf("delete last line\n");
    memset( l, 0x00, sizeof(line) - 1 + strlen(&(l->contents)) + 1 );
    l->number = 0;
    l->length = 0;
    strcpy( &(l->contents), "" );
  }
  else
  {
    // printf("delete not last line\n");
    char* dst = (char*) l;
    char* src = (char*) next;
 
    line* lend = _find_end( next );
    lend = _next(lend); // Move to next empty slot (we keep the sentinel in the copy)
    char* end = (char*) lend;
    size_t size = (char*) end - src;
    memmove( dst, src, size );

    size_t rest = src - dst;
    memset( end - rest, 0x00, rest );
  }

  // hexdump( "delete", __memory, 256 );
  
  return true;
}

  static bool
_in_range(uint16_t i, uint16_t low, uint16_t high)
{
  if(low==0 && high==0)
  {
    return true;
  }
  if(low==0 && i<=high)
  {
    return true;
  }
  if(high==0 && i>=low)
  {
    return true;
  }
  if(i>= low && i<=high)
  {
    return true;
  }
  return false;
} 

  void
lines_list(uint16_t start, uint16_t end, lines_list_cb out)
{
  char *p = __memory;
  
  line* l = (line*) p;
  while( ! _is_end( l ) )
  {
    if(_in_range(l->number, start, end))
    {
      out(l->number, &(l->contents) );
    }
    l = _next( l );
  }
}

  void
lines_clear(void)
{
  char* end = (char*) _next( _find_end( (line*) __memory ) );
  memset( __memory, 0x00, end - __memory );
  line* l = (line*) __memory;
  l->number = 0;
  l->length = 0;
  // hexdump( "clear", __memory, 256 );
}

  char*
lines_get_contents(uint16_t number)
{
  line* l = (line*) __memory;
  while( ! _is_end( l ) && l->number != number)
  {
    l = _next( l );
  }

  if ( _is_end( l ) )
  {
    return NULL;
  }

  return &(l->contents);
}

  uint16_t
lines_first(void)
{
  line* l = (line*) __memory;
  return l->number;
}

  uint16_t
lines_next(uint16_t number)
{
  line* l = (line*) __memory;
  while( ! _is_end( l ) && l->number <= number)
  {
    l = _next( l );
  }

  if ( number == l->number )
  {
    return 0;
  }

  return l->number;
}
