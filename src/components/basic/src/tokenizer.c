#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>

#include "arch.h"
#include "tokenizer.h"
#include "hexdump.h"
#include "array.h"

static array* token_array = NULL;

add_token( T_ERROR, NULL );
add_token( T_EOF, NULL );
add_token( T_NUMBER, NULL );
add_token( T_STRING, NULL );
add_token( T_VARIABLE_STRING, NULL );
add_token( T_VARIABLE_NUMBER, NULL );
add_token( T_PLUS, "+" );
add_token( T_MINUS, "-" );
add_token( T_MULTIPLY, "*" );
add_token( T_DIVIDE, "/" );
add_token( T_LEFT_BANANA, "(" );
add_token( T_RIGHT_BANANA, ")" );
add_token( T_COLON, ":" );
add_token( T_SEMICOLON, ";" );
add_token( T_EQUALS, "=" );
add_token( T_LESS, "<" );
add_token( T_GREATER, ">" );
add_token( T_COMMA, "," );

char *tokenizer_line = NULL;
char *tokenizer_p = NULL;
char *tokenizer_next_p = NULL;

token tokenizer_actual_token;
float tokenizer_actual_number;
char tokenizer_actual_char;
char tokenizer_actual_string[tokenizer_string_length];
char tokenizer_actual_variable[tokenizer_variable_length];

void tokenizer_setup(void)
{
  token_array = array_new(sizeof(token_entry));
  tokenizer_register_token( &_T_ERROR);
  tokenizer_register_token( &_T_EOF);
  tokenizer_register_token( &_T_NUMBER);
  tokenizer_register_token( &_T_STRING);
  tokenizer_register_token( &_T_VARIABLE_STRING);
  tokenizer_register_token( &_T_VARIABLE_NUMBER);
  tokenizer_register_token( &_T_PLUS);
  tokenizer_register_token( &_T_MINUS);
  tokenizer_register_token( &_T_MULTIPLY);
  tokenizer_register_token( &_T_DIVIDE);
  tokenizer_register_token( &_T_LEFT_BANANA);
  tokenizer_register_token( &_T_RIGHT_BANANA);
  tokenizer_register_token( &_T_COLON);
  tokenizer_register_token( &_T_SEMICOLON);
  tokenizer_register_token( &_T_EQUALS);
  tokenizer_register_token( &_T_LESS);
  tokenizer_register_token( &_T_GREATER);
  tokenizer_register_token( &_T_COMMA);
}

void tokenizer_init(char *input)
{
  tokenizer_line = input;
  tokenizer_p = tokenizer_next_p = tokenizer_line;
}

char* tokenizer_char_pointer(char* set)
{
  if ( set != NULL )
  {
    tokenizer_p = set; 
    return NULL;
  }

  // Skip white space
  while ( *tokenizer_p && isspace(*tokenizer_p) ) {
    tokenizer_p++;
  } 
  return tokenizer_p;
}

static bool
isvarchar(char c)
{

  if (c >= 'A' && c <= 'Z') {
    return true;
  }

  if ( c == '$' ) {
    return true;
  }

  if (c >= '0' && c <= '9'){
    return true;
  }

  return false;
}

token _find_registered(void)
{

  // printf("_find registered\n");

  for(size_t i=0; i<array_size(token_array); i++)
  {
    // token_entry* entry = registered_tokens_ptr + i * sizeof(token_entry);
    token_entry* entry = (token_entry*) array_get(token_array, i);
  
    // printf("t: '%s'\n", entry->name);

    if ( entry->name == NULL ) continue;

    // printf("look for: '%s', in '%s'\n", entry->name, tokenizer_p);

    if (strncmp(tokenizer_p, entry->name, strlen(entry->name)) == 0) {
       // printf("found '%s'\n", entry->name);
       tokenizer_next_p = tokenizer_p + strlen(entry->name);
       tokenizer_p = tokenizer_next_p;
       return entry->token;
    }
  }
  return T_THE_END;
}

