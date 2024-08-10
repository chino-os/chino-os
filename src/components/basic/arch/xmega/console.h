#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_def_char(unsigned char code, char *definition);
void console_plot(int x, int y, unsigned char code);
void console_cursor(int cursor);
void console_cursor_type(int block);
void console_line_overflow(int overflow);
void console_fontbank(int row, int bank);
void console_cls(void);
void console_move_cursor(int x, int y);
void console_invert(int invert);

#endif
