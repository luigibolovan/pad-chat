#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 9000
#define LISTEN_BACKLOG 10
#define DATA_PKG_LENGTH 1024


void handle_msg(void * arg){
    int size = 0;
    char *out = (char*)calloc(DATA_PKG_LENGTH, sizeof(char));
    char *buffer = (char*)calloc(DATA_PKG_LENGTH, sizeof(char));
    int connection_fd = *(int *)arg;
    printf("Connection file descriptor: %d\n", connection_fd);
    while(1){
        size = read(connection_fd, buffer, DATA_PKG_LENGTH);
        printf("Server has read: %s\n", buffer);
        if(size < 0){
            printf("error when reading from server\n");
        }
        sprintf(out, "FROM SERVER : %s\n", buffer);
        write(connection_fd, out, DATA_PKG_LENGTH);
    }

    free(buffer);
    free(out);
    close(connection_fd);
}

int main(){
    int socket_fd;
    int connection_fd;
    socklen_t receive_len;
    struct sockaddr_in local_addr, remote_addr;
    pthread_mutex_t mutex_id;
    pthread_t thread_id;
    int thread_cnt = 0;

    if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("%s\n", strerror(errno));
        exit(1);
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(SERVER_PORT);
    local_addr.sin_family = AF_INET;


    if(bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
        printf("%s\n", strerror(errno));
        printf("ERROR @ BIND\n");
        exit(2);
    }

    if(listen(socket_fd, LISTEN_BACKLOG) == -1){
        printf("%s", strerror(errno));
        exit(1);
    } else {
        printf("\nSERVER LISTENING ON PORT: %d\n", SERVER_PORT);
    }

    if(pthread_mutex_init(&mutex_id, NULL) != 0){
        printf("Mutex initialization: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while(1){
        receive_len = sizeof(remote_addr);
        if((connection_fd = accept(socket_fd, (struct sockaddr *)&remote_addr, &receive_len)) == -1){
            printf("%s\n", strerror(errno));
            exit(3);
        }
        if(pthread_create(&thread_id, NULL, (void *)handle_msg, &connection_fd) != 0){
            printf("Thread creation: %s\n", strerror(errno));
        }
        
        pthread_mutex_lock(&mutex_id);
        thread_cnt++;
        printf("%d active threads \n", thread_cnt);
        pthread_mutex_unlock(&mutex_id);
    }

    close(socket_fd);

    return 0;
}