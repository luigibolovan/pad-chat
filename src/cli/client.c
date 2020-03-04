#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include "../sockio/send.h"

#define PKG_SIZE                256
#define ALLOWED_DATA_LENGTH     32678

void listen_server(void *arg){
    int read_size   = 0;
    int socket_fd   = *(int *) arg;
    char *buffer    = (char*)calloc(PKG_SIZE, sizeof(char));

    while(1){
        while((read_size = read(socket_fd, buffer, PKG_SIZE)) > 0)
            printf("%s",buffer);
    }

    free(buffer);
}


int main(int argc, char **argv){
    
    int                 socket_fd;
    struct sockaddr_in  local_addr, remote_addr;
    socklen_t           receive_len;
    int                 connection_fd;
    pthread_t           thread_id;
    int                 port;
    char *server_addr   = (char*)calloc(16, sizeof(char));
    char *inp           = (char*)calloc(ALLOWED_DATA_LENGTH, sizeof(char));
    

    strcpy(server_addr, argv[1]);
    printf("%s\n", server_addr);
    port = atoi(argv[2]);

    if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("%s\n", strerror(errno));
        exit(1);
    }

    local_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    local_addr.sin_port         = 0;
    local_addr.sin_family       = AF_INET;

    if(bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
        printf("%s\n", strerror(errno));
        exit(2);
    }

    remote_addr.sin_addr.s_addr = inet_addr(server_addr);
    remote_addr.sin_port        = htons(port);
    remote_addr.sin_family      = AF_INET;

    if(connect(socket_fd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == -1){
        printf("CONNECTION\n");
        printf("%s\n", strerror(errno));
        exit(3);
    }
    printf("CONNECTED TO PORT:%d\n", port);

    if(pthread_create(&thread_id, NULL, (void *)listen_server, &socket_fd) != 0){
        printf("Thread creation error: %s\n", strerror(errno));
    }

    while(1){
        scanf("%[^\n]s", inp);
        while((getchar()) != '\n');
        if(strcmp(inp, "exit") == 0){
            break;
        }
        SockIO_send(socket_fd, inp);
        // write(socket_fd, inp, PKG_SIZE);
    }

    free(server_addr);
    close(socket_fd);
}
