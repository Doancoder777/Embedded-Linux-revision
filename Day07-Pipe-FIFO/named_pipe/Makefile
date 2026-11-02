# Named Pipe - FIFO

CC := gcc

all:
	$(CC) -o producer producer.c
	$(CC) -o consumer consumer.c

clean:
	rm -rf  producer consumer myfifo

createfifo:
	mkfifo -m 0666 testfifo