include config.mk

INC = $(wildcard *.h)
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)

all: $(BIN) tags

$(OBJ): $(INC)

clean:
	rm -f tags $(OBJ) $(BIN)

install: $(BIN)
	chmod 755 $(BIN)
	mkdir -p $(PREFIX)/bin
	sudo cp -f $(BIN) $(PREFIX)/bin

uninstall:
	rm -f $(addprefix $(PREFIX)/bin/,$(BIN))

# https://github.com/alex-courtis/arch/blob/7ca6c8d7f7aa910ec522470bb7a96ddb24c9a1ea/bin/ctags-something
tags: $(SRC) $(INC)
	ctags-c $(CPPFLAGS) --project-src $(^)

.PHONY: all clean install uninstall

