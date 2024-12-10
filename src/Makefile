TARGET ?= ifshow # Defaut value : ifshow

CC = aarch64-alpine-linux-musl-gcc
CFLAGS = -Wall -Wextra -O2	# Active all warnings and optimize the code

SRC = $(TARGET)/$(TARGET).c
BIN = $(TARGET)/$(TARGET)

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm -f $(BIN)
.PHONY: clean

install:
	cp $(BIN) /usr/local/bin/
.PHONY: install

uninstall:
	rm -f /usr/local/bin/$(BIN)
.PHONY: uninstall
