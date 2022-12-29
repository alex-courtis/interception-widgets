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

.PHONY: all clean install uninstall

