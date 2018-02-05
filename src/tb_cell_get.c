#include <termbox.h>
#include "tb_cell_get.h"

inline struct tb_cell *tb_cell_get(struct tb_cell *cells, int w, int x, int y)
{
    return &cells[w*y+x]; 
}
