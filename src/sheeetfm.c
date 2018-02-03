#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <termbox.h>
#include "utils.h"

#define NaN(F)                                      \
       (((((unsigned char *)&F)[3] & 0x7f) == 0x7f) \
    &&   (((unsigned char *)&F)[2] & 0x80))

char *argv0;

static inline void tb_change_prompt(struct tb_cell *buf)
{
    tb_cell_get(buf, tb_width(), 3, tb_height() - 1)->ch = ':';
}

static inline char *zp_selected_path(struct zipper *z)
{
    return (zp_empty_fst(z)) ? NULL : (char *)z->fst.top->data;
}

static int is_dir(char *path)
{
    struct stat st;
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

static void tb_putfile(struct tb_cell *buf, int w, int x, int y, char *path)
{
    int put;
    char link[4096] = {0};
    struct stat st;

    if (lstat(path, &st) != 0) {
        tb_put(buf, w, x, y, path);
        return;
    }

    /* ugly train of ifs and elses */
    if (S_ISREG(st.st_mode)) {
        if (st.st_mode & S_IXUSR)
            tb_putcol(buf, w, x, y, path, TB_BOLD|TB_GREEN, TB_DEFAULT);
        else
            tb_put(buf, w, x, y, path);
    } else if (S_ISDIR(st.st_mode)) {
        tb_putcol(buf, w, x, y, path, TB_BOLD|TB_BLUE, TB_DEFAULT);
    } else if (S_ISFIFO(st.st_mode)) {
        tb_putcol(buf, w, x, y, path, TB_YELLOW, TB_DEFAULT);
    } else if (S_ISSOCK(st.st_mode)) {
        tb_putcol(buf, w, x, y, path, TB_MAGENTA, TB_DEFAULT);
    } else if (S_ISBLK(st.st_mode)) {
        tb_putcol(buf, w, x, y, path, TB_RED, TB_DEFAULT);
    } else if (S_ISCHR(st.st_mode)) {
        tb_putcol(buf, w, x, y, path, TB_BOLD|TB_YELLOW, TB_DEFAULT);
    } else if (S_ISLNK(st.st_mode)) {
        put = tb_putcol(buf, w, x, y, path, TB_BOLD|TB_CYAN, TB_DEFAULT);
        readlink(path, link, 4096);
        tb_put(buf, w, x + put + 1, y, "➤");
        tb_put(buf, w, x + put + 3, y, link);
    } else {
        tb_put(buf, w, x, y, path);
    }
}

static inline float zp_percent(struct zipper *z)
{
	float val = z->snd.size / (float)(z->fst.size + z->snd.size) * 100;
    return NaN(val) ? 0.0 : val;
}

static void tb_draw_statusbar(struct zipper *z, struct tb_cell *cells, int w, int h)
{
    char cwd[4096], perc[8];

    snprintf(perc, 8, "%.2f%%", zp_percent(z));
    tb_putcol(cells, w, w - 12, h, perc, TB_BOLD|TB_MAGENTA, TB_DEFAULT);

    getcwd(cwd, 4096);
    tb_putcol(cells, w, 3, h, cwd, TB_BOLD|TB_RED, TB_DEFAULT);
}

static struct tb_cell *tb_list_files(struct zipper *z, int bar)
{
    void *iter;
    char *entity;
    int width, height, h;
    struct tb_cell *cells;

    tb_clear();

    cells  = tb_cell_buffer();
    width = tb_width();
    height = tb_height() - 1;
    iter = zp_iter_init(z);
    h = 1;

    if (bar)
        tb_draw_statusbar(z, cells, width, height);

    while (iter && h < height) {
        entity = zp_iter_yield(&iter);
        tb_putfile(cells, width, 3, h, entity);
        h++;
        zp_iter_next(&iter);
    }

    if (bar)
        tb_present();

    return cells;
}

static inline void zp_refetch(struct zipper *z, char *path, int hidden)
{
    zp_cleanup(z);
    zp_init(z);
    zp_fetch_dir(z, path, hidden);
}

static void tb_cd_selected(struct zipper *z, int hidden)
{
    char *path = zp_selected_path(z);

    if (!path)
        return;

    if (is_dir(path)) {
        chdir(path);
        zp_refetch(z, ".", hidden);
    }
}

static void zp_zip_many(struct zipper *z, int n)
{
    if (n <= 0 || zp_zip(z) != 0)
        return;

    zp_zip_many(z, n - 1);
}

static void zp_unzip_many(struct zipper *z, int n)
{
    if (n <= 0 || zp_unzip(z) != 0)
        return;

    zp_unzip_many(z, n - 1);
}

static void zp_lookup(struct zipper *haystack, char *needle)
{
    int many = 0;
    void *iter;
    char *entity;

    iter = zp_iter_init(haystack);

    while (iter) {
        entity = zp_iter_yield(&iter);

        if (strstr(entity, needle))
            break;

        zp_iter_next(&iter);
        many++;
    }

    zp_zip_many(haystack, many);
}

static void cmd_do(struct zipper *z, int hidden, char *sock, char *cmd)
{
    int ufd, r;
    struct sockaddr_un address;
    char cwd[4096], cmdfmt[8192], *top;

    /* change current working directory */
    if (cmd[0] == 'c' &&  cmd[1] == 'd') {
        chdir(cmd + 3);
        zp_refetch(z, ".", hidden);
        return;
    }

    /* change current working directory */
    if (cmd[0] == 's' &&  cmd[1] == ' ') {
        zp_lookup(z, cmd + 2);
        return;
    }

    /* execute a shell command */
    if (cmd[0] == '!') {
        /* create new unix socket  */
        ufd = socket(PF_UNIX, SOCK_STREAM, 0);

        /* exit on fail */
        if (ufd < 0)
            return;

        /* setup struct */
        memset(&address, 0, sizeof(struct sockaddr_un));
        address.sun_family = AF_UNIX;
        strncat(address.sun_path, sock, sizeof(address.sun_path));

        /* connect to server */
        r = connect(ufd,
                    (struct sockaddr *)&address,
                    sizeof(struct sockaddr_un));

        /* exit on fail */
        if (r != 0)
            return;

        /* send cwd */
        getcwd(cwd, 4096);
        write(ufd, cwd, strlen(cwd));
        write(ufd, "\n", 1);

        /* send cmd */
        cmd++;

        if (strstr(cmd, "%s")) {
            top = zp_selected_path(z);
            snprintf(cmdfmt, 8192, cmd, top);
            write(ufd, cmdfmt, strlen(cmdfmt));
        } else {
            write(ufd, cmd, strlen(cmd));
        }

        write(ufd, "\n", 1);

        /* close socket */
        close(ufd);

        return;
    }
}

static void usage(void)
{
    fprintf(stderr, "usage: %s [-d <directory>] -s <unix socket>\n", argv0);
    exit(1);
}

int main(int argc, char *argv[])
{
    int t, h = 1;
    char *yield, *home, *sock = NULL;
    struct zipper z;
    struct tb_event e;
    struct tb_input in;
    struct tb_cell *cells;

    argv0 = *argv;
    home = getenv("HOME");

    ARGBEGIN {
        case 's':
            sock = EARGF(usage());
            break;
        case 'd':
            chdir(EARGF(usage()));
            break;
        default:
            usage();
    } ARGEND;

    if (!sock)
        usage();

    tb_init();
    tb_input_init(&in);
    zp_init(&z);

    zp_fetch_dir(&z, ".", h);
    tb_list_files(&z, 1);

    while (1) {
        t = tb_peek_event(&e, 200);

        switch (t) {
            case TB_EVENT_KEY:
                if(e.key == TB_KEY_CTRL_Q)
                    goto exit;

                if (tb_input_is_typing(&in)) {
                    switch (tb_input(&e, &in)) {
                        case TB_INPUT_DISCARD:
                            tb_input_done(&in);
                            break;
                        case TB_INPUT_DONE:
                            yield = tb_input_done(&in);
                            cmd_do(&z, h, sock, yield);
                            break;
                        default:
                            break;
                    }

                    goto update_window;
                }

                if (e.ch == ':') {
                    tb_input_begin(&in, 4, tb_height() - 1);
                } else if (e.ch == '!') {
                    tb_input_begin(&in, 4, tb_height() - 1);
                    in.buf[0] = '!';
                    in.cx++;
                    in.ptr++;
                } else if (e.ch == '/') {
                    tb_input_begin(&in, 4, tb_height() - 1);
                    in.buf[0] = 's'; in.buf[1] = ' ';
                    in.cx += 2;
                    in.ptr += 2;
                } else if (e.ch == 'c') {
                    tb_input_begin(&in, 4, tb_height() - 1);
                    in.buf[0] = 'c'; in.buf[1] = 'd'; in.buf[2] = ' ';
                    in.cx += 3;
                    in.ptr += 3;
                } else if (e.key == TB_KEY_CTRL_L) {
                    cmd_do(&z, h, sock, "!clear");
                } else if (e.ch == 'j' || e.key == TB_KEY_ARROW_DOWN) {
                    zp_zip(&z);
                } else if (e.ch == 'k' || e.key == TB_KEY_ARROW_UP) {
                    zp_unzip(&z);
                } else if (e.ch == 'h' || e.key == TB_KEY_ARROW_LEFT) {
                    chdir("..");
                    zp_refetch(&z, ".", h);
                } else if (e.ch == 'l' || e.key == TB_KEY_ARROW_RIGHT) {
                    tb_cd_selected(&z, h);
                } else if (e.ch == 'H') {
                    chdir(home);
                    zp_refetch(&z, ".", h);
                } else if (e.ch == 'r') {
                    zp_refetch(&z, ".", h);
                } else if (e.ch == '.') {
                    h ^= 1;
                    zp_refetch(&z, ".", h);
                } else if (e.ch == 'g') {
                    zp_unzipall(&z);
                } else if (e.ch == 'G') {
                    zp_zipall(&z);
                } else if (e.key == TB_KEY_CTRL_D) {
                    zp_zip_many(&z, 10);
                } else if (e.key == TB_KEY_CTRL_U) {
                    zp_unzip_many(&z, 10);
                } else {
                    continue;
                }
update_window:
            case TB_EVENT_RESIZE:
                cells = tb_cell_buffer();

                if (tb_input_is_typing(&in)) {
                    cells = tb_list_files(&z, 0);
                    yield = tb_input_yield(&in);
                    tb_change_prompt(cells);
                    tb_putcol(cells, tb_width(), 4, tb_height() - 1, yield, TB_BOLD, TB_DEFAULT);
                    tb_present();
                } else {
                    tb_list_files(&z, 1);
                }
                break;
            default:
                break;
        }
    }

exit:
    tb_shutdown();
    zp_cleanup(&z);

    return 0;
}
