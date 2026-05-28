/*******************************************************************************
 *
 *      vi.c
 *
 *      @brief
 *
 *      @author   Yukang Chen  @date  2013-08-04 10:50:42
 *
 *******************************************************************************/

#include <string.h>
#include <stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <syscall.h>

#define BUF_LEN 1024
#define CMD_LEN 16
#define STATUS_LEN 80
#define VIEW_ROWS 24
#define VIEW_COLS 79
#define OUT_LEN 4096

#define ESC     27
#define MODE_NORMAL 0
#define MODE_INSERT 1

#define KEY_LEFT  0x11
#define KEY_RIGHT 0x12
#define KEY_UP    0x13
#define KEY_DOWN  0x14

static char outbuf[OUT_LEN];
static int outpos;

static void out_flush() {
    if(outpos > 0) {
        write(1, outbuf, outpos);
        outpos = 0;
    }
}

static void out_ch(char c) {
    if(outpos >= OUT_LEN)
        out_flush();
    outbuf[outpos++] = c;
}

static void out_str(char* s) {
    while(*s)
        out_ch(*s++);
}

static void out_num(int n) {
    char tmp[16];
    sprintf(tmp, "%d", n);
    out_str(tmp);
}

static void out_cursor(int row, int col) {
    out_str("\033[");
    out_num(row);
    out_ch(';');
    out_num(col);
    out_ch('H');
}

static void out_clear_eol() {
    out_str("\033[K");
}

static void build_path(char* path, char* name) {
    char cwd[BUF_LEN];

    memset(path, 0, BUF_LEN);
    if(name[0] == '/') {
        strcpy(path, name);
        return;
    }

    memset(cwd, 0, sizeof(cwd));
    getcwd(cwd, sizeof(cwd));
    strcpy(path, cwd);
    path[strlen(path)] = '/';
    strcat(path, name);
}

static int min_int(int a, int b) {
    return a < b ? a : b;
}

static void set_status(char* status, char* text) {
    memset(status, 0, STATUS_LEN);
    strncpy(status, text, STATUS_LEN - 1);
}

static int line_start(char* buf, int pos) {
    while(pos > 0 && buf[pos - 1] != '\n')
        pos--;
    return pos;
}

static int line_end(char* buf, int len, int pos) {
    while(pos < len && buf[pos] != '\n')
        pos++;
    return pos;
}

static int cursor_line(char* buf, int pos) {
    int i, line = 0;
    for(i = 0; i < pos; i++) {
        if(buf[i] == '\n')
            line++;
    }
    return line;
}

static int cursor_col(char* buf, int pos) {
    return pos - line_start(buf, pos);
}

static int start_of_line_no(char* buf, int len, int line) {
    int i = 0;
    int cur_line = 0;
    while(i < len && cur_line < line) {
        if(buf[i++] == '\n')
            cur_line++;
    }
    return i;
}

static int update_top_line(char* buf, int cur, int top_line) {
    int line = cursor_line(buf, cur);
    if(line < top_line)
        return line;
    if(line >= top_line + VIEW_ROWS)
        return line - VIEW_ROWS + 1;
    return top_line;
}

static void move_left(int* cur) {
    if(*cur > 0)
        (*cur)--;
}

static void move_right(int len, int* cur) {
    if(*cur < len)
        (*cur)++;
}

static void move_up(char* buf, int* cur) {
    int start = line_start(buf, *cur);
    int col = *cur - start;
    int prev_end;
    int prev_start;

    if(start == 0)
        return;

    prev_end = start - 1;
    prev_start = line_start(buf, prev_end);
    *cur = prev_start + min_int(col, prev_end - prev_start);
}

static void move_down(char* buf, int len, int* cur) {
    int start = line_start(buf, *cur);
    int col = *cur - start;
    int end = line_end(buf, len, *cur);
    int next_start;
    int next_end;

    if(end >= len)
        return;

    next_start = end + 1;
    next_end = line_end(buf, len, next_start);
    *cur = next_start + min_int(col, next_end - next_start);
}

