#ifndef TB_PUT_H
#define TB_PUT_H

#include <termbox.h>
#include <stdint.h>

extern int tb_put(struct tb_cell *buf, int w, int x, int y, char *src);
extern int tb_putcol(struct tb_cell *buf, int w, int x, int y, char *src, uint16_t fg, uint16_t bg);

#endif
