#ifndef __SOUND_H__
#define __SOUND_H__

void sound_timerproc(void);
void sound_init(void);
void sound_play(uint16_t freq, uint16_t duration);

#endif
