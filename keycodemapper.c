#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>

#include "common.h"

typedef struct Mapping {
    int from;
    int to;
} Mapping;

static const Mapping mappings[] = {
    { KEY_LEFTALT, KEY_LEFTMETA, },
    { KEY_RIGHTALT, KEY_RIGHTMETA, },
    { KEY_LEFTMETA, KEY_LEFTALT, },
};

int
read_event(struct input_event *event) {
    return fread(event, sizeof(struct input_event), 1, stdin) == 1;
}

void
write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1)
        exit(EXIT_FAILURE);
}

void
loop() {
    struct input_event input;
    size_t i;

    while (read_event(&input)) {
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event, including SYNs
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        // map
        for (i = 0; i < LENGTH(mappings); i++) {
            if (mappings[i].from == input.code) {
                input.code = mappings[i].to;
                break;
            }
        }

        // forward
        write_event(&input);
    }
}

int
main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    loop();
}

