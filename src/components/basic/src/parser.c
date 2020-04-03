#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "arch.h"
#include "error.h"
#include "tokenizer.h"
#include "variables.h"
#include "lines.h"
#include "array.h"
#include "kbhit.h"
#include "io.h"
#include "parser.h"

/*
  line = [number] statement [ : statement ] CR

  statement =
    PRINT expression-list [ ; ]
    | IF relation-expression THEN statement
    | GOTO expression
    | INPUT variable-list
    | LET variable = expression
    | GOSUB expression
    | RETURN
    | FOR numeric_variable '=' numeric_expression TO numeric_expression [ STEP number ] 
    | CLEAR | NEW
    | LIST
    | RUN
    | END
    | DIM variable "(" expression ")"
    | SAVE literal_string
    | LOAD literal_string
    | DELETE literal_string
    | DIR
    | DEF FN(X) = expression

  expression-list = ( string | expression ) [, expression-list]

  variable-list = variable [, variable-list]

  expression = string_expression | numeric_expression
  
  numeric_expression = ["+"|"-"] term {("+"|"-"|"OR") term} .

  term = factor {( "*" | "/" | "AND" ) factor} .

  factor = 
    func "(" expression ")" 
    | number
    | "(" expression ")"
    | variable
    | relation-expression

  relation-expression =
    expression relation-operator expression

  func =
    ABS
    | AND
    | ATN
    | COS
    | EXP
    | INT
    | LOG
    | NOT
    | OR
    | RND
    | SGN
    | SIN
    | SQR
    | TAN

  string = literal_string | string_func "(" string_expression ")"

  literal_string = '"' ... '"'
  
  string_func =
    CHR$
    ...

  string_expression = literal_string | string_variable

  variable = ( numeric_variable | string_variable | indexed_variable )

  numeric_variable = A | B | C ... | X | Y | Z

  string_variable = A$ | B$ | C$ ... | X$ | Y$ | Z$

  indexed_variable = ( numeric_variable | string_variable ) "(" expression ")"

  relation-operator = ( "<" | "<=" | "=" | ">=" | ">" )

*/

#define MAX_LINE tokenizer_string_length

typedef union
{
  function_0 function_0;
  function_1 function_1;
  function_2 function_2;
  function_3 function_3;
  function_4 function_4;
  function_5 function_5;
} basic_function_union;

typedef struct {
  token token;
  uint32_t type : 3;
  uint32_t nr_arguments : 3;
  uint32_t kind_1 : 1;
  uint32_t kind_2 : 1;
  uint32_t kind_3 : 1;
  uint32_t kind_4 : 1;
  uint32_t kind_5 : 1;
  basic_function_union function; 
} basic_function;

static array* basic_tokens = NULL;
static array* basic_functions = NULL;
static token t_keyword_print;
static token t_keyword_print_short;
static token t_keyword_spc;
static token t_keyword_tab;
static token t_keyword_goto;
static token t_keyword_on;
static token t_keyword_if;
static token t_keyword_then;
static token t_keyword_gosub;
static token t_keyword_return;
static token t_keyword_list;
static token t_keyword_clear;
static token t_keyword_new;
static token t_keyword_run;
static token t_keyword_end;
static token t_keyword_stop;
static token t_keyword_for;
static token t_keyword_to;
static token t_keyword_step;
static token t_keyword_next;
static token t_keyword_rem;
static token t_keyword_dim;
static token t_keyword_data;
static token t_keyword_read;
static token t_keyword_restore;
static token t_keyword_cls;
static token t_keyword_load;
static token t_keyword_save;
static token t_keyword_delete;
static token t_keyword_dir;
// static token t_keyword_def;
// static token t_keyword_fn;

static token t_op_or;
static token t_op_and;

uint16_t __line;
static char* __cursor;
static char* __memory;
static char* __stack;
static size_t __memory_size;
static size_t __stack_size;
static size_t __program_size;
static size_t __stack_p;

basic_putchar __putch = putchar;
basic_getchar __getch = getchar;

bool __RUNNING = false;
bool __REPL = true;
bool __STOPPED = false;

typedef enum
{
  data_state_init,
  data_state_find,
  data_state_read
} data_state;

typedef struct
{
  uint16_t line;
  char* cursor;
  char* char_pointer;
  uint32_t state : 2;
} data_pointer;

static data_pointer __data;

typedef union
{
  float numeric;
  char *string;
} expression_value;

typedef enum
{
  expression_type_numeric,
  expression_type_string
} expression_type;

typedef struct
{
  expression_type type;
  expression_value value;    
} expression_result;

typedef enum
{
  stack_frame_type_for,
  stack_frame_type_gosub
} stack_frame_type;

typedef struct
{
  stack_frame_type type;
  char variable_name[tokenizer_variable_length]; 
  float end_value;
  float step;
  size_t line;
  char* cursor; 
} stack_frame_for;

typedef struct
{
  stack_frame_type type;
  size_t line;
  char* cursor;
} stack_frame_gosub;

static int basic_dispatch_function(basic_function* function, basic_type* rv);
static basic_function* find_basic_function_by_type(token sym, basic_function_type type);

static size_t get_vector(size_t* vector, size_t size);
static char* string_term(void);
int str_len(basic_type* str, basic_type* rv);
int str_asc(basic_type* str, basic_type* rv);
int str_val(basic_type* str, basic_type* rv);
int str_str(basic_type* number, basic_type* rv);
int dump(basic_type* rv);
static void move_to_next_statement(void);

typedef enum {
  OP_NOP,
  OP_LT,
  OP_LE,
  OP_EQ,
  OP_GE,
  OP_GT,
  OP_NE
} relop;

static bool string_condition(char *left, char *right, relop op);
static bool numeric_condition(float left, float right, relop op);
static relop get_relop(void);

token sym;
void
get_sym(void)
{
  sym = tokenizer_get_next_token();
  // printf("sym: %d\n", sym);
}

static void
set_line( uint16_t line_number )
{
  __line = line_number;
  char *cursor = lines_get_contents( __line );
  tokenizer_char_pointer( cursor );
}

static float numeric_expression(void);
static char* string_expression(void);

void
expression(expression_result *result)
{
  // printf("expression\n");
  char *string = string_expression();
  if ( NULL != string )
  {
    // Got string, check for relop and apply
    relop op = get_relop();
    if ( op == OP_NOP )
    {
      result->type = expression_type_string;
      result->value.string = string;
    }
    else
    {
      char *string_right = string_expression();
      result->type = expression_type_numeric;
      result->value.numeric = string_condition(string, string_right, op);
      free(string_right);
      free(string);
    }
  }
  else
  {
    // printf("numeric");
    result->type = expression_type_numeric;
    result->value.numeric = numeric_expression();
  }
}

  static void
expression_print(expression_result* expr)
{
  if (expr->type == expression_type_string)
  {
    basic_io_print(expr->value.string);
  }
  else
    if (expr->type == expression_type_numeric)
    {
      char buffer[16];
      float value = expr->value.numeric;
      long ivalue = (int) value;
      if (ivalue == value)
      {
        snprintf(buffer, sizeof(buffer), "%ld", ivalue);
        basic_io_print(buffer);
      }
      else
      {
        snprintf(buffer, sizeof(buffer), "%f", value);
        basic_io_print(buffer);
      }
    }
    else
    {
      error("UNKNOWN EXPRESSION");
    }
}

