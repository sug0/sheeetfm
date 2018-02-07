#include <string.h>
#include "tb_cell_get.h"

int tb_put(struct tb_cell *buf, int w, int x, int y, char *src)
{
    int put = 0;
    uint32_t ch;
    struct tb_cell *cell;

    while (x < w && *src) {
        cell = tb_cell_get(buf, w, x, y);
        src += tb_utf8_char_to_unicode(&ch, src);
        cell->ch = ch;
        x++;
        put++;
    }

    return put;
}

int tb_putcol(struct tb_cell *buf, int w, int x, int y, char *src, uint16_t fg, uint16_t bg)
{
    int put = 0;
    uint32_t ch;
    struct tb_cell *cell;

    while (x < w && *src) {
        cell = tb_cell_get(buf, w, x, y);
        src += tb_utf8_char_to_unicode(&ch, src);
        cell->ch = ch;
        cell->fg = fg;
        cell->bg = bg;
        x++;
        put++;
    }

    return put;
}