static int insert_char(char* buf, int* len, int* cur, char c) {
    int i;
    if(*len >= BUF_LEN - 1)
        return -1;
    for(i = *len; i >= *cur; i--)
        buf[i + 1] = buf[i];
    buf[*cur] = c;
    (*cur)++;
    (*len)++;
    return 0;
}

static void delete_at(char* buf, int* len, int cur) {
    int i;
    if(cur >= *len)
        return;
    for(i = cur; i < *len; i++)
        buf[i] = buf[i + 1];
    (*len)--;
}

static void draw_char(char c) {
    if(c == '\t')
        out_ch(' ');
    else if(c >= ' ')
        out_ch(c);
    else
        out_ch(' ');
}

static int screen_row_of(char* buf, int cur, int top_line) {
    int line = cursor_line(buf, cur);
    int row = line - top_line + 1;
    if(row < 1)
        row = 1;
    if(row > VIEW_ROWS)
        row = VIEW_ROWS;
    return row;
}

static int screen_col_of(char* buf, int cur) {
    int col = cursor_col(buf, cur) + 1;
    if(col < 1)
        col = 1;
    if(col > VIEW_COLS)
        col = VIEW_COLS;
    return col;
}

static void move_screen_cursor(char* buf, int cur, int top_line) {
    out_cursor(screen_row_of(buf, cur, top_line),
               screen_col_of(buf, cur));
}

static void draw_status(char* buf, int cur, int mode,
                        char* filename, char* status) {
    char status_line[160];
    int i;
    int cur_line = cursor_line(buf, cur) + 1;
    int col = cursor_col(buf, cur) + 1;

    memset(status_line, 0, sizeof(status_line));
    if(status[0]) {
        sprintf(status_line, "%s  Ln %d, Col %d", status, cur_line, col);
    } else if(mode == MODE_INSERT) {
        sprintf(status_line, "INSERT  Ln %d, Col %d", cur_line, col);
    } else {
        sprintf(status_line, "NORMAL  Ln %d, Col %d", cur_line, col);
    }

    out_cursor(25, 1);
    for(i = 0; status_line[i] && i < VIEW_COLS; i++)
        out_ch(status_line[i]);
    out_clear_eol();
}

static void draw_screen(char* buf, int len, int cur, int top_line,
                        int mode, char* filename, char* status) {
    int row, col, pos;

    pos = start_of_line_no(buf, len, top_line);

    for(row = 1; row <= VIEW_ROWS; row++) {
        out_cursor(row, 1);
        col = 0;
        while(pos < len && buf[pos] != '\n') {
            if(col < VIEW_COLS) {
                draw_char(buf[pos]);
                col++;
            }
            pos++;
        }
        if(pos < len && buf[pos] == '\n')
            pos++;
        out_clear_eol();
    }

    draw_status(buf, cur, mode, filename, status);
    move_screen_cursor(buf, cur, top_line);
    out_flush();
}

static void refresh_cursor(char* buf, int cur, int top_line,
                           int mode, char* filename, char* status) {
    draw_status(buf, cur, mode, filename, status);
    move_screen_cursor(buf, cur, top_line);
    out_flush();
}

static int read_cmd(char* cmd, int len) {
    char c;
    int idx = 0;

    out_flush();
    printf("\033[25;1H\033[K:");
    while((c = getchar()) != '\n') {
        if(c == ESC) {
            printf("\033[25;1H\033[K");
            return 0;
        }
        if(c == 0x08) {
            if(idx > 0) {
                idx--;
                cmd[idx] = 0;
                printf("\b \b");
            }
            continue;
        }
        if(idx == 0 && (c == ' ' || c == '\t'))
            continue;
        if(idx < len - 1) {
            cmd[idx++] = c;
            printf("%c", c);
        }
    }

    while(idx > 0 && (cmd[idx - 1] == ' ' || cmd[idx - 1] == '\t'))
        idx--;
    cmd[idx] = 0;
    return idx > 0;
}

