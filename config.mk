PREFIX ?= /usr/local

INCS = -I/usr/include/libevdev-1.0

CPPFLAGS = $(INCS)

WFLAGS = -pedantic -Wall -Wextra -Werror -Wno-unused-parameter
COMPFLAGS = $(WFLAGS)
CFLAGS += $(COMPFLAGS) -std=gnu17

LDFLAGS +=

LDLIBS += -levdev

CC = gcc

