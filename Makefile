include config.mk

HDR =
CSRC = keycodemapper.c modifiertrainer.c
CXXSRC =

OBJ = $(CSRC:.c=.o) $(CXXSRC:.cpp=.o)

all: keycodemapper modifiertrainer tags

$(OBJ): config.mk $(HDR)

clean:
	rm -f keycodemapper modifiertrainer tags $(OBJ)

install:
	mkdir -p $(PREFIX)/bin
	cp -f keycodemapper $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/keycodemapper
	cp -f modifiertrainer $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/modifiertrainer

uninstall:
	rm -f $(PREFIX)/bin/keycodemapper $(PREFIX)/bin/modifiertrainer

tags: $(CSRC)
	ctags --fields=+S --c-kinds=+p \
		$(^) \
		/usr/include/st*.h \
		/usr/include/linux/input*.h

.PHONY: all clean install uninstall ctags
