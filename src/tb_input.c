#include <string.h>
#include "tb_input.h"

inline void tb_input_init(struct tb_input *in)
{
    in->c_state = TB_INPUT_DONE;
}

inline void tb_input_begin(struct tb_input *in, int x, int y)
{
    in->cx = x;
    in->ix = x;
    in->iy = y;
    in->c_state = TB_INPUT_DONE;
    in->ptr = (char *)in->buf;
    in->end = (char *)(in->buf + TB_INPUT_BUFSZ);
    memset(in->buf, 0, TB_INPUT_BUFSZ);
    tb_set_cursor(x, y);
    tb_input_set(in, TB_INPUT_TYPING);
}

inline char *tb_input_yield(struct tb_input *in)
{
    tb_set_cursor(in->cx, in->iy);
    return (char *)in->buf;
}

inline char *tb_input_done(struct tb_input *in)
{
    char *yield = tb_input_yield(in);
    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    return yield;
}

inline int tb_input_one_more(struct tb_input *in)
{
    return (in->ptr != in->end);
}

inline int tb_input_is_begin(struct tb_input *in)
{
    return (in->ptr == in->buf);
}

inline void tb_input_set(struct tb_input *in, enum tb_input_state c_state)
{
    in->c_state = c_state;
}

inline int tb_input_is_typing(struct tb_input *in)
{
    return (in->c_state == TB_INPUT_TYPING);
}

enum tb_input_state tb_input(struct tb_event *e, struct tb_input *in)
{
    int len;

    if (in->c_state != TB_INPUT_TYPING)
        return TB_INPUT_ERROR;

    if (e->ch && tb_input_one_more(in)) {
        len = tb_utf8_unicode_to_char(in->ptr, e->ch);
        in->ptr += len;
        in->cx++;
    } else if (e->key == TB_KEY_SPACE && tb_input_one_more(in)) {
        *in->ptr++ = ' ';
        in->cx++;
    } else if ((e->key == TB_KEY_BACKSPACE2
           ||   e->key == TB_KEY_BACKSPACE  ) && !tb_input_is_begin(in)) {
        len = tb_utf8_char_length(*in->ptr);
        in->ptr -= len;
        *in->ptr = 0;
        in->cx--;
    } else if (e->key == TB_KEY_CTRL_U) {
        in->ptr = in->buf;
        memset(in->ptr, 0, TB_INPUT_BUFSZ);
        in->cx = in->ix;
    } else if (e->key == TB_KEY_CTRL_W) {
        while (*in->ptr != ' ' && !tb_input_is_begin(in)) {
            *in->ptr = 0;
            in->ptr--;
            in->cx--;
        }
        *in->ptr = 0;
    } else if (e->key == TB_KEY_ENTER
           ||  e->key == TB_KEY_CTRL_J) {
        in->c_state = TB_INPUT_DONE;
    } else if (e->key == TB_KEY_ESC) {
        in->c_state = TB_INPUT_DISCARD;
    }

    return in->c_state;
}
