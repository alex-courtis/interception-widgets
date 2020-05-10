#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>

#define LENGTH(X) (sizeof X / sizeof X[0])

typedef struct Mapping {
    int from;
    int to;
} Mapping;

static const Mapping mappings[] = {
    { KEY_F11,  KEY_PASTE,  },
    { KEY_F12,  KEY_COPY,   },
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
        // uinput doesn't need sync events
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        // map
        for (i = 0; i < LENGTH(mappings); i++)
            if (mappings[i].from == input.code)
                input.code = mappings[i].to;

        // forward
        write_event(&input);
    }
}

int
main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    loop();
}

