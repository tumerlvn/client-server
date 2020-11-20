#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

enum errors {
    OK,
    ERR_INCORRECT_ARGS,
    ERR_SOCKET,
    ERR_SETSOCKETOPT,
    ERR_BIND,
    ERR_LISTEN
};

int init_socket(int port) {
    //open socket, return socket descriptor
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Fail: open socket");
        _exit(ERR_SOCKET);
    }
 
    //set socket option
    int socket_option = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));
    if (server_socket < 0) {
        perror("Fail: set socket options");
        _exit(ERR_SETSOCKETOPT);
    }

    //set socket address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_BIND);
    }

    //listen mode start
    if (listen(server_socket, 5) < 0) {
        perror("Fail: bind socket address");
        _exit(ERR_LISTEN);
    }
    return server_socket;
}

int getFname(char *fname, int client_socket) {
    int size = 0;
    char http[28];
    while(read(client_socket, &(fname[size]), 1) > 0) {
        size++;
        if (fname[size-1] == ' ') {
            fname[size-1] = 0;
            break;
        }
    }
    read(client_socket, http, 28);
    return size;
}

void writeNum(int client_socket, int size) {
    char buf[1000] = {0};
    char bufSize = 0;
    if (size == 0) {
        write(client_socket, "0", 1);
    } else {
        while (size > 0) {
            buf[bufSize] = size % 10 + '0';
            size /= 10;
            bufSize++;
        }
        for (int i = bufSize - 1; i >= 0; i--) {
            write(client_socket, &(buf[i]), 1);
        }
    }
}

void sendDataBack(char *fname, int client_socket) {
    char ch;
    int fd =  open(fname, O_RDONLY);
    if (fd > 0) {
        write(client_socket, "HTTP/1.1 200\n", 13);
        write(client_socket, "content-type: html/text\n", 24);
        write(client_socket, "content-length: ", 16);
        int size = 0; 
        char buf[10000] = {0};
        while (read(fd, &ch, 1)) {
            buf[size] = ch;
            size++;
        }
        writeNum(client_socket, size);
        write(client_socket, "\n\n", 2);
        write(client_socket, buf, size);
    } else {
        write(client_socket, "HTTP/1.1 404\n", 13);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        puts("Incorrect args.");
        puts("./server <port>");
        puts("Example:");
        puts("./server 5000");
        return ERR_INCORRECT_ARGS;
    }
    int port = atoi(argv[1]);
    int server_socket = init_socket(port);
    puts("Wait for connection");
    struct sockaddr_in client_address;
    socklen_t size;
    int client_socket = accept(server_socket, 
                           (struct sockaddr *) &client_address,
                           &size);
    char ch;
    int j = 0;
    while (read(client_socket, &ch, 1) > 0) {
        if (ch == ' ') {
            char fname[100] = {0};
            j = getFname(fname, client_socket);
            if (fork() == 0) {
                sendDataBack(fname, client_socket);
                _exit(1);
            }
            wait(NULL);
            j = 0; 
        }
    }
    close(client_socket);

    return OK;
}
