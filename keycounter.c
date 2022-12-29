#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/limits.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define PERIOD_SEC 5

static FILE *log = NULL;
static time_t last = 0;

int read_event(struct input_event *event) {
    return fread(event, sizeof(struct input_event), 1, stdin) == 1;
}

void write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1)
        exit(EXIT_FAILURE);
}

char *time_string(time_t *t) {
    static char buf[32];

    strftime(buf, sizeof(buf), "%Y-%m-%d-%H:%M:%S", localtime(t));

    return buf;
}

void count(void) {
    static long count = 0;
    static time_t now = 0;

    now = time(NULL);
    count++;

    // fill in periods with no activity
    while (now - 2 * PERIOD_SEC >= last) {
        fprintf(log, "%s,0\n", time_string(&last));

        last += PERIOD_SEC;
    }

    // record period
    if (now - PERIOD_SEC >= last) {
        fprintf(log, "%s,%ld\n", time_string(&last), count);

        last += PERIOD_SEC;
        count = 0;
    }

    fflush(log);
}

void loop(void) {
    struct input_event input;

    while (read_event(&input)) {
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event, including SYNs
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        // presses only
        if (input.value == INPUT_VAL_PRESS) {
            count();
        }

        // forward
        write_event(&input);
    }
}

int main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    char buf[PATH_MAX];
    time_t now = time(NULL);

    sprintf(buf, "/tmp/keycounter.%s.csv", time_string(&now));
    log = fopen(buf, "w");

    // round to nearest
    last = time(NULL);
    last -= last % PERIOD_SEC;

    loop();
}