static int
f_abs(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = fabs(n->value.number);
  return 0;
}

static int
f_rnd(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  if (n->value.number > 0)
  {
    int random = rand();
    rv->value.number = (random * 1.0) / RAND_MAX;
    return 0;
  }

  if (n->value.number < 0)
  {
    srand(n->value.number);
    int random = rand();
    rv->value.number = (random * 1.0) / RAND_MAX;
    return 0;
  }

  time_t now;
  struct tm *tm;
  now = time(NULL);    
  tm = localtime(&now);
  rv->value.number = (tm->tm_sec * 1.0) / 60;
  return 0;
}

static int
f_int(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  int i = (int) n->value.number;  
  rv->value.number = 1.0 * i;
  return 0;
}

static int
f_sqr(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (float) sqrt( (double) n->value.number );
  return 0;
}

static int
f_sgn(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  if (n->value.number < 0)
  {
    rv->value.number = -1.0;
  }
  else
  if (n->value.number > 0)
  {
    rv->value.number = 1.0;
  }
  else
  {
    rv->value.number = 0.0;
  }
  return 0; 
}

static int
f_not(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (float) ( ~ (int) n->value.number );
  return 0;
}

static int
f_sin(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = sinf(n->value.number);
  return 0;
}

static int
f_cos(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = cosf(n->value.number);
  return 0;
}

static int
f_tan(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = tanf(n->value.number);
  return 0;
}

static int
f_log(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = logf(n->value.number);
  return 0;
}

static int
f_exp(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = expf(n->value.number);
  return 0;
}

static int
f_pow(basic_type* x, basic_type* y, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = powf(x->value.number, y->value.number);
  return 0;
}

static int
f_atn(basic_type* n, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = atanf(n->value.number);
  return 0;
}

static float
_or(float a, float b)
{
  return (float) ( (int) a | (int) b );
}

static float
_and(float a, float b)
{
  return (float) ( (int) a & (int) b );
}

static int
str_chr(basic_type* n, basic_type* rv)
{
  rv->kind = kind_string;
  char *chr = malloc(2);
  snprintf(chr, 2, "%c", (int) n->value.number);
  rv->value.string = chr;
  rv->mallocd = true;
  return 0;
}

static int
str_mid(basic_type* str, basic_type* start, basic_type* length, basic_type* rv)
{
  rv->kind = kind_string;
  char* source = str->value.string;
  int _start = (int) start->value.number - 1;
  if(_start>strlen(source)) _start = strlen(source);
  int _length;
  if(length->empty){
    _length = strlen(source) - _start;
  } else {
    _length = (int) length->value.number;
    if(_length+_start>strlen(source)) _length = strlen(source) - _start;
  }
  char* string = strdup(&source[_start]);
  string[_length] = '\0'; 
  rv->value.string = string;
  rv->mallocd = true;
  return 0;
}

static int
str_right(basic_type* str, basic_type* length, basic_type* rv)
{
  rv->kind = kind_string;
  char* source = str->value.string;
  rv->value.string = strdup(&source[strlen(source) - (int) length->value.number]);
  rv->mallocd = true;
  return 0;
}

static int
str_left(basic_type* str, basic_type* length, basic_type* rv)
{
  rv->kind = kind_string;
  rv->value.string = strdup(str->value.string);
  rv->value.string[(int) length->value.number] = '\0';
  rv->mallocd = true;
  return 0;
}

bool
accept(token t)
{
  // printf("accept %ld\n", t);
  if (t == sym) {
    get_sym();
    return true;
  }
  // printf("accept got %ld, expected %ld\n", sym, t);
  return false;
}

static bool
expect(token t)
{
  // printf("expect %ld, have %ld\n", t, sym);
  if (accept(t)) {
    return true;
  }
  error("UNEXPECTED SYMBOL");
  return false;
}

static float factor(void);

static float
numeric_factor(void)
{
  // printf("  factor: %ld\n", sym);

  float number = 0;
  basic_function* bf;
  if ( (bf = find_basic_function_by_type(sym, basic_function_type_numeric)) != NULL ) {
    basic_type rv;
    basic_dispatch_function( bf, &rv);
    if (rv.kind != kind_numeric)
    {
      error("EXPECTED NUMERIC FACTOR");
    }
    number = rv.value.number;
  } else if (sym == T_NUMBER) {
    number = tokenizer_get_number();
    accept(T_NUMBER);
  } else if (sym == T_VARIABLE_NUMBER) {
      char var_name[tokenizer_variable_length];
      tokenizer_get_variable_name(var_name);
      get_sym();
      if (sym == T_LEFT_BANANA)
      {
        // printf("is array\n");  
        //var_name = realloc(var_name, strlen(var_name)+2);
        //var_name = strcat(var_name, "(");
        size_t l = strlen(var_name);
        var_name[l] = '(';
        var_name[l+1] = '\0';
        // printf("name: %s\n", var_name);
        accept(T_LEFT_BANANA);
        size_t vector[5];
        get_vector(vector,5);
        number = variable_array_get_numeric(var_name, vector);
        expect(T_RIGHT_BANANA);
      }
      else
      {
    number = variable_get_numeric(var_name);
    accept(T_VARIABLE_NUMBER);
    }
  } else if (accept(T_LEFT_BANANA)) {
    number = numeric_expression();
    expect(T_RIGHT_BANANA);
  } else {
    error("FACTOR SYNTAX ERROR");
    get_sym();
  }

  relop op = get_relop();
  if (op != OP_NOP)
  {
    float right_number = factor();    
    number = numeric_condition(number, right_number, op);
  }

  return number; 
}

static float
factor(void)
{
  if ( sym == T_STRING || sym == T_VARIABLE_STRING ) {
    char* s1 = string_term(); 
    relop op = get_relop();
    if (op == OP_NOP)
    {
      free(s1);
      error("EXPECTED RELOP");
      return 0;
    }
    char* s2 = string_term();
    float r = string_condition(s1, s2, op);
    free(s2);
    free(s1);
    return r;
  } else {
    return numeric_factor();
  }
}

static float
term(void)
{
  // printf("term\n");

  float f1 = factor();
  while (sym == T_MULTIPLY || sym == T_DIVIDE || sym == t_op_and ) {
    token operator = sym;
    get_sym();
    float f2 = factor();
    switch(operator) {
      case T_MULTIPLY:
        f1 = f1 * f2;
        break;
      case T_DIVIDE:
        f1 = f1 / f2;
        break;
      default:
        if (operator == t_op_and)
        {
          f1 = _and( f1, f2 );
        }
        else
        {
        error("TERM SYNTAX ERROR");    
        }
    }
  }
  return f1;
}

