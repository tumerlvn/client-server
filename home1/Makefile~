all: server client
	
server: src/server.c
	gcc server.c -o server

client: src/client.c
	gcc client.c -o client

clean:
	rm server client
