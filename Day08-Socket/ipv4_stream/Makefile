PORT ?= 1234
IP ?= 127.0.0.1

all:
	gcc -o server stream_server.c
	gcc -o client stream_client.c

run-server:
	./server $(PORT)

run-client:
	./client $(IP) $(PORT)

clean:
	rm -rf client server