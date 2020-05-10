include config.mk

HDR =
CSRC = keycodemapper.c modifiertrainer.c
CXXSRC =

OBJ = $(CSRC:.c=.o) $(CXXSRC:.cpp=.o)

all: keycodemapper modifiertrainer

$(OBJ): config.mk $(HDR)

keycodemapper: keycodemapper.o

modifiertrainer: modifiertrainer.o

clean:
	rm -f keycodemapper modifiertrainer $(OBJ)

install:
	mkdir -p $(PREFIX)/bin
	cp -f keycodemapper $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/keycodemapper
	cp -f modifiertrainer $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/modifiertrainer

uninstall:
	rm -f $(PREFIX)/bin/keycodemapper $(PREFIX)/bin/modifiertrainer

ctags:
	ctags-c $(CPPFLAGS) $(HDR) $(CSRC) $(CXXSRC)

.PHONY: all clean install uninstall ctags
