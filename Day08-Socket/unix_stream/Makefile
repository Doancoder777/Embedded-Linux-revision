CC = gcc
CFLAGS ?= -Wall -Wextra
MSG ?= hello

.PHONY: all run-server run-client clean

all: server client

server: stream_server.c
	$(CC) $(CFLAGS) -o $@ $<

client: stream_client.c
	$(CC) $(CFLAGS) -o $@ $<

run-server: server
	./server

run-client: client
	./client "$(MSG)"

clean:
	rm -f server client
