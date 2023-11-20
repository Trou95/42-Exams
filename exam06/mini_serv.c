#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int id, new;
} t_client;
t_client clients[10000] = {[0 ... 9999] = 1,1}; // You can use memset(&clients, 1, sizeof(clients)); instead
char buffer[10000], c;
int serv_fd, conn_fd, max_fd, client_count = 0;
fd_set sockets,r_fd, w_fd;

int ft_error(char* msg) {
    return write(2, msg, strlen(msg));
}

void send_all(char* buffer, int owner_fd) {
    for(int fd = 0; fd <= max_fd; fd++) {
        if(FD_ISSET(fd, &w_fd) && fd != owner_fd)
            write(fd, buffer, strlen(buffer));
    }
}

int main(int ac, char** av) {
    if(ac != 2)
        return ft_error("Wrong number of arguments\n");
    if((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return ft_error("Fatal error\n");
    struct sockaddr_in servaddr = {AF_INET, htons(atoi(av[1])), htonl(INADDR_ANY)};
    if(bind(serv_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        return ft_error("Fatal error\n");
    if(listen(serv_fd, 10) < 0)
        return ft_error("Fatal error\n");

    FD_ZERO(&sockets);
    FD_SET(serv_fd, &sockets);
    max_fd = serv_fd;
    for(int fd = 0; fd <= max_fd; fd++) {
        r_fd = w_fd = sockets;
        if(select(max_fd + 1, &r_fd, &w_fd, NULL, NULL) < 0)
            continue;
        if(FD_ISSET(fd, &r_fd)) {
            if(fd == serv_fd) {
                if((conn_fd = accept(serv_fd, NULL, NULL) < 0))
                    continue;
                sprintf(buffer, "server: client %d just arrived\n", client_count);
                send_all(buffer,conn_fd);
                clients[conn_fd].id = client_count++;
                FD_SET(conn_fd, &sockets);
                max_fd = conn_fd > max_fd ? conn_fd : max_fd;
            }
            else {
                if(recv(fd, &c, 1, 0) <= 0) {
                    sprintf(buffer, "server: client %d just left\n", clients[fd].id);
                    send_all(buffer, fd);
                    FD_CLR(fd, &sockets);
                    close(fd);
                }
                else {
                    if(clients[fd].new) {
                        clients[fd].new = 0;
                        sprintf(buffer, "client %d: ", clients[fd].id);
                        send_all(buffer, fd);
                    }
                    if(c == '\n')
                        clients[fd].new = 1;
                    send_all(&c, fd);
                }
            }
        }
    }
}