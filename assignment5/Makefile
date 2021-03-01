CC=gcc
CFLAGS= -std=gnu99 -Wall -g

all: enc_server dec_server enc_client dec_client keygen

enc_server: enc_server.c
	$(CC) $(CFLAGS) enc_server.c -o enc_server

dec_server: dec_server.c
	$(CC) $(CFLAGS) dec_server.c -o dec_server

enc_client: enc_client.c
	$(CC) $(CFLAGS) enc_client.c -o enc_client

dec_client: dec_client.c
	$(CC) $(CFLAGS) dec_client.c -o dec_client

keygen: keygen.c
	$(CC) $(CFLAGS) keygen.c -o keygen

.PHONY: clean
clean:
	rm -rf enc_server dec_server enc_client dec_client keygen
