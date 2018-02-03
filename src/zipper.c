#include <stdio.h>
#include <string.h>

#include "zipper.h"

static struct zip_cons *zp_cons(void *data, void (*free_func)(void *), struct zip_cons *next)
{
    struct zip_cons *cons = malloc(sizeof(struct zip_cons));

    if (!cons) {
        perror("malloc");
        exit(1);
    }

    cons->data = data;
    cons->free_func = free_func;
    cons->next = next;

    return cons;
}

inline void zp_init(struct zipper *z)
{
    z->fst.size = 0;
    z->fst.top = NULL;
    z->fst.bot = NULL;

    z->snd.size = 0;
    z->snd.top = NULL;
    z->snd.bot = NULL;
}

inline void zp_add_fst(struct zipper *z, void *data, void (*free_func)(void *))
{
    struct zip_cons *cons = zp_cons(data, free_func, NULL);

    if (zp_empty_fst(z)) {
        z->fst.top = z->fst.bot = cons;
    } else {
        z->fst.bot->next = cons;
        z->fst.bot = cons;
    }

    z->fst.size++;
}

inline void zp_add_snd(struct zipper *z, void *data, void (*free_func)(void *))
{
    struct zip_cons *cons = zp_cons(data, free_func, NULL);

    if (zp_empty_snd(z)) {
        z->snd.top = z->snd.bot = cons;
    } else {
        z->snd.bot->next = cons;
        z->snd.bot = cons;
    }

    z->snd.size++;
}

inline void zp_push_fst(struct zipper *z, void *data, void (*free_func)(void *))
{
    z->fst.top = zp_cons(data, free_func, z->fst.top);
    z->fst.size++;
}

inline void zp_push_snd(struct zipper *z, void *data, void (*free_func)(void *))
{
    z->snd.top = zp_cons(data, free_func, z->snd.top);
    z->snd.size++;
}

void zp_insert_fst(struct zipper *z, void *data, void (*free_func)(void *), int (*cmp)(const void *, const void*))
{
    void *temp;
    struct zip_cons *cons = z->fst.top, *new;

    while (cons) {
        if (cmp(data, cons->data) <= 0) {
            temp = cons->data;
            cons->data = data;
            new = zp_cons(temp, free_func, cons->next);
            cons->next = new;

            if (z->fst.bot == cons)
                z->fst.bot = new;

            z->fst.size++;

            return;
        }

        cons = cons->next;
    }

    zp_add_fst(z, data, free_func);
}

void zp_insert_snd(struct zipper *z, void *data, void (*free_func)(void *), int (*cmp)(const void *, const void*))
{
    void *temp;
    struct zip_cons *cons = z->snd.top, *new;

    while (cons) {
        if (cmp(data, cons->data) <= 0) {
            temp = cons->data;
            cons->data = data;
            new = zp_cons(temp, free_func, cons->next);
            cons->next = new;

            if (z->snd.bot == cons)
                z->snd.bot = new;

            z->snd.size++;

            return;
        }

        cons = cons->next;
    }

    zp_add_snd(z, data, free_func);
}

inline int zp_zip(struct zipper *z)
{
    if (zp_empty_fst(z))
        return -1;

    struct zip_cons *head = z->fst.top,
                    *tail = head->next;

    z->fst.top = tail;
    head->next = z->snd.top;
    z->snd.top = head;

    z->fst.size--;
    z->snd.size++;

    return 0;
}

inline int zp_unzip(struct zipper *z)
{
    if (zp_empty_snd(z))
        return -1;

    struct zip_cons *head = z->snd.top,
                    *tail = head->next;

    z->snd.top = tail;
    head->next = z->fst.top;
    z->fst.top = head;

    z->fst.size++;
    z->snd.size--;

    return 0;
}

void zp_zipall(struct zipper *z)
{
    while (zp_zip(z) == 0);
}

void zp_unzipall(struct zipper *z)
{
    while (zp_unzip(z) == 0);
}

inline int zp_empty_fst(struct zipper *z)
{
    return (z->fst.size == 0);
}

inline int zp_empty_snd(struct zipper *z)
{
    return (z->snd.size == 0);
}

inline size_t zp_size_fst(struct zipper *z)
{
    return z->fst.size;
}

inline size_t zp_size_snd(struct zipper *z)
{
    return z->snd.size;
}

static void zp_cleanup_cons(struct zip_cons *cons)
{
    struct zip_cons *temp;

    while (cons) {
        temp = cons->next;

        if (cons->free_func)
            cons->free_func(cons->data);

        free(cons);
        cons = temp;
    }
}

inline void zp_cleanup(struct zipper *z)
{
    if (!zp_empty_fst(z))
        zp_cleanup_cons(z->fst.top);

    if (!zp_empty_snd(z))
        zp_cleanup_cons(z->snd.top);
}

inline void *zp_iter_init(struct zipper *z)
{
    return (void *)z->fst.top;
}

inline void zp_iter_next(void **iter)
{
    *((struct zip_cons **)iter) = (*(struct zip_cons **)iter)->next;
}

inline void *zp_iter_yield(void **iter)
{
    return (*(struct zip_cons **)iter)->data;
}
