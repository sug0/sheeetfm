#include "tb_utf8_rune_start.h"

inline int tb_utf8_rune_start(const char *s)
{
    return (*s & 0xc0) != 0x80;
}
