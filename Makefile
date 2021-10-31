include config.mk

INC_H =

SRC_C = $(wildcard *.c)
SRC_O = $(SRC_C:.c=.o)

all: keycodemapper modifiertrainer tags

$(OBJ): $(INC_H)

clean:
	rm -f keycodemapper modifiertrainer tags $(SRC_O)

install:
	mkdir -p $(PREFIX)/bin
	cp -f keycodemapper $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/keycodemapper
	cp -f modifiertrainer $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/modifiertrainer

uninstall:
	rm -f $(PREFIX)/bin/keycodemapper $(PREFIX)/bin/modifiertrainer

# https://github.com/alex-courtis/arch/blob/7ca6c8d7f7aa910ec522470bb7a96ddb24c9a1ea/bin/ctags-something
tags: $(SRC_C)
	ctags-c   $(CFLAGS)   $(CPPFLAGS) --project-src $(SRC_C)

.PHONY: all clean install uninstall