static float
numeric_expression(void)
{
  // printf("numeric expression?\n");

  token operator = T_PLUS;
  if (sym == T_PLUS || sym == T_MINUS) {
    operator = sym;
    get_sym();
  }
  // printf("get term 1\n");
  float t1 = term();
  if (operator == T_MINUS) {
    t1 = -1 * t1;
  }
  while ( sym == T_PLUS || sym == T_MINUS || sym == t_op_or ) {
    operator = sym;
    get_sym();
    float t2 = term();
    switch(operator) {
      case T_PLUS:
        t1 = t1 + t2;
        break;
      case T_MINUS:
        t1 = t1 - t2;
        break;
      default:
        if ( operator == t_op_or )
        {
          t1 = _or( t1, t2 );
        }
        else
        {
        error("EXPRESSION SYNTAX ERROR");
        }
    }
  }
  
  // printf("expression: %f\n", t1);
  
  return t1;
}


static void
ready(void)
{
  if(__REPL){
    puts("READY.");
  }
}

static void
list_out(uint16_t number, char* contents)
{
  char buffer[tokenizer_string_length];
  snprintf(buffer, sizeof(buffer), "%d %s\n", number, contents);
  basic_io_print(buffer);
}

static int
do_list(basic_type* rv)
{
  uint16_t start = 0;
  uint16_t end = 0;

  accept(t_keyword_list);
  if(sym == T_NUMBER)
  {
    start = (uint16_t) tokenizer_get_number();
    accept(T_NUMBER);
    if(sym==T_MINUS)
    {
      accept(T_MINUS);
      if(sym==T_NUMBER)
      {
        end = (uint16_t) tokenizer_get_number();
        accept(T_NUMBER);
      }
    }
  }

  lines_list(start, end, list_out);
  ready();

  return 0;
}

  static int
do_clear(basic_type* rv)
{
  accept(t_keyword_clear);
  lines_clear();
  ready();
  return 0;
}

static char*
string_term(void)
{
  char *string = NULL;
  char var_name[tokenizer_variable_length];

  switch (sym)
  {
    case T_STRING:
      string = strdup(tokenizer_get_string());
      accept(T_STRING);
      break;
    case T_VARIABLE_STRING:
      tokenizer_get_variable_name(var_name);
      get_sym();
      if (sym == T_LEFT_BANANA)
      {
        // printf("is array\n");  
        // var_name = realloc(var_name, strlen(var_name)+2);
        // var_name = strcat(var_name, "(");
        size_t l = strlen(var_name);
        var_name[l] = '(';
        var_name[l+1] = '\0';
        // printf("name: %s\n", var_name);
        accept(T_LEFT_BANANA);
        size_t vector[5];
        get_vector(vector,5);
        string = strdup(variable_array_get_string(var_name, vector));
        expect(T_RIGHT_BANANA);
      }
      else
      {
        string = strdup(variable_get_string(var_name));
        accept(T_VARIABLE_STRING);
      }
      break;
    default:
      {
        basic_function* bf = find_basic_function_by_type(sym, basic_function_type_string);
        if ( bf != NULL )
        {
          basic_type rv;
          basic_dispatch_function( bf, &rv);
          if (rv.kind != kind_string)
          {
            error("EXPECTED STRING TERM");
          }
          string = strdup(rv.value.string);
          if(rv.mallocd==true) free(rv.value.string);
        }
      }
      break;
  }

  return string;
}

static char*
string_expression(void)
{
  char* s1 = string_term();

  while (sym == T_PLUS)
  {
    accept(T_PLUS);
    char *s2 = string_term();
    size_t len = strlen(s1) + strlen(s2) + 1;
    s1 = realloc(s1, len);
    s1 = strcat(s1, s2);
    free(s2);
  }
 
  return s1; 
}


  static int
do_print(basic_type* rv)
{

  accept(t_keyword_print);
  accept(t_keyword_print_short);

  if ( sym == T_EOF || sym == T_COLON ) // Just a print stm
  {
    __putch('\n');
  }
  else
  {
    while (sym != T_EOF && sym != T_COLON)
    {
      basic_function* bf = find_basic_function_by_type(sym, basic_function_type_print);
      if ( bf != NULL )
      {
        basic_type rv;
        basic_dispatch_function( bf, &rv);
      }
      else
      {
        expression_result expr;
        expression(&expr);
        expression_print(&expr);
        if (expr.type == expression_type_string){
           free(expr.value.string);
        }
      }

      if (sym == T_SEMICOLON)
      {
        accept(T_SEMICOLON);
      }
      else
        if (sym == T_COMMA)
        {
          accept(T_COMMA);
          __putch('\t');
        }
        else
        {
          __putch('\n');
        }
    }
  }

  fflush(stdout);

  return 0;
}

