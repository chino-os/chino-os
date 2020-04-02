#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "parser.h"
#include "sound.h"
#include "joystick.h"
#include "console.h"

#include "diskio.h"

#include "error.h"

#include "ff.h"

extern uint16_t __line;
extern bool __STOPPED;

extern token sym;
extern bool accept(token t);
extern void get_sym(void);
static token t_keyword_batch;

// 100 Hz
ISR(TCC0_OVF_vect)
{
  disk_timerproc();
  sound_timerproc();
}

void init_xtal(void)
{
  // Use an external 16Mhz crystal and x 2 PLL to give a clock of 32Mhz

  // Enable the external oscillator
  OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc ;
  OSC.CTRL |= OSC_XOSCEN_bm ;
  while( (OSC.STATUS & OSC_XOSCRDY_bm) == 0 ){} // wait until stable

  // Now configure the PLL to be eXternal OSCillator * 2
  OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | 2 ;
  OSC.CTRL |= OSC_PLLEN_bm ; // enable the PLL...
  while( (OSC.STATUS & OSC_PLLRDY_bm) == 0 ){} // wait until stable

  // And now switch to the PLL as the clocksource
  CCP = CCP_IOREG_gc; // protected write follows
  CLK.CTRL = CLK_SCLKSEL_PLL_gc;
}

void set_usartctrl( USART_t *usart, uint8_t bscale, uint16_t bsel)
{
  usart->BAUDCTRLA = (bsel & USART_BSEL_gm);
  usart->BAUDCTRLB = ((bscale << USART_BSCALE0_bp) & USART_BSCALE_gm) | ((bsel >> 8) & ~USART_BSCALE_gm);
}

void init_uart_bscale_bsel(USART_t *usart, int8_t bscale, int16_t bsel)
{
  usart->CTRLA = 0;
  usart->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
  usart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
  set_usartctrl(usart, bscale, bsel);
}


int _uart_putc(int ch)
{
  while ( ! (USARTE0.STATUS & USART_DREIF_bm) ) {};
  USARTE0.DATA = (char) ch;
  return 1;
}


int uart_putc(int ch)
{
  _uart_putc(ch);
  if (ch == '\n')
  {
    _uart_putc('\r');
  }
  return 1;
}

void uart_puts(unsigned char* s)
{
  while(*s){
    uart_putc(*s);
    s++;
  }
}

int uart_getc(void)
{
  while ( ! (USARTE0.STATUS & USART_RXCIF_bm) ) {};
  char ch = (char) USARTE0.DATA;
  return ch;
}

int uart_fputc(char c, FILE* stream)
{
  uart_putc(c);
  return 0;
}

int keyboard_getc(void)
{
  while ( ! (USARTD0.STATUS & USART_RXCIF_bm) ) {};
  unsigned char ch = (unsigned char) USARTD0.DATA;
  return ch;
}

int uart_fgetc(FILE* stream)
{
  return keyboard_getc();
}

void tellymate_enable_transmit(void)
{
  uart_putc(0x1b);
  for(int i=0; i<4; i++){
    uart_putc('~');
  }
}

int console_charat(int x, int y)
{
  putchar(0x1b);
  putchar('`');
  putchar(' ' + y);
  putchar(' ' + x);
  return uart_getc();
}

FILE uart_stdio = FDEV_SETUP_STREAM(uart_fputc, uart_fgetc, _FDEV_SETUP_RW);

void init_xmega(void)
{
  PORTE.DIRSET = PIN3_bm;
  PORTE.DIRCLR = PIN2_bm;
  // init_uart_bscale_bsel(&USARTE0, -5, 3301); // 19K2 @ 32MHz
  init_uart_bscale_bsel(&USARTE0, -5, 1079); // 57K6 @ 32MHz
  PORTD.DIRSET = PIN3_bm;
  PORTD.DIRCLR = PIN2_bm;
  init_uart_bscale_bsel(&USARTD0, -5, 3301); // 19K2 @ 32MHz
  stdout = stdin = &uart_stdio;

  _delay_ms(500);

  _uart_putc(0x1b);
  _uart_putc('E');
}

  static int
do_joystick(basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = joystick_read_stick(); 
  return 0;
}  

  static int