int main(int argc, char* argv[]) {
    char buf[BUF_LEN];
    char cmd[CMD_LEN];
    char filename[BUF_LEN];
    char status[STATUS_LEN];
    char c;
    struct stat s;
    int fd, r;
    int len = 0;
    int cur = 0;
    int top_line = 0;
    int mode = MODE_NORMAL;
    int pending_w = 0;

    if(argc != 2) {
        printf("usage: vi <filename>\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    memset(status, 0, sizeof(status));
    build_path(filename, argv[1]);
    memset(&s, 0, sizeof(s));

    if(stat(filename, &s) < 0) {
        fd = open(filename, O_CREATE|O_RDWR, 0);
    } else {
        if(!S_ISREG(s.st_mode)) {
            printf("vi: %s is not a regular file\n", filename);
            return -1;
        }
        fd = open(filename, O_RDONLY, 0);
        if(fd < 0) {
            printf("vi: open %s failed\n", filename);
            return -1;
        }
        r = read(fd, buf, BUF_LEN - 1);
        close(fd);
        if(r > 0)
            len = r;
        fd = open(filename, O_RDWR, 0);
    }

    if(fd < 0) {
        printf("vi: open %s failed\n", filename);
        return -1;
    }

    top_line = update_top_line(buf, cur, top_line);
    draw_screen(buf, len, cur, top_line, mode, filename, status);

    while(1) {
        c = getchar();

        if(mode == MODE_INSERT) {
            if(c == ESC) {
                mode = MODE_NORMAL;
                pending_w = 0;
            } else if(c == 0x08) {
                if(cur > 0) {
                    move_left(&cur);
                    delete_at(buf, &len, cur);
                }
            } else if(insert_char(buf, &len, &cur, c) < 0) {
                set_status(status, "vi: buffer full");
            } else {
                status[0] = 0;
            }
            top_line = update_top_line(buf, cur, top_line);
            draw_screen(buf, len, cur, top_line, mode, filename, status);
            continue;
        }

        {
        int old_top = top_line;
        int redraw = 0;
        status[0] = 0;

        if(pending_w) {
            pending_w = 0;
            if(c == 'q')
                goto save_exit;
        }

        if(c == 'h' || c == KEY_LEFT) {
            move_left(&cur);
        } else if(c == 'l' || c == KEY_RIGHT) {
            move_right(len, &cur);
        } else if(c == 'k' || c == KEY_UP) {
            move_up(buf, &cur);
        } else if(c == 'j' || c == KEY_DOWN) {
            move_down(buf, len, &cur);
        } else if(c == '0') {
            cur = line_start(buf, cur);
        } else if(c == '$') {
            cur = line_end(buf, len, cur);
        } else if(c == 'x') {
            delete_at(buf, &len, cur);
            redraw = 1;
        } else if(c == 'i') {
            mode = MODE_INSERT;
            pending_w = 0;
        } else if(c == 'a') {
            move_right(len, &cur);
            mode = MODE_INSERT;
            pending_w = 0;
        } else if(c == ':') {
            pending_w = 0;
            memset(cmd, 0, sizeof(cmd));
            if(!read_cmd(cmd, sizeof(cmd))) {
                top_line = update_top_line(buf, cur, top_line);
                refresh_cursor(buf, cur, top_line, mode, filename, status);
                continue;
            }

            if(strcmp(cmd, "wq") == 0)
                goto save_exit;
            if(strcmp(cmd, "q") == 0 || strcmp(cmd, "q!") == 0)
                goto exit;

            set_status(status, "vi: unknown command");
        } else if(c == 'w') {
            pending_w = 1;
        } else {
            pending_w = 0;
        }

        top_line = update_top_line(buf, cur, top_line);
        if(redraw || top_line != old_top)
            draw_screen(buf, len, cur, top_line, mode, filename, status);
        else
            refresh_cursor(buf, cur, top_line, mode, filename, status);
        }
    }

save_exit:
    close(fd);
    fd = open(filename, O_TRUNC|O_RDWR, 0);
    if(fd < 0) {
        printf("vi: save %s failed\n", filename);
        return -1;
    }
    write(fd, buf, len);
exit:
    close(fd);
    printf("\033[25;1H\033[Kexiting vi ...\n");
    return 0;
}
