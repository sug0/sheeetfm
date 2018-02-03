#ifndef TB_INPUT_H
#define TB_INPUT_H

#include <termbox.h>

#define TB_INPUT_BUFSZ  512

enum tb_input_state {
    TB_INPUT_ERROR,
    TB_INPUT_DONE,
    TB_INPUT_TYPING,
    TB_INPUT_DISCARD
};

struct tb_input {
    int cx;
    int ix;
    int iy;
    enum tb_input_state c_state;
    char buf[TB_INPUT_BUFSZ];
    char *ptr;
    char *end;
};

/* init the struct (required!) */
extern void tb_input_init(struct tb_input *in);

/* begin and end the input stream */
extern void tb_input_begin(struct tb_input *in, int x, int y);
extern char *tb_input_done(struct tb_input *in);

/* yield the current buffer state */
extern char *tb_input_yield(struct tb_input *in);

/* call this function on each event between
 * tb_input_begin and tb_input_done */
extern enum tb_input_state tb_input(struct tb_event *e, struct tb_input *in);

/* various utilities */
extern int tb_input_one_more(struct tb_input *in);
extern int tb_input_is_begin(struct tb_input *in);
extern int tb_input_is_typing(struct tb_input *in);

/* set the input state */
extern void tb_input_set(struct tb_input *in, enum tb_input_state c_state);

#endif
