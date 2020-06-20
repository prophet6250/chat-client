CC = gcc

server.exe: server.c client.exe
	$(CC) server.c -o server.exe

client.exe: client.c
	$(CC) client.c -o client.exe

clean:
	rm *.exe
