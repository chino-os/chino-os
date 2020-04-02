#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdlib.h>

static volatile size_t _sound_ticks;

void sound_timerproc(void)
{
  if(_sound_ticks>0){
    _sound_ticks--;
    return;
  }
}

void sound_init(void)
{
  PORTE.OUTCLR = PIN1_bm;
  PORTE.DIRSET = PIN1_bm;
  TCE0.CTRLA = TC_CLKSEL_OFF_gc;
  TCE0.CTRLB = TC0_CCBEN_bm| TC_WGMODE_FRQ_gc;
}

void sound_play(uint16_t f, uint16_t ms)
{
  _sound_ticks = ms / 10;
  if (f>0){
    TCE0.CCABUF = (((uint32_t) F_CPU/f) >> 2) - 1;
    TCE0.CNT = 0; // reset timer/counter
    TCE0.CTRLA = TC_CLKSEL_DIV2_gc; // prescaling
  }
  while(_sound_ticks > 0){
    __asm__ volatile ("nop");
  }
  if(f>0){
    TCE0.CTRLA = TC_CLKSEL_OFF_gc; // timer/counter off
    TCE0.CTRLC = 0; // outputs low
  }
}
