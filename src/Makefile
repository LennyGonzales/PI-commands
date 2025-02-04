TARGET ?= ifshow # Defaut value : ifshow

CC = gcc
CFLAGS = -static -Wall -Wextra -O2	# Active all warnings and optimize the code

SRC = $(TARGET).c
BIN = $(TARGET)

all: $(BIN)

$(BIN): $(SRC)
#	$(CC) $(CFLAGS) -o ifshow ifshow.c interfaces.c
	$(CC) $(CFLAGS) -o ifnetshow ifnetshow.c interfaces.c
#	$(CC) $(CFLAGS) -o neighborshow neighborshow.c


clean:
	rm -f $(BIN)
.PHONY: clean

install:
	cp $(BIN) /usr/local/bin/
.PHONY: install

uninstall:
	rm -f /usr/local/bin/$(BIN)
.PHONY: uninstall
