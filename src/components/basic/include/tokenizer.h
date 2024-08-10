#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <stdlib.h>

#define tokenizer_string_length 64
#define tokenizer_variable_length 8

typedef unsigned int token;
typedef char *token_name;
typedef char *token_keyword;

typedef struct {
    token token;
    token_name name;
} token_entry;

#define add_token(t, k) static token_entry _##t = {t, k};

typedef enum {
    // Standard token types needed by the tokenizer
    T_THE_END,
    T_ERROR,
    T_EOF,
    T_NUMBER,
    T_STRING,
    T_VARIABLE_STRING,
    T_VARIABLE_NUMBER,

    // Some tokens that are standard as well
    T_PLUS,
    T_MINUS,
    T_MULTIPLY,
    T_DIVIDE,
    T_LEFT_BANANA,
    T_RIGHT_BANANA,
    T_COLON,
    T_SEMICOLON,
    T_EQUALS,
    T_LESS,
    T_GREATER,
    T_COMMA,
    TOKEN_TYPE_END
} token_type;

void tokenizer_setup(void);
void tokenizer_init(char *input);
token tokenizer_get_next_token(void);

float tokenizer_get_number(void);
char *tokenizer_get_string(void);
void tokenizer_get_variable_name(char *name);

char *tokenizer_token_name(token);

char *tokenizer_char_pointer(char *set);

void tokenizer_add_tokens(token_entry *tokens);

void tokenizer_register_token(token_entry *entry);
void tokenizer_free_registered_tokens(void);

#endif // __TOKENIZER_H__
