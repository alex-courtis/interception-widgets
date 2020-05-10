#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>

/* https://www.kernel.org/doc/html/latest/input/event-codes.html */
#define INPUT_VAL_PRESS 1
#define INPUT_VAL_RELEASE 0
#define INPUT_VAL_REPEAT 2

#define LENGTH(X) (sizeof X / sizeof X[0])

typedef struct Modifier {
    int code;
    unsigned int left;
    unsigned int active;
} Modifier;

static Modifier mods[] = {
    { KEY_LEFTSHIFT,  1, 0, },
    { KEY_RIGHTSHIFT, 0, 0, },
};

typedef struct Key {
    int code;
    unsigned int press_squished;
} Key;

static Key keysleft[] = {
    { KEY_RIGHTBRACE, 0, }, { KEY_2, 0, }, { KEY_3, 0, }, { KEY_4, 0, }, { KEY_5, 0, }, { KEY_6, 0, },
    { KEY_1, 0, },          { KEY_Q, 0, }, { KEY_W, 0, }, { KEY_E, 0, }, { KEY_R, 0, }, { KEY_T, 0, },
    { KEY_LEFTBRACE, 0, },  { KEY_A, 0, }, { KEY_S, 0, }, { KEY_D, 0, }, { KEY_F, 0, }, { KEY_G, 0, },
    { KEY_KP0, 0, },        { KEY_Z, 0, }, { KEY_X, 0, }, { KEY_C, 0, }, { KEY_V, 0, }, { KEY_B, 0, },
};

static Key keysright[] = {
    { KEY_7, 0, }, { KEY_8, 0, }, { KEY_9, 0, },     { KEY_0, 0, },   { KEY_MINUS, 0, },     { KEY_EQUAL, 0, },
    { KEY_Y, 0, }, { KEY_U, 0, }, { KEY_I, 0, },     { KEY_O, 0, },   { KEY_P, 0,     },     { KEY_BACKSLASH, 0, },
    { KEY_H, 0, }, { KEY_J, 0, }, { KEY_K, 0, },     { KEY_L, 0, },   { KEY_SEMICOLON, 0, }, { KEY_APOSTROPHE, 0, },
    { KEY_N, 0, }, { KEY_M, 0, }, { KEY_COMMA, 0, }, { KEY_DOT, 0, }, { KEY_SLASH, 0, },     { KEY_GRAVE, 0, },
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

unsigned int
should_squish(const struct input_event *input, Key keys[], const size_t nkeys) {
    static size_t i;

    for (i = 0; i < nkeys; i++) {
        if (input->code == keys[i].code) {
            if (input->value == INPUT_VAL_PRESS) {
                keys[i].press_squished = 1;
                return 1;
            }
            if (keys[i].press_squished) {
                if (input->value == INPUT_VAL_RELEASE) {
                    keys[i].press_squished = 0;
                }
                return 1;
            }
        }
    }

    return 0;
}

void
loop() {
    struct input_event input;
    size_t i;
    unsigned int al, ar, squish;

    while (read_event(&input)) {
        squish = 0;

        // uinput doesn't need sync events
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        // modifier states
        for (i = 0, al = 0, ar = 0; i < LENGTH(mods); i++) {
            if (input.code == mods[i].code)
                mods[i].active = input.value != INPUT_VAL_RELEASE;

            if (mods[i].active) {
                if (mods[i].left) {
                    al++;
                }
                else {
                    ar++;
                }
            }
        }

        // maybe squish if the user is pressing modifiers from one side only
        if (al > 0 && ar == 0)
            squish = should_squish(&input, keysleft, LENGTH(keysleft));
        else if (al == 0 && ar > 0)
            squish = should_squish(&input, keysright, LENGTH(keysright));

        if (!squish)
            write_event(&input);
    }
}

int
main() {
    setbuf(stdin, NULL), setbuf(stdout, NULL);

    loop();
}
