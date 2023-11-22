#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int new, id;
} t_client;
t_client clients[10000] = {[0 ... 9999] = 1}; // You can use memset(&clients, 1, sizeof(clients)); instead
char buffer[10000], c;
int serv_fd, conn_fd, max_fd, client_count = 0;
fd_set sockets,r_fd, w_fd;

int ft_error(char* msg) {
    return write(2, msg, strlen(msg)) || 1;
}

void send_all(char* msg, int arg, int owner_fd) {
    sprintf(buffer, msg, arg);
    for(int fd = 0; fd <= max_fd; fd++)
        if(FD_ISSET(fd, &w_fd) && fd != owner_fd)
            write(fd, buffer, strlen(buffer));
}

int main(int ac, char** av) {
    if(ac != 2)
        return ft_error("Wrong number of arguments\n");
    if((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return ft_error("Fatal error\n");
    struct sockaddr_in servaddr = (struct sockaddr_in){0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(av[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(serv_fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) < 0)
        return ft_error("Fatal error\n");
    if(listen(serv_fd, 10) < 0)
        return ft_error("Fatal error\n");

    FD_ZERO(&sockets);
    FD_SET(serv_fd, &sockets);
    max_fd = serv_fd;
    while(1) {
        for(int fd = 0; fd <= max_fd; fd++) {
            r_fd = w_fd = sockets;
            if(select(max_fd + 1, &r_fd, &w_fd, NULL, NULL) < 0)
                continue;
            if(FD_ISSET(fd, &r_fd)) {
                if(fd == serv_fd) {
                    if((conn_fd = accept(serv_fd, NULL, NULL)) < 0)
                        continue;
                    send_all("server: client %d just arrived\n", client_count, conn_fd);
                    clients[conn_fd].id = client_count++;
                    FD_SET(conn_fd, &sockets);
                    max_fd = conn_fd > max_fd ? conn_fd : max_fd;
                }
                else {
                    if(recv(fd, &c, 1, 0) <= 0) {
                        send_all("server: client %d just left\n", clients[fd].id, fd);
                        FD_CLR(fd, &sockets);
                        close(fd);
                    }
                    else {
                        if(clients[fd].new) {
                            clients[fd].new = 0;
                            send_all("client %d: ", clients[fd].id, fd);
                        }
                        if(c == '\n')
                            clients[fd].new = 1;
                        send_all(&c, 0, fd);
                    }
                }
            }
        }
    }
}
