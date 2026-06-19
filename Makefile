CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Werror -std=c11 -O3

PREFIX ?= /usr/local
DESTDIR ?=

INCLUDEDIR := $(DESTDIR)$(PREFIX)/include
LIBDIR := $(DESTDIR)$(PREFIX)/lib

NAME := nini
LIB  := lib$(NAME).a
SRC  := $(NAME).c
OBJ  := $(NAME).o

all: $(LIB)

$(OBJ): $(SRC) $(NAME).h
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

$(LIB): $(OBJ)
	ar rcs $(LIB) $(OBJ)

install: $(LIB)
	install -d $(INCLUDEDIR)
	install -d $(LIBDIR)
	install -m 644 $(NAME).h $(INCLUDEDIR)/
	install -m 644 $(LIB) $(LIBDIR)/

uninstall:
	rm -f $(INCLUDEDIR)/$(NAME).h
	rm -f $(LIBDIR)/$(LIB)

clean:
	rm -f $(OBJ) $(LIB)

.PHONY: all install uninstall clean