static int
do_spc(basic_type* n, basic_type* rv)
{
  for(size_t i=0; i<n->value.number;i++)
  {
    __putch(' ');
  }
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

static int
do_tab(basic_type* n, basic_type* rv)
{
  for(size_t i=0; i<n->value.number;i++)
  {
    __putch(' ');
  }
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_cls(basic_type* rv)
{
  basic_io_print("\033[2J");
  basic_io_print("\033[0;0H");
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}


static int
do_goto(basic_type* rv)
{
  accept(t_keyword_goto);
  
  if (sym != T_NUMBER) {
    error("GOTO EXPECTED NUMBER");
    return 0;
  }

  int line_number = (int) tokenizer_get_number();
  accept(T_NUMBER);

  char* line = lines_get_contents(line_number);
  if (line == NULL) {
    error("GOTO LINE NOT FOUND");
    return 0;
  }

  set_line( line_number );

  return 0;
}

  static size_t
get_list(size_t* list, size_t max_size)
{
  // printf("get list\n");
  size_t size = 0;
  do
  {  
    if (sym == T_COMMA)
    {
      accept(T_COMMA);
    }
    //printf(" sym: %ld\n", sym);
    float n = numeric_expression();
    //printf(" l[%ld] = %d\n", size, (int)n);
    list[size] = n;
    size++;
    if (size>max_size)
    {
      error("LIST MAX SIZE");
      return size;
    }
  } while (sym == T_COMMA);

  return size;
}

  static int
do_on_goto(basic_type* rv)
{
  accept(t_keyword_on);
  
  int index = numeric_expression();

  token what = T_EOF;
  if (sym == t_keyword_goto){
    what = t_keyword_goto;
  } else
  if (sym == t_keyword_gosub){
    what = t_keyword_gosub;
  } else {
    error("ON WITHOUT GOTO OR GOSUB");
    return 0;
  }
  accept(what);

  size_t list[10];
  size_t size = get_list(list, 10);

  if(index>size){
    error("ON OUT OF BOUNDS");
    return 0;
  }

  size_t line_number = list[index-1];  
  if (what == t_keyword_goto){
    //TODO: refactor to helper and use in goto as well
    char* line = lines_get_contents(line_number);
    if (line == NULL) {
      error("LINE NOT FOUND");
    }
    set_line( line_number );
  } else {
    //TODO: refactor to helper and use in gosub as well
    stack_frame_gosub *g;
    if ( __stack_p < sizeof(stack_frame_gosub) )
    {
      error("STACK FULL");
      return 0;
    }

    __stack_p -= sizeof(stack_frame_gosub);
    g = (stack_frame_gosub*) &(__stack[__stack_p]);

    g->type = stack_frame_type_gosub;
    g->line = __line;
    g->cursor = tokenizer_char_pointer(NULL); 
    set_line( line_number );
  }
  return 0;
}


static int
do_gosub(basic_type* rv)
{
  // printf("do_gosub\n");
  accept(t_keyword_gosub);
  
  if (sym != T_NUMBER) {
    error("EXPECTED NUMBER");
    return 0;
  }
  int line_number = (int) tokenizer_get_number();
  // printf("line number: %d\n", line_number);
  accept(T_NUMBER);

  stack_frame_gosub *g;
  if ( __stack_p < sizeof(stack_frame_gosub) )
  {
    error("STACK FULL");
    return 0;
  }

  __stack_p -= sizeof(stack_frame_gosub);
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  g->type = stack_frame_type_gosub;
  g->line = __line;
  g->cursor = tokenizer_char_pointer(NULL); 

  set_line( line_number );

  return 0;
}

static int
do_return(basic_type* rv)
{
  // printf("do_return");
  accept(t_keyword_return);

  stack_frame_gosub *g;
  g = (stack_frame_gosub*) &(__stack[__stack_p]);

  if ( g->type != stack_frame_type_gosub )
  {
    error("EXPECTED GOSUB STACK FRAME");
    return 0;
  }

  __line = g->line;
  tokenizer_char_pointer( g->cursor );

  __stack_p += sizeof(stack_frame_gosub);

  return 0;
}

static int
do_for(basic_type* rv)
{
  // printf("do_for\n");

  accept(t_keyword_for);

  if ( sym != T_VARIABLE_NUMBER ) {
    error("EXPECTED VAR");
    return 0;
  }

  char name[tokenizer_variable_length];
  tokenizer_get_variable_name(name);
  get_sym();
  expect(T_EQUALS);
  float value = numeric_expression();
  variable_set_numeric(name, value);

  expect(t_keyword_to);
  
  float end_value = numeric_expression();

  float step = 1.0;
  if (sym != T_EOF && sym != T_COLON)
  {
    expect(t_keyword_step);
    step = numeric_expression();
  }  

  stack_frame_for *f;
  if ( __stack_p <  sizeof(stack_frame_for) )
  {
    error("STACK FULL");
    return 0;
  }  

  __stack_p -= sizeof(stack_frame_for);
  f = (stack_frame_for*) &(__stack[__stack_p]);
  
  f->type = stack_frame_type_for;
  strncpy(f->variable_name, name, tokenizer_variable_length);
  f->end_value = end_value;
  f->step = step;
  f->line = __line;
  f->cursor = tokenizer_char_pointer(NULL); 

  return 0;
}

static int
do_next(basic_type* rv)
{
  // printf("do_next\n");
  accept(t_keyword_next);

  stack_frame_for *f;
  f = (stack_frame_for*) &(__stack[__stack_p]);

  if ( f->type != stack_frame_type_for )
  {
    error("EXPECTED FOR STACK FRAME");
    return 0;
  }

  if (sym == T_VARIABLE_NUMBER)
  {
    char var_name[tokenizer_variable_length];
    tokenizer_get_variable_name(var_name);
    accept(T_VARIABLE_NUMBER);
    if ( strcmp(var_name, f->variable_name) != 0 )
    {
      char _error[32];
      snprintf(_error, sizeof(_error), "EXPECTED NEXT WITH %s, GOT %s", var_name, f->variable_name);
      error(_error);
      return 0;
    }
  }

  // check end condition 
  float value = variable_get_numeric(f->variable_name) + f->step;
  if ( (f->step > 0 && value > f->end_value) || (f->step < 0 && value < f->end_value) )
  {
      __stack_p += sizeof(stack_frame_for);
      return 0;
  }

  variable_set_numeric(f->variable_name, value); 

  __line = f->line;
  tokenizer_char_pointer( f->cursor );

  return 0;
}

static int
do_end(basic_type* rv)
{
  __RUNNING = false;
  return 0;
}

static int
do_rem(basic_type* rv)
{
  accept(t_keyword_rem);
  set_line(lines_next(__line));
  get_sym();
  return 0;
}

  static size_t
get_vector(size_t* vector, size_t size)
{

  // printf("get_vector(%p, %ld)\n", vector, size);

  for(size_t i=0; i<size; i++)
  {
    vector[i] = 0;
  }
  // printf("get vector\n");
  size_t dimensions = 0;
  while (sym != T_RIGHT_BANANA)
  {
    // printf(" sym: %ld\n", sym);
    // expect(T_NUMBER);
    // float n = tokenizer_get_number();
    float n = numeric_expression();
    // printf(" dim %ld = %d\n", dimensions, (int) n);
    vector[dimensions] = n;
    dimensions++;
    if (dimensions>size)
    {
      error("MAX DIM");
      return dimensions;
    }
    // accept(T_NUMBER);
    if (sym == T_COMMA)
    {
      accept(T_COMMA);
    }
  }
  return dimensions;
}

  static int
do_dim(basic_type* rv)
{
  accept(t_keyword_dim);

  // printf("dim\n");

  while (sym != T_EOF && sym != T_COLON)
  {

    // printf("while\n");

    // get_sym();
    // printf(" s: %ld (%d,%d)\n", sym, T_VARIABLE_NUMBER, T_VARIABLE_STRING);
    if ( sym == T_VARIABLE_NUMBER || sym == T_VARIABLE_STRING )
    {
      variable_type type = (sym == T_VARIABLE_STRING) ? variable_type_string : variable_type_numeric ;
      size_t vector[5];
      char name[tokenizer_variable_length];
      tokenizer_get_variable_name(name);

      size_t l = strlen(name);
      name[l] = '(';
      name[l+1] = '\0';
      //name = realloc(name, name_len + 2);
      //strcat(name, "(");
      // printf(" n: %s\n", name);
      accept(sym);
      expect(T_LEFT_BANANA); 
      size_t dimensions = get_vector(vector, 5);
      expect(T_RIGHT_BANANA);

      // printf("DIM for %s: %ld dimension(s)\n", name, dimensions);
      variable_array_init(name, type, dimensions, vector);
    }

    if (sym == T_COMMA)
    {
      accept(T_COMMA);
    }

  }

  return 0;
}

  static int
do_data(basic_type* rv)
{
  // printf("data\n");
  accept(t_keyword_data);
  move_to_next_statement();
  return 0;
}

  static bool
_data_find(variable_type type, value* value)
{
  tokenizer_init( __data.cursor );
  tokenizer_char_pointer( __data.char_pointer );
  while (__data.cursor)
  {
    get_sym();
    while (sym != T_EOF)
    {
      if (sym == t_keyword_data)
      {
        accept(t_keyword_data);
        if (type == variable_type_string)
        {
          value->string = tokenizer_get_string();
        } else {
          value->number = tokenizer_get_number();
        }  
        __data.state = data_state_read;
        __data.char_pointer = tokenizer_char_pointer(NULL);
        return true;
      }
      get_sym();
    }
    __data.line = lines_next(__data.line);
    __data.cursor = lines_get_contents(__data.line);
    tokenizer_init(__data.cursor);
  }
  return false;
}  

  static bool
_data_read(variable_type type, value* value)
{
  bool rv = false;

  tokenizer_init( __data.cursor );
  tokenizer_char_pointer( __data.char_pointer );
  get_sym();
  if ( sym != T_EOF )
  {
    accept(T_COMMA); // seperated by comma's
    if (type == variable_type_string)
    {
      value->string = tokenizer_get_string();
    }
    else
    {
      value->number = tokenizer_get_number();
    }
    __data.char_pointer = tokenizer_char_pointer(NULL);
    rv = true; 
  }
  else
  {  
    __data.cursor = lines_get_contents(__data.line);
  }
  return rv;
}  

  static bool
_do_data_read(variable_type type, value* value)
{
  char* save_pointer = tokenizer_char_pointer(NULL);
  bool rv = false;

  switch (__data.state)
  {
    case data_state_init:
      __data.line = lines_first();
      __data.cursor = lines_get_contents(__data.line);
      __data.char_pointer = tokenizer_char_pointer(NULL);
      __data.state = data_state_find;
      rv = _data_find(type, value);
      break;

    case data_state_find:  
      rv = _data_find(type, value);
      break;

    case data_state_read:
      {
        bool data_found = _data_read(type, value);
        if (data_found)
        {
          rv = true;
        }
        else
        {
          rv = _data_find(type, value);
        }
      }
  }

  tokenizer_init( __cursor );
  tokenizer_char_pointer(save_pointer);

  return rv;
}

  static int
do_read(basic_type* rv)
{
  bool is_array =  false;
  size_t vector[5];

  accept(t_keyword_read);

  // if not initialized data_pointer, find first data statement
  // while not end of variable list
  //  read data, put in variable
  //  proceed to next data statement
  while (sym != T_EOF && sym != T_COLON)
  {
    if ( sym == T_VARIABLE_NUMBER || sym == T_VARIABLE_STRING )
    {
      variable_type type = (sym == T_VARIABLE_STRING) ? variable_type_string : variable_type_numeric ;
      char name[tokenizer_variable_length];
      tokenizer_get_variable_name(name);
      accept(sym);
      if (sym == T_LEFT_BANANA)
      {
        is_array = true;
        //name = realloc(name, strlen(name)+2);
        //name = strcat(name, "(");
        size_t l = strlen(name);
        name[l] = '(';
        name[l+1] = '\0';
        accept(T_LEFT_BANANA);
        get_vector(vector,5);
        expect(T_RIGHT_BANANA);
      }
      // printf("variable name: %s\n", name);
      value v;
      bool read_ok = _do_data_read(type, &v);
      if ( ! read_ok)
      {
        error("READ WITHOUT DATA");
        return 0;
      }
      if ( type == variable_type_string )
      {
        if (is_array)
        {
         variable_array_set_string(name, v.string, vector);
        }
        else
        {  
          variable_set_string(name, v.string);
        }
      } else {
        if (is_array)
        {
          variable_array_set_numeric(name, v.number, vector);
        }
        else
        {
          variable_set_numeric(name, v.number);
        }
      }
    }
    get_sym();
    accept(T_COMMA);
  }

  return 0;
}

  static int
do_restore(basic_type* rv)
{
  accept(t_keyword_restore);
  // __data.inited = false;
  __data.line = 0;
  __data.cursor = 0;
  __data.state = data_state_init;
  return 0;
}

  static int
is_comment(const char *s)
{
  return s[0] == '#';
}

  static int
is_empty(const char *s)
{
  while (*s != '\0') {
    if (!isspace(*s))
      return 0;
    s++;
  }
  return 1;
}

// Insert '\0' behind first non space character, starting from right.
  static void
_trim(char* s)
{
  char* p = s + strlen(s) - 1; // without the '\0'
  while(p >= s && isspace(*p)){
    --p;
  } 
  *(p+1) = '\0'; 
}  

  static void
_store(char* line)
{
  int number;
  sscanf(line, "%d", &number);

  char* p = line;
  while(isdigit(*p)){
    p++;  
  }
  while(isspace(*p)){
    p++;
  }
  _trim(p);
  printf("%d %s\n", number, p);
  lines_store((uint16_t)number, p);
}  

  static void
_load_cb(char* line, void* context)
{
  if(!(is_empty(line) || is_comment(line))){
    _store(line);
  }
}  

  static int
do_load(basic_type* rv)
{
  accept(t_keyword_load);
  if (sym != T_STRING) {
    error("EXPECTED LITERAL STRING");
    return 0;
  }
  char *filename = tokenizer_get_string();
  accept(T_STRING);
  lines_clear();
  arch_load(filename, _load_cb, NULL);
  ready();

  return 0;
}

typedef struct {
  uint16_t number;
} _save_cb_ctx;

  static uint16_t 
_save_cb(char** line, void* context)
{
  _save_cb_ctx* ctx = (_save_cb_ctx*) context;
  uint16_t number = ctx->number;
  ctx->number = lines_next(number);
   
  *line = lines_get_contents(number);

  return number;
}  

  static int
do_save(basic_type* rv)
{
  accept(t_keyword_save);
  if (sym != T_STRING) {
    error("EXPECTED LITERAL STRING");
    return 0;
  }
  char *filename = tokenizer_get_string();
  accept(T_STRING);
  _save_cb_ctx ctx;
  ctx.number = lines_first();
  arch_save(filename, _save_cb, &ctx);
  ready();

  return 0;
}

  static int
do_delete(basic_type* rv)
{
  accept(t_keyword_delete);
  if (sym != T_STRING) {
    error("EXPECTED LITERAL STRING");
    return 0;
  }
  char *filename = tokenizer_get_string();
  accept(T_STRING);

  arch_delete(filename);
  ready();

  return 0;
}

  static void
_dir_cb(char* name, size_t size, bool label, void* context)
{
  if (label) {
    printf("-- %-13s --\n", name);
  } else {
    printf("> %-8s : %6ld\n", name, (int)size);
  }
}  

  static int
do_dir(basic_type* rv)
{
  accept(t_keyword_dir);
  arch_dir(_dir_cb, NULL);
  ready();

  return 0;
}

//   static int
// do_def_fn(basic_type* rv)
// {
//   accept(t_keyword_def);
// 
//   if(sym != t_keyword_fn){
//     error("EXPECTED FN");
//     return 0;
//   }
// 
//   // Find 'X' between '(', ')'.
//   
//   // Associate 'X' with the location of the expression (string pointer).
//   // When 'evalled', use the string pointer to run the expression valuator.
// 
//   return 0;
// }  

static void parse_line(void);
static bool statement(void);

static int
do_run(basic_type* rv)
{
  __line = lines_first();
  __cursor = lines_get_contents(__line);
  tokenizer_init( __cursor );
  __RUNNING = true;
  __STOPPED = false;
  while (__cursor && __RUNNING)
  {
    // printf("stack available: %" PRIu16 "\n", __stack_p);
    // printf("memory used: %" PRIu16 "\n", lines_memory_used() );
    get_sym();
    if ( sym == T_EOF ) {
      __line = lines_next(__line);
      __cursor = lines_get_contents(__line);
      if ( __cursor == NULL )
      {
        __RUNNING = false;
        break;
      }
      tokenizer_init( __cursor );
    }
    parse_line();
  }
 
  ready(); 

  return 0; 
}

static relop
get_relop(void)
{

  if (sym == T_LESS) {
    accept(T_LESS);
    if (sym == T_EQUALS) {
      accept(T_EQUALS);
      return OP_LE;
    }
    else
    if (sym == T_GREATER)
    {
      accept(T_GREATER);
      return OP_NE;
    }
    return OP_LT;
  }

  if (sym == T_EQUALS) {
    accept(T_EQUALS);
    return OP_EQ;
  }

  if (sym == T_GREATER) {
    accept(T_GREATER);
    if (sym == T_EQUALS) {
      accept(T_EQUALS);
      return OP_GE;
    }
    return OP_GT;
  }

  return OP_NOP;
}

static bool
numeric_condition(float left, float right, relop op)
{

  // printf("numeric condition %f, %f, %d\n", left, right, op);

  switch(op) {
    case OP_NOP:
      error("EXPECTED RELOP");
      break;
    case OP_LT:
      return left < right;
    case OP_LE:
      return left <= right;
    case OP_EQ:
      return left == right;
    case OP_GE:
      return left >= right;
    case OP_GT:
      return left > right;  
    case OP_NE:
      return left != right;
  }

  return false;
}

static bool
string_condition(char *left, char *right, relop op)
{
  int comparison = strcmp(left, right);

  // printf("String condition('%s','%s'): %d\n", left, right, comparison);

  switch(op) {
    case OP_NOP:
      error("EXPECTED RELOP");
      break;
    case OP_LT:
      return comparison < 0;
    case OP_LE:
      return comparison <= 0;
    case OP_EQ:
      return comparison == 0;
    case OP_NE:
      return comparison != 0;
    case OP_GE:
      return comparison >= 0;
    case OP_GT:
      return comparison > 0;  
  }

  return false;
}

static bool
condition(expression_result *left_er, expression_result *right_er, relop op)
{

  if (left_er->type == expression_type_numeric)
  {
    if (right_er->type != expression_type_numeric)
    {
      error("EXPECTED NUMERIC RIGHT HAND TYPE");  
    }
    return numeric_condition(left_er->value.numeric, right_er->value.numeric, op);
  }
  else
  {
    if (right_er->type != expression_type_string)
    {
      error("EXPECTED STRING RIGHT HAND TYPE");
    }
    return string_condition(left_er->value.string, right_er->value.string, op);;
  }
}

static void
move_to_next_statement(void)
{
  while (sym != T_EOF && sym != T_COLON) {
    get_sym();
  }
}

static void
move_to_next_line(void)
{
  set_line(lines_next(__line));
  get_sym();
}

static int
do_if(basic_type* rv)
{

  expression_result left_side, right_side;
  bool result;

  expression(&left_side);

  if ( left_side.type == expression_type_string )
  {
    relop op = get_relop();
    expression(&right_side);
    result = condition(&left_side, &right_side, op);
  }
  else
  {
    result = left_side.value.numeric == 1.0;
  }

  if (sym != t_keyword_then) {
    error("IF WITHOUT THEN");
    return 0;
  } 
  
  if (result) {
    get_sym();

    if ( sym == T_NUMBER )
    {
      float line_number = tokenizer_get_number();
      accept(T_NUMBER);
      set_line(line_number);
    }
    else
    { 
      parse_line();
    }

  } else {
    move_to_next_line();
  }

  return 0;
}

  static int
do_let(basic_type* rv)
{
  // printf("do let\n");

  bool is_array = false;
  size_t vector[5];

  if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
    error("EXPECTED VAR");
    return 0;
  }

  char name[tokenizer_variable_length];
  tokenizer_get_variable_name(name);
  token var_type = sym;
  get_sym();
  if (sym == T_LEFT_BANANA)
  {
    is_array = true;
    // printf("is array\n");  
    // name = realloc(name, strlen(name)+2);
    // name = strcat(name, "(");
    size_t l = strlen(name);
    name[l] = '(';
    name[l+1] = '\0';
    accept(T_LEFT_BANANA);
    get_vector(vector, 5);
    expect(T_RIGHT_BANANA);
  }
  // printf("name: %s\n", name);

  expect(T_EQUALS);
  
  if (var_type == T_VARIABLE_NUMBER) {
    // printf("number\n");
    float value = numeric_expression();
    if (is_array)
    {
      variable_array_set_numeric(name, value, vector);
    }
    else
    {
      // printf("set %s to %f\n", name, value);
      variable_set_numeric(name, value);
    }
  }

  if (var_type == T_VARIABLE_STRING) {
    // printf("string\n");
    char *value = string_expression();
    if (is_array)
    {
      variable_array_set_string(name, value, vector);
    }
    else
    {
      variable_set_string(name, value);
    }
    free(value);
  }

  return 0;
}

  static int
do_input(basic_type* rv)
{
  bool prompt = false;
  expression_result expr;

  if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
    expression(&expr);
    if(sym == T_COMMA || sym == T_SEMICOLON){
      get_sym();
    } else {
      error("UNEXPECTED TOKEN");
    }
    prompt = true;
  }

  if (sym != T_VARIABLE_NUMBER && sym != T_VARIABLE_STRING) {
    error("EXPECTED VAR");
    return 0;
  }

  char name[tokenizer_variable_length];
  token type = sym; 
  if (type == T_VARIABLE_NUMBER) {
    tokenizer_get_variable_name(name);
    accept(T_VARIABLE_NUMBER);
  }

  if (type == T_VARIABLE_STRING) {
    tokenizer_get_variable_name(name);
    accept(T_VARIABLE_STRING);
  }

  if (prompt)
  {
    expression_print(&expr);
    if (expr.type == expression_type_string){
      free(expr.value.string);
    }
  }
  // char* line = readline( prompt ? "" : "?" );

  char line[MAX_LINE];
  basic_io_readline( (prompt ? " " : "? "), line, sizeof(line) ); 

  if (type == T_VARIABLE_NUMBER) {
    char* t;
    float value = strtof(line, &t); 
    variable_set_numeric(name, value);
  }

  if (type == T_VARIABLE_STRING) {
    variable_set_string(name, line);
  }

  return 0;
}

  static int
