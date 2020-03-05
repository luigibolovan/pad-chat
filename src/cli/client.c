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

#define DATA_PKG_LENGTH 1024

char name[32];

void str_ovrewrite_stdout(){
  printf("%s", "> ");
  fflush(stdout);
}

void trim_newline(char *array, int length){
  for(int i = 0; i < length; i++){
    if(array[i] == '\n'){
      array[i] = '\0';
      break;
    }
  }
}

void listen_server(void *arg){
    int read_size   = 0;
    int socket_fd   = *(int *) arg;
    char *buffer    = (char*)calloc(DATA_PKG_LENGTH, sizeof(char));

    while(1){
        while((read_size = read(socket_fd, buffer, DATA_PKG_LENGTH)) > 0)
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
    char *buffer        = (char*)calloc(DATA_PKG_LENGTH, sizeof(char));
    char *out           = (char*)calloc(DATA_PKG_LENGTH, sizeof(char));
    char *server_addr   = (char*)calloc(16,sizeof(char));
    int i = 1;

    if(argc != 4){
      printf("Usage: %s <ip> <port> <name>", argv[0]);
      exit(1);
    }

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
        //str_ovrewrite_stdout();
        //scanf("%d", &inp);

        char inp[DATA_PKG_LENGTH] = {};

        fgets(inp, DATA_PKG_LENGTH, stdin);
        trim_newline(inp, DATA_PKG_LENGTH);

        sprintf(out, "%s: %s", argv[3],inp);
        if(inp == NULL){
            break;
        }

        if(write(socket_fd, out, DATA_PKG_LENGTH) < 0){
            printf("error at sending\n");
            exit(7);
        }
    }
    free(buffer);
    free(out);
    free(server_addr);
    close(socket_fd);
}
