#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

void joystick_init(void);

// 0: neutral
// 1: U
// 2: UR
// 3: R
// 4: DR
// 5: D
// 6: DL
// 7: L
// 8: UL
int joystick_read_stick(void);

// 1: pressed
int joystick_read_button(void);

#endif
