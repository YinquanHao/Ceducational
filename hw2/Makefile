CC = gcc
CFLAGS = -Wall -Werror
GG= -g
DD = -DCSE320
BIN = utfconverter
BIN1 = utfconverter.c


SRC = $(wildcard *.c)

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: debug

debug :	
	$(CC) $(GG) $(DD) $(BIN1) -o $(BIN)


.PHONY: clean

clean:
	rm -f *.o $(BIN)

