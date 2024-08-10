#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>

void joystick_init(void) {
    PORTA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
    PORTA.PIN0CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN1CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN2CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN3CTRL |= PORT_OPC_PULLUP_gc;
    PORTA.PIN4CTRL |= PORT_OPC_PULLUP_gc;
}

int joystick_read_button(void) { return (int)0 == (PORTA.IN & PIN0_bm); }

int joystick_read_stick(void) {

    // R    L    D    U    S     bits    N    LABEL
    // --+----+----+----+------+------+----+-------
    // 1    1    1    1    15    1111    0    N
    // 1    1    1    0    14    1110    1    U
    // 0    1    1    0    6     0110    2    UR
    // 0    1    1    1    7     0111    3    R
    // 0    1    0    1    5     0101    4    DR
    // 1    1    0    1    13    1101    5    D
    // 1    0    0    1    9     1001    6    DL
    // 1    0    1    1    11    1011    7    L
    // 1    0    1    0    10    1010    8    UL

    uint8_t stick = PORTA.IN >> 1;
    switch (stick) {
    case 15:
        return 0;
    case 14:
        return 1;
    case 6:
        return 2;
    case 7:
        return 3;
    case 5:
        return 4;
    case 13:
        return 5;
    case 9:
        return 6;
    case 11:
        return 7;
    case 10:
        return 8;
    default:
        return 0;
    }
}
