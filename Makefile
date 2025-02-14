CC = gcc
CFLAGS = -Wall -Wextra -I./src
LFLAGS = -lssl -lcrypto
SRC = src/main.c src/file_handler.c src/deduplication.c src/backup_manager.c src/network.c
OBJ = $(SRC:.c=.o)

all: cborgbackup

cborgbackup: $(OBJ)
	$(CC) -o $@ $^ $(LFLAGS)

clean:
	rm -f $(OBJ) lp25_borgbackup