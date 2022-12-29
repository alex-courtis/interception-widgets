#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/input.h>
#include <linux/limits.h>
#include <unistd.h>

#include "common.h"

static FILE *lf = NULL;
static struct timeval tv;

void lg_print(const char *prefix, const char *__restrict __format, va_list __args) {

    gettimeofday(&tv, NULL);
    struct tm *tm = localtime(&tv.tv_sec);

    fprintf(lf, "%s [%02d:%02d:%02d.%03ld] ", prefix, tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec / 1000);
    vfprintf(lf, __format, __args);
    fprintf(lf, "\n");
    fflush(lf);
}

void lg_err(const char *__restrict __format, ...) {
    va_list args;
    va_start(args, __format);
    lg_print("E", __format, args);
    va_end(args);
}

void lg_inf(const char *__restrict __format, ...) {
    va_list args;
    va_start(args, __format);
    lg_print("I", __format, args);
    va_end(args);
}

void lg_contents(char *contents, char *desc) {
    static char buf[32];
    static int n;

    n = snprintf(buf, 32, "%s", contents);
    lg_inf("%s '%s%s'", desc, buf, n > 32 ? "..." : "");
}

bool shift_down = false;
bool twiddled = false;

#define CLIP_SEL_CHUNK 16384
static char *clipboard = NULL;
static int clipboard_size = 0;
static char *selection = NULL;
static int selection_size = 0;

int
read_event(struct input_event *event) {
    return fread(event, sizeof(struct input_event), 1, stdin) == 1;
}

void
write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1)
        exit(EXIT_FAILURE);
}

bool write_cmd(char *command, char **from) {
    static int rc;
    static int dup_stdout;
    static int dup_stderr;
    bool success = true;

    // redirect stdout, stderr to our log file for the duration of the command
    dup_stdout = dup(fileno(stdout));
    dup2(fileno(lf), fileno(stdout));
    dup_stderr = dup(fileno(stderr));
    dup2(fileno(lf), fileno(stderr));

    // execute with command stdin set to fc
    FILE *fc = popen(command, "w");
    if (fc == NULL) {
        lg_err("'%s' popen failed errno=%d '%s'", command, errno, strerror(errno));
        success = false;
        goto finally;
    }

    // write to its stdin
    fputs(*from, fc);

    // close and look at the return code
    rc = pclose(fc);
    if (rc != 0) {
        if (rc == -1) {
            lg_err("'%s' pclose failed errno=%d '%s'", command, errno, strerror(errno));
        } else {
            lg_err("'%s' failed rc=%d", command, rc >> 8);
        }
        success = false;
        goto finally;
    }

finally:

    // restore stdout, stderr
    dup2(dup_stdout, fileno(stdout));
    dup2(dup_stderr, fileno(stderr));

    return success;
}

bool read_cmd(char *command, char **to, int *to_size, bool trim_trailing) {
    static char buf[1024];
    static int rc;
    static int to_offset;
    static int dup_stderr;
    bool success = true;

    if (*to == NULL) {
        *to_size = CLIP_SEL_CHUNK;
        *to = malloc(CLIP_SEL_CHUNK);
    }

    // redirect stderr to our log file for the duration of the command
    dup_stderr = dup(fileno(stderr));
    dup2(fileno(lf), fileno(stderr));

    // execute with command stdout set to fc
    FILE *fc = popen(command, "r");
    if (fc == NULL) {
        lg_err("'%s' popen failed errno=%d '%s'", command, errno, strerror(errno));
        success = false;
        goto finally;
    }

    // read the stdout results
    to_offset = 0;
    while (fgets(buf, 1024, fc) != NULL) {
        if (*to_size - to_offset < 1024) {
            *to_size += CLIP_SEL_CHUNK;
            *to = realloc(*to, *to_size);
        }
        to_offset += snprintf(*to + to_offset, 1024, "%s", buf);
    }

    // close and look at the return code
    rc = pclose(fc);
    if (rc != 0) {
        if (rc == -1) {
            lg_err("'%s' pclose failed errno=%d '%s'", command, errno, strerror(errno));
        } else {
            lg_err("'%s' failed rc=%d", command, rc >> 8);
        }
        success = false;
        goto finally;
    }

    if (trim_trailing) {
        (*to)[strlen(*to) - 1] = '\0';
    }

finally:

    // restore stderr
    dup2(dup_stderr, fileno(stderr));

    return success;
}

void twiddle(void) {
    if (twiddled)
        return;

    // read the clipboard
    if (read_cmd("wl-paste", &clipboard, &clipboard_size, true)) {
        lg_contents(clipboard, "wl-paste repl clipboard");

        // read the selection
        if (!read_cmd("wl-paste -p", &selection, &selection_size, true))
            return;
        lg_contents(selection, "with selection");

        // write the selection to clipboard
        if (!write_cmd("wl-copy", &selection))
            return;

    } else if (read_cmd("xsel -o -b", &clipboard, &clipboard_size, false)) {
        lg_contents(clipboard, "xsel repl clipboard");

        // read the selection
        if (!read_cmd("xsel -o -p", &selection, &selection_size, false))
            return;
        lg_contents(selection, "with selection");

        // write the selection to clipboard
        if (!write_cmd("xsel -i -b", &selection))
            return;

    } else {
        lg_err("failed to twiddle with wl-paste and wl-copy");
        return;
    }

    twiddled = true;
}

void untwiddle(void) {
    if (!twiddled)
        return;

    // we are not twiddled regardless of failure
    twiddled = false;

    // restore the clipboard
    if (write_cmd("wl-copy", &clipboard)) {
        lg_contents(clipboard, "wl-copy rest clipboard");
    } else if (write_cmd("xsel -i -b", &clipboard)) {
        lg_contents(clipboard, "xsel rest clipboard");
    } else {
        lg_err("failed to untwiddle with wl-paste and wl-copy");
    }
}

void
loop(void) {
    struct input_event input;

    while (read_event(&input)) {
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event, including SYNs
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        if (input.code == KEY_LEFTSHIFT || input.code == KEY_RIGHTSHIFT) {
            shift_down = input.value != INPUT_VAL_RELEASE;
        }

        // twiddle on shifted insert PRESS
        if (shift_down && input.code == KEY_INSERT && input.value == INPUT_VAL_PRESS) {
            twiddle();
        }

        // twiddle on shifted insert RELEASE
        if (shift_down && input.code == KEY_INSERT && input.value == INPUT_VAL_RELEASE) {
            untwiddle();
        }

        // forward
        write_event(&input);
    }
}

int
// main_real() {
main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    char buf[PATH_MAX];
    sprintf(buf, "/tmp/selpaster.%d", getpid());
    lf = fopen(buf, "w");
    lg_inf("selpaster starting");

    loop();

    return 0;
}

int
main_cli_test(void) {
// main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    char buf[PATH_MAX];
    sprintf(buf, "/tmp/selpaster.log");
    lf = fopen(buf, "w");

    lg_inf("selpaster starting");

    twiddle();
    // untwiddle();

    return 0;
}