do_get(basic_type* rv)
{
  if (sym != T_VARIABLE_STRING) {
    error("EXPECTED STRING VAR");
    return 0;
  }

  char name[tokenizer_variable_length];
  tokenizer_get_variable_name(name);

  accept(T_VARIABLE_STRING);

  char c[4] = "";
    int ch = __getch();
    if ( ch == 10 ) {
      ch = 13;
    } 
    snprintf(c, sizeof(c), "%c", ch);
  variable_set_string(name, c);

  return 0;
}

int do_sleep(basic_type* delay, basic_type* rv)
{
  int milliseconds = delay->value.number;

  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  //nanosleep(&ts, NULL);

  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

static void
parse_line(void)
{
  while (sym != T_EOF && sym != T_COLON) {
    bool ok = statement();
    if ( ! ok )
    {
      break;
    }

  }
}

static bool
statement(void)
{
  switch(sym) {
    case T_ERROR:
      error("STATEMENT ERROR");
      break;
    default:
      {
        basic_function* bf = find_basic_function_by_type(sym, basic_function_type_keyword);
        if ( bf != NULL )
        {
          basic_type rv;
          basic_dispatch_function( bf, &rv);
        }
        else
        {
          do_let(NULL);
        }
      }
      break;
  }
  return last_error == NULL;
}


void basic_destroy(void)
{
  variables_destroy();
	tokenizer_free_registered_tokens();
	array_destroy(basic_tokens);
	array_destroy(basic_functions);
	free(__stack);
	free(__memory);
}

void basic_init(size_t memory_size, size_t stack_size)
{

  //printf("stack sizes\n");
 // printf("- available: %" PRIu16 "\n", stack_size);
  //printf("- for:   %" PRIu16 "\n", sizeof(stack_frame_for));
  //printf("- gosub: %" PRIu16 "\n", sizeof(stack_frame_gosub));
  //printf("__\n");

  __memory = malloc(memory_size);
  if(!__memory){
    error("CANNOT ALLOCATE PROGRAM SPACE");
    return;
  }
  __memory_size = memory_size;
  __program_size = __memory_size;

  __stack = malloc(stack_size);
  if(!__stack){
    error("CANNOT ALLOCATE STACK SPACE");
    return;
  }
  __stack_size = stack_size;
  __stack_p = __stack_size;

  __line = 0;
  __data.state = data_state_init;

  basic_tokens = array_new(sizeof(token_entry));
  basic_functions = array_new(sizeof(basic_function));

  tokenizer_setup();

  // Which ones do we keep for xmega if memory pressure too big? (X marks delete for xmega)

  // BASIC keywords
  t_keyword_list = register_function_0(basic_function_type_keyword, "LIST", do_list);
  t_keyword_clear = register_function_0(basic_function_type_keyword, "CLEAR", do_clear); // X
  t_keyword_new = register_function_0(basic_function_type_keyword, "NEW", do_clear);
  t_keyword_goto = register_function_0(basic_function_type_keyword, "GOTO", do_goto);
  t_keyword_on = register_function_0(basic_function_type_keyword, "ON", do_on_goto);
  t_keyword_gosub = register_function_0(basic_function_type_keyword, "GOSUB", do_gosub); 
  t_keyword_return = register_function_0(basic_function_type_keyword, "RETURN", do_return);
  t_keyword_run = register_function_0(basic_function_type_keyword, "RUN", do_run);
  t_keyword_if = register_function_0(basic_function_type_keyword, "IF", do_if);
  t_keyword_then = register_token("THEN");
  t_keyword_for = register_function_0(basic_function_type_keyword, "FOR", do_for);
  t_keyword_to = register_token("TO");
  t_keyword_step = register_token("STEP");
  t_keyword_next = register_function_0(basic_function_type_keyword, "NEXT", do_next);
  t_keyword_end = register_function_0(basic_function_type_keyword, "END", do_end);
  t_keyword_stop = register_function_0(basic_function_type_keyword, "STOP", do_end);
  t_keyword_rem = register_function_0(basic_function_type_keyword, "REM", do_rem);
  t_keyword_dim = register_function_0(basic_function_type_keyword, "DIM", do_dim);
  t_keyword_data = register_function_0(basic_function_type_keyword, "DATA", do_data);
  t_keyword_read = register_function_0(basic_function_type_keyword, "READ", do_read);
  t_keyword_restore = register_function_0(basic_function_type_keyword, "RESTORE", do_restore); 
  t_keyword_load = register_function_0(basic_function_type_keyword, "LOAD", do_load);
  t_keyword_save = register_function_0(basic_function_type_keyword, "SAVE", do_save);
  t_keyword_delete = register_function_0(basic_function_type_keyword, "DELETE", do_delete);
  t_keyword_dir = register_function_0(basic_function_type_keyword, "DIR", do_dir);
  // t_keyword_def = register_function_0(basic_function_type_keyword, "DEF", do_def_fn);
  // t_keyword_fn = register_token("FN");
 
  register_function_0(basic_function_type_keyword, "LET", do_let);
  register_function_0(basic_function_type_keyword, "INPUT", do_input);
  register_function_0(basic_function_type_keyword, "GET", do_get);

  // LOGICAL and BINARY operators
  t_op_or = register_token("OR");
  t_op_and = register_token("AND");

  // Output related
  t_keyword_print = register_function_0(basic_function_type_keyword, "PRINT", do_print);
  t_keyword_print_short = register_function_0(basic_function_type_keyword, "?", do_print);
  t_keyword_spc = register_function_1(basic_function_type_print, "SPC", do_spc, kind_numeric);
  t_keyword_tab = register_function_1(basic_function_type_print, "TAB", do_tab, kind_numeric);
  t_keyword_cls = register_function_0(basic_function_type_keyword, "CLS", do_cls);

  // BASIC functions 
  register_function_1(basic_function_type_numeric, "ABS", f_abs, kind_numeric);
  register_function_1(basic_function_type_numeric, "SIN", f_sin, kind_numeric);
  register_function_1(basic_function_type_numeric, "COS", f_cos, kind_numeric);
  register_function_1(basic_function_type_numeric, "RND", f_rnd, kind_numeric);
  register_function_1(basic_function_type_numeric, "INT", f_int, kind_numeric);
  register_function_1(basic_function_type_numeric, "TAN", f_tan, kind_numeric);
  register_function_1(basic_function_type_numeric, "SQR", f_sqr, kind_numeric);
  register_function_1(basic_function_type_numeric, "SGN", f_sgn, kind_numeric);
  register_function_1(basic_function_type_numeric, "LOG", f_log, kind_numeric);
  register_function_1(basic_function_type_numeric, "EXP", f_exp, kind_numeric);
  register_function_2(basic_function_type_numeric, "POW", f_pow, kind_numeric, kind_numeric);
  register_function_1(basic_function_type_numeric, "ATN", f_atn, kind_numeric);
  register_function_1(basic_function_type_numeric, "NOT", f_not, kind_numeric);

  // BASIC string functions
  register_function_1(basic_function_type_numeric, "LEN", str_len, kind_string);
  register_function_1(basic_function_type_string, "CHR$", str_chr, kind_numeric);
  register_function_3(basic_function_type_string, "MID$", str_mid, kind_string, kind_numeric, kind_numeric);
  register_function_2(basic_function_type_string, "LEFT$", str_left, kind_string, kind_numeric);
  register_function_2(basic_function_type_string, "RIGHT$", str_right, kind_string, kind_numeric);
  register_function_1(basic_function_type_numeric, "ASC", str_asc, kind_string);
  register_function_1(basic_function_type_numeric, "VAL", str_val, kind_string);
  register_function_1(basic_function_type_string, "STR$", str_str, kind_numeric);

  // Special
  register_function_1(basic_function_type_keyword, "SLEEP", do_sleep, kind_numeric);

  // DEBUG
  register_function_0(basic_function_type_keyword, "DUMP", dump);

  lines_init(__memory, __program_size);
  variables_init();
  __data.line = 0;
  __data.cursor = 0;
  __data.state = data_state_init;

  arch_init();
}

  void
basic_register_io(basic_putchar putch, basic_getchar getch)
{
  __putch = putch;
  __getch = getch;
}


void
basic_run(void)
{
  __REPL = false;
  basic_eval("RUN");
}

void
basic_eval(char *line)
{
  if(strlen(line) > 0 && (strlen(line)-1)>tokenizer_string_length)
  {
    error("LINE TOO LONG");
    return;
  }

  char line_string[tokenizer_string_length];
  strncpy(line_string, line, sizeof(line_string));
  _trim(line_string);
  if(is_empty(line_string) || is_comment(line_string)){
    return;
  }

  last_error = NULL;
  tokenizer_init( line_string );
  get_sym();
  if (sym == T_NUMBER ) {
    float line_number = tokenizer_get_number();
    char* line = tokenizer_char_pointer(NULL);
    get_sym();
    if (sym == T_EOF) {
      lines_delete( line_number );
    } else {
      lines_store( line_number, line);
    }
  } else {
    parse_line();
  }
  // printf("stack available: %" PRIu16 "\n", __stack_p);
  // printf("memory available: %" PRIu16 "\n", lines_memory_available() );
}

float evaluate(char *expression_string)
{
  last_error = NULL;
  tokenizer_init( expression_string );
  get_sym();
  float result =  numeric_expression();
  expect(T_EOF);
  return result;
}

void evaluate_print(char *line)
{
  float result = evaluate(line); 
  printf("%s = %f\n", line, result);
}

const char* evaluate_last_error(void)
{
  return last_error;
}

void clear_last_error(void)
{
  last_error = NULL;
}

// - Register functions

static size_t basic_token_id = TOKEN_TYPE_END + 1000; 

token
register_token(char* token_name)
{
  token_entry token;

  token.token = basic_token_id++;
  token.name = token_name;

  // printf("token '%s' = %ld\n", keyword, token.token);
  tokenizer_register_token(&token);

  return token.token;
}

token
register_function_0(basic_function_type type, char* keyword, function_0 function)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 0,
    .function.function_0 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_1(basic_function_type type, char* keyword, function_1 function, kind v1)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 1,
    .kind_1 = v1,
    .function.function_1 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_2(basic_function_type type, char* keyword, function_2 function, kind v1, kind v2)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 2,
    .kind_1 = v1,
    .kind_2 = v2,
    .function.function_2 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_3(basic_function_type type, char* keyword, function_3 function, kind v1, kind v2, kind v3)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 3,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .function.function_3 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_4(basic_function_type type, char* keyword, function_4 function, kind v1, kind v2, kind v3, kind v4)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 4,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .kind_4 = v4,
    .function.function_4 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

token
register_function_5(basic_function_type type, char* keyword, function_5 function, kind v1, kind v2, kind v3, kind v4, kind v5)
{
  token t = register_token(keyword);
  basic_function bf = {
    .token = t,
    .type = type,
    .nr_arguments = 5,
    .kind_1 = v1,
    .kind_2 = v2,
    .kind_3 = v3,
    .kind_4 = v4,
    .kind_5 = v5,
    .function.function_5 = function
  };

  array_push(basic_functions, &bf);

  return t;
}

static basic_function*
find_basic_function_by_type(token sym, basic_function_type type)
{
  // printf("find bf token=%d, type=%d\n", sym, type);
  // printf("#functions: %ld\n", array_size(basic_functions));
  // printf("#size: %ld\n", sizeof(basic_function) * array_size(basic_functions));
  for(size_t i=0; i<array_size(basic_functions); i++)
  {
    basic_function* bf = (basic_function*) array_get(basic_functions, i);
    // printf(">token:%d, type:%d\n", bf->token, bf->type);
    if (bf->type == type && bf->token == sym)
    {
      return bf;
    }
  }
  return NULL;
}

  static void
get_parameter(kind k, basic_type* v)
{
  v->empty = false;
  v->mallocd = false;
  if (k == kind_string)
  {
    char *s = string_expression();
    v->kind = kind_string;
    v->value.string = s;
  }
  else
  {
    float n = numeric_expression();
    v->kind = kind_numeric;
    v->value.number = n;
  }
}

static kind function_kind_i(basic_function *function, int i)
{
  switch(i){
    case 0:
      return function->kind_1;
    case 1:
      return function->kind_2;
    case 2:
      return function->kind_3;
    case 3:
      return function->kind_4;
    case 4:
      return function->kind_5;
    default:
      return kind_numeric;
  }
}

static int
basic_dispatch_function(basic_function* function, basic_type* rv)
{
  if(function->nr_arguments>5){
    error("MAX ARGUMENTS");
    return -1;
  }

  accept(sym);

  if(function->nr_arguments==0 && function->type == basic_function_type_keyword){
    function->function.function_0(rv);
    return 0;
  }

  basic_type v[5];
  expect(T_LEFT_BANANA);
  int i = 0;
  for(; i<function->nr_arguments; i++){
    if(sym!=T_RIGHT_BANANA){
      get_parameter(function_kind_i(function, i), &(v[i]));
      if(i<function->nr_arguments-1){
        if(sym!=T_COMMA){
          // Probably the rest are default parameters...
          break; // break for loop 
        }  
        expect(T_COMMA);
      }
    }
  }
  // loop further, marking empty the variables
  for(; i<function->nr_arguments; i++){
    v[i].empty = true;
    v[i].mallocd = false;
    v[i].kind = kind_numeric;
  }
  expect(T_RIGHT_BANANA);

  rv->mallocd = false;
  switch (function->nr_arguments)
  {
    case 0:
      function->function.function_0(rv);
      break;
    case 1:
      function->function.function_1(&(v[0]), rv);
      break;
    case 2:
      function->function.function_2(&(v[0]), &(v[1]), rv);
      break;
    case 3:
      function->function.function_3(&(v[0]), &(v[1]), &(v[2]), rv);
      break;
    case 4:
      function->function.function_4(&(v[0]), &(v[1]), &(v[2]), &(v[3]), rv);
      break;
    case 5:
      function->function.function_5(&(v[0]), &(v[1]), &(v[2]), &(v[3]), &(v[4]), rv);
      break;
    default:
      return -1;
  }

  for(int i=0; i<function->nr_arguments; i++){
    if(v[i].kind == kind_string){
       free(v[i].value.string);
    }
  }

  return 0;
}

int
str_len(basic_type* str, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (int) strlen(str->value.string);
  return 0;
}

int
str_asc(basic_type* str, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = (int) *(str->value.string);
  return 0;
}

int
str_val(basic_type* str, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = atof(str->value.string);
  return 0;
}

int
str_str(basic_type* number, basic_type* rv)
{
  rv->kind = kind_string;
  rv->value.string = malloc(16);
  snprintf(rv->value.string, 16, "%f", number->value.number);
  rv->mallocd = true;
  return 0;
}

void dump_var(variable* var, void* context)
{
}

int
dump(basic_type* rv)
{
  variables_each(dump_var, NULL);
  return 0;
}
