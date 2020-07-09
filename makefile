CC = gcc

server: server.c client
	$(CC) server.c -o server

client: client.c
	$(CC) client.c -o client

clean:
	rm client server
