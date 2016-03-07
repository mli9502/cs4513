all: client server

client: client.c
	gcc -o client client.c -lcrypt

server: server.c
	gcc -o server server.c -lcrypt

clean:
	rm client server
	rm *.o
