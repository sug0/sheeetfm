#ifndef ZIPPER_H
#define ZIPPER_H

#include <stdlib.h>

struct zip_cons {
    void *data;
    void (*free_func)(void *);
    struct zip_cons *next;
};

struct zip {
    size_t size;
    struct zip_cons *top;
    struct zip_cons *bot;
};

struct zipper {
    struct zip fst;
    struct zip snd;
};

extern void zp_init(struct zipper *z);
extern void zp_add_fst(struct zipper *z, void *data, void (*free_func)(void *));
extern void zp_add_snd(struct zipper *z, void *data, void (*free_func)(void *));
extern void zp_push_fst(struct zipper *z, void *data, void (*free_func)(void *));
extern void zp_push_snd(struct zipper *z, void *data, void (*free_func)(void *));
extern void zp_insert_fst(struct zipper *z, void *data, void (*free_func)(void *), int (*cmp)(const void *, const void*));
extern void zp_insert_snd(struct zipper *z, void *data, void (*free_func)(void *), int (*cmp)(const void *, const void*));
extern int zp_zip(struct zipper *z);
extern int zp_unzip(struct zipper *z);
extern void zp_zipall(struct zipper *z);
extern void zp_unzipall(struct zipper *z);
extern int zp_empty_fst(struct zipper *z);
extern int zp_empty_snd(struct zipper *z);
extern size_t zp_size_fst(struct zipper *z);
extern size_t zp_size_snd(struct zipper *z);
extern void zp_cleanup(struct zipper *z);
extern void *zp_iter_init(struct zipper *z);
extern void zp_iter_next(void **iter);
extern void *zp_iter_yield(void **iter);

#endif