do_button(basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = joystick_read_button(); 
  return 0;
}  

  static int
do_led(basic_type* status, basic_type* rv)
{
  if(status->value.number>0){
    PORTE.OUTSET = PIN0_bm;
  } else {
    PORTE.OUTCLR = PIN0_bm;
  }  
  rv->kind = kind_numeric;
  rv->value.number = 0; 
  return 0;
}  

  static int
do_sound(basic_type* freq, basic_type* duration, basic_type* rv)
{
  sound_play((uint16_t)freq->value.number, (uint16_t)(1000*duration->value.number));
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_plot(basic_type* x, basic_type* y, basic_type* code, basic_type* rv)
{
  console_plot((int)x->value.number, (int)y->value.number, (unsigned char)code->value.number);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_defchar(basic_type* code, basic_type* definition, basic_type* rv)
{
  console_def_char((unsigned char)code->value.number,definition->value.string);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}  

  static int
do_cursor(basic_type* cursor, basic_type* rv)
{
  console_cursor((int)cursor->value.number);
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_charat(basic_type* x, basic_type* y, basic_type* rv)
{
  unsigned char at = console_charat((int)x->value.number, (int)y->value.number);
  rv->kind = kind_numeric;
  rv->value.number = at;
  return 0;
}  

  static int
do_fontbank(basic_type* p1, basic_type* p2, basic_type* rv)
{
  if(p2->empty){ // Called with 1 parameter
    int bank = (int) p1->value.number;
    for(int row=0; row<25; row++){
      console_fontbank(row, bank);
    }
  } else {  
    int row = (int) p1->value.number;
    int bank = (int) p2->value.number;
    console_fontbank(row, bank);
  }
  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}  

  static int
do_gfx(basic_type* gfx, basic_type* rv)
{
  if(gfx->value.number>0){
    console_cursor(0);
    console_cls();
    console_line_overflow(0);
    for(int row=0; row<25; row++){
      console_fontbank(row, 1);
    }
  } else {
    console_cursor(1);
    console_cls();
    console_line_overflow(1);
    for(int row=0; row<25; row++){
      console_fontbank(row, 0);
    }
  }  
  rv->kind = kind_numeric;
  rv->value.number = 0; 
  return 0;
}  

  static int
do_hplot(basic_type* _x, basic_type* _y, basic_type* _set, basic_type* rv)
{
  unsigned char x = (unsigned char)_x->value.number;
  unsigned char y = (unsigned char)_y->value.number;
  unsigned char set = (unsigned char)_set->value.number;

  unsigned char row ;
  unsigned char col ;
  unsigned char screen ;

  static unsigned char s_row = -1;
  static unsigned char s_col = -1;
  static unsigned char s_s = 0 ;

  row = y / 3 ; // 3 pixels down per character
  col = x / 2 ; // 2 pixels across per character
      
  if ((row == s_row) & (col == s_col))
  {
    screen = s_s;
  }
  else
  {
    screen = console_charat(col, row);
  }

  if (screen >= 0)
  {
    s_row = row;
    s_col = col;

    unsigned char s = screen;
    s = s & 0b00111111 ;

    unsigned char yy = row * 3;
    unsigned char xx = col * 2;

    unsigned char shift = 0;
    if (x > xx) shift += 1;
    if (y > yy) shift += 2;
    if (y > (yy+1)) shift += 2;
    unsigned char bitvalue = 1 << shift ;

    if (set)
    {
      s = s | bitvalue ; // set the bit
    }
    else
    {
      s = s & ~bitvalue ; // clear the bit
    }
    s = 0b11000000 | s  ;

    s_s = s ;

    if (s != screen)
    {
      console_plot(col, row, s);
    }
  }

  rv->kind = kind_numeric;
  rv->value.number = 0;
  return 0;
}

  static int
do_overflow(basic_type* overflow, basic_type* rv)
{
  if(overflow->value.number>0){
    console_line_overflow(1);
  } else {
    console_line_overflow(0);
  }  
  rv->kind = kind_numeric;
  rv->value.number = 0; 
  return 0;
}  

  static int
do_invert(basic_type* invert, basic_type* rv)
{
  if(invert->value.number>0){
    console_invert(1);
  } else {
    console_invert(0);
  }  
  rv->kind = kind_numeric;
  rv->value.number = 0; 
  return 0;
}  

  static int
do_at(basic_type* x, basic_type* y, basic_type* rv)
{
  rv->kind = kind_numeric;
  rv->value.number = 10;
  console_move_cursor((int)x->value.number, (int)y->value.number);
  return 0;
}

  static void
batch(char *filename)
{
  FIL fil;
  FRESULT fr;
  char line[tokenizer_string_length];
  fr = f_open(&fil, filename, FA_READ);
  if(fr) return;
  while (f_gets(line, sizeof line, &fil)){
    basic_eval(line);
  }
  f_close(&fil);
}  

  static int
do_batch(basic_type* rv)
{
  char filename[8+1+3+1];
  accept(t_keyword_batch);
  if (sym != T_STRING) {
    error("EXPECTED LITERAL STRING");
    return 0;
  }
  char *name = tokenizer_get_string();
  accept(T_STRING);
  strncpy(filename,name,9);
  strcat(filename,".BAS");
  batch(filename);
  return 0;
}

int main(int argc, char *argv[])
{
  char input[64];

  init_xtal();
  init_xmega();
  sound_init();
  joystick_init();

  PORTE.DIRSET = PIN0_bm; // LED
  PORTE.OUTSET = PIN0_bm; //  on

  // timer
  TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
  TCC0.CTRLA = TC_CLKSEL_DIV256_gc; 
  TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
  TCC0.PER = 1249; // t = N * (PER+1) / f 
  PMIC.CTRL |= PMIC_LOLVLEN_bm;
  sei();

  tellymate_enable_transmit();
  console_cursor(1);
  console_cursor_type(1);
  console_line_overflow(1);
  puts("  (\\/)");
  puts(" ( ..)");
  puts("C(\")(\")");
  puts("");
  puts("~BASIC-1~");
  puts("(c) 2015-2017 JVdB");
  puts("");

  for(uint16_t i=0; i<3; i++){
     sound_play(1000, 50);
     sound_play(0, 50);
  }
  sound_play(1000, 100);

  basic_register_io(uart_putc, keyboard_getc);
  basic_init(3072, 256);
  
  register_function_2(basic_function_type_keyword, "SOUND", do_sound, kind_numeric, kind_numeric);
  register_function_1(basic_function_type_keyword, "LED", do_led, kind_numeric);
  register_function_0(basic_function_type_numeric, "JOYSTICK", do_joystick);
  register_function_0(basic_function_type_numeric, "BUTTON", do_button);
  register_function_3(basic_function_type_keyword, "PLOT", do_plot, kind_numeric, kind_numeric, kind_numeric);
  register_function_2(basic_function_type_keyword, "DEFCHAR", do_defchar, kind_numeric, kind_string);
  register_function_1(basic_function_type_keyword, "CURSOR", do_cursor, kind_numeric);
  register_function_2(basic_function_type_numeric, "CHARAT", do_charat, kind_numeric, kind_numeric);
  register_function_2(basic_function_type_keyword, "FONTBANK", do_fontbank, kind_numeric, kind_numeric);
  register_function_1(basic_function_type_keyword, "GFX", do_gfx, kind_numeric);
  register_function_3(basic_function_type_keyword, "HPLOT", do_hplot, kind_numeric, kind_numeric, kind_numeric);
  register_function_1(basic_function_type_keyword, "OVERFLOW", do_overflow, kind_numeric);
  register_function_1(basic_function_type_keyword, "INVERT", do_invert, kind_numeric);
  register_function_2(basic_function_type_keyword, "AT", do_at, kind_numeric, kind_numeric);
  t_keyword_batch = register_function_0(basic_function_type_keyword, "BATCH", do_batch);

  batch("AUTORUN.BAS");

  while(1)
  {
    basic_io_readline("", input, sizeof(input)); 
    basic_eval(input);
    if (evaluate_last_error()) {
      printf("ERR LINE %d: %s\n", __line, evaluate_last_error());
      clear_last_error();
    }
    if(__STOPPED){
      console_cursor(1);
      __STOPPED = false;
    }
  }
  
  return EXIT_SUCCESS;
}
