#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "zp_fetch_dir.h"

static inline int cmp_str(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

int zp_fetch_dir(struct zipper *z, const char *dir, int hidden)
{
    DIR *dp;
    struct dirent *ep;

    dp = opendir(dir);

    if (!dp)
        return -1;

    while ((ep = readdir(dp))) {
        if (strcmp(ep->d_name, ".")  == 0
        ||  strcmp(ep->d_name, "..") == 0) {
            continue;
        } else if (ep->d_name[0] == '.'
               &&  hidden) {
            continue;
        } else {
            zp_insert_fst(z, strdup(ep->d_name), free, cmp_str);
        }
    }

    closedir(dp);

    return 0;
}