token tokenizer_get_next_token(void)
{
  if ( ! *tokenizer_p ) {
    return T_EOF;
  } 

  // Skip white space
  while ( *tokenizer_p && isspace(*tokenizer_p) ) {
    tokenizer_p++;
  } 

  // Check for number
  if (isdigit(*tokenizer_p) || *tokenizer_p == '.') {
    // puts("read a number");
    tokenizer_next_p = tokenizer_p;
    size_t l=0;
    while (*tokenizer_next_p && ( isdigit(*tokenizer_next_p) || *tokenizer_next_p == '.') ) {
      l++;
      tokenizer_next_p++;
    }
    char *number = alloca(l + 1);
    memset(number, 0, l+1);
    // strlcpy(number, tokenizer_p, sizeof(number) );
    strncpy(number, tokenizer_p, l );
    number[l] = '\0';
    tokenizer_p = tokenizer_next_p;
    float f;
    // printf("[%s]\n", number);
    sscanf(number, "%f", &f);
    // printf("Got float: '%f'\n", f);
    tokenizer_actual_number = f;
    return T_NUMBER;
  }

  // Check for string
  if ( '"' == *tokenizer_p ) {
    // puts("read string");
    tokenizer_p++; // skip "
    tokenizer_next_p = tokenizer_p;
    size_t l=0;
    while(*tokenizer_next_p && '"' != *tokenizer_next_p) {
      l++;
      tokenizer_next_p++;
    }

    if (*tokenizer_next_p) {
      tokenizer_next_p++; // skip trailing "
    }

    if(l>80){
      return T_ERROR;
    }

    memcpy(tokenizer_actual_string, tokenizer_p, l);
    tokenizer_actual_string[l] = '\0';
   
    tokenizer_p = tokenizer_next_p;

    return T_STRING; 
  }

  token t = _find_registered();
  if ( t != T_THE_END )
  {
    return t;
  }

  // Check for variable
  tokenizer_next_p = tokenizer_p;
  size_t len = 0;
  while(*tokenizer_next_p && isvarchar(*tokenizer_next_p)) {
    len++;
    tokenizer_next_p++;
  }

  if(len>tokenizer_variable_length){
    return T_ERROR;
  }

  if (len > 0) {
    memcpy(tokenizer_actual_variable, tokenizer_p, len);
    tokenizer_actual_variable[len] = '\0';
    tokenizer_p = tokenizer_next_p;
    if (tokenizer_actual_variable[len-1] == '$') {
      return T_VARIABLE_STRING;
    }
    return T_VARIABLE_NUMBER;
  }

  return T_ERROR; 
}

float tokenizer_get_number(void)
{
  return tokenizer_actual_number;
}

char *tokenizer_get_string(void)
{
  return tokenizer_actual_string;
}

void tokenizer_get_variable_name(char *name)
{
  strncpy(name, tokenizer_actual_variable, sizeof(tokenizer_actual_variable));
}

  void
tokenizer_register_token( token_entry* entry )
{
  /*
  // Create space for token_entry
  registered_tokens_count++;
  registered_tokens_ptr = realloc((char*)registered_tokens_ptr, sizeof(token_entry) * registered_tokens_count);
  // Copy data
  //token_entry* new = registered_tokens_ptr + sizeof(token_entry) * ( registered_tokens_count - 1 );
  token_entry* new = registered_tokens_ptr + registered_tokens_count - 1;
  // printf("n:%p, e:%p, i:%ld, s:%ld\n", new, entry, registered_tokens_count, sizeof(token_entry));
  memcpy(new, entry, sizeof(token_entry));
  // hexdump("tokens", new, sizeof(token_entry) * registered_tokens_count );
  */
  array_push(token_array, entry);
}

  void
tokenizer_free_registered_tokens(void)
{
  array_destroy(token_array);
  /*
  registered_tokens_count = 0;
  free(registered_tokens_ptr);
  registered_tokens_ptr = NULL;
  */
}

