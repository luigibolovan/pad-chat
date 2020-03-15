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
#include "../sockio/send.h"
#include "../auth/SearchUser.h"
#include <stdbool.h>

#define SERVER_PORT 9000
#define LISTEN_BACKLOG 10
#define PKG_SIZE 256
#define ALLOWED_NO_OF_CONNECTIONS 10
#define PASSWORD_MAX_SIZE 16

static int             saiConnectionFds[ALLOWED_NO_OF_CONNECTIONS];
static int             siConnectionCounter;
static pthread_mutex_t sMutexID;
static int             iThreadCounter;
static char            sastrConnectedUsers[ALLOWED_NO_OF_CONNECTIONS][PKG_SIZE]; //holds the names of the connected users

/**
 * Adds a new connection to the server in the connections array.
 */
static void addConnection(int connection_fd){
    if(siConnectionCounter < ALLOWED_NO_OF_CONNECTIONS){
        saiConnectionFds[siConnectionCounter++] = connection_fd;
    }
}

/**
 * Removes a new connection from the connections array.
 */
static void removeConnection(int connection_fd){
    int poz = 0;
    for(int i = 0; i < siConnectionCounter; i++){
        if(saiConnectionFds[i] == connection_fd) poz = i;
    }
    for(int i = poz; i < siConnectionCounter; i++){
        saiConnectionFds[i] = saiConnectionFds[i+1];
    }
    saiConnectionFds[siConnectionCounter - 1] = 0;
    siConnectionCounter--;
}

/**
 * Handles the connections to the server.
 * Firstly, it checks if the username is valid and aftewards it waits for the client to send messages.
 */
void handleConnections(void * pvArg){

    bool boUserIsInvalid        = false;
    bool boIsConnected          = false;
    bool boPermitMsg            = false;
    bool boIsAuthenticated      = false;
    bool boIsUsernameReceived   = false;
    bool boIsPasswordReceived   = false;
    char *strUsername           = (char*)calloc(PKG_SIZE, sizeof(char));
    char *strPassword           = (char*)calloc(PASSWORD_MAX_SIZE, sizeof(char));
    int iConnectionFD           = *(int *)pvArg;
    int iNoOfPackages           = 0;
    char *strBuffer             = (char*)calloc(PKG_SIZE, sizeof(char));
    bool boIsExitRequested      = false;
    FILE *pfUsersFile           = fopen("src/auth/assets/users.dat", "r"); 

    if(pfUsersFile == NULL){
        printf("FILE NOT FOUND\n");
        exit(EXIT_FAILURE);
    }

    printf("Connection file descriptor: %d\n", iConnectionFD);

    while(1){
        if(!boIsAuthenticated && !boPermitMsg){
            /*read the username*/
            if(read(iConnectionFD, &iNoOfPackages, 1) < 0){
                printf("Error\n");
                exit(EXIT_FAILURE);
            }
            if(!boIsUsernameReceived){
                printf("receiving username..\n");
                if(read(iConnectionFD, strUsername, PKG_SIZE) < 0){
                    printf("Username receving error: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                boIsUsernameReceived = true;
            }
            /*read the password*/
            if(read(iConnectionFD, &iNoOfPackages, 1) < 0){
                printf("Error\n");
                exit(EXIT_FAILURE);
            }
            if(!boIsPasswordReceived){
                printf("receiving password...\n");
                if(read(iConnectionFD, strPassword, PKG_SIZE) < 0){
                    printf("Error when receiving password from client: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                boIsPasswordReceived = true;
            }
            /*validate the user*/
            if(boIsPasswordReceived == true && boIsUsernameReceived == true && boIsAuthenticated == false){
                printf("Validating user...\n");
                for(int i = 1; i <= siConnectionCounter; i++){
                    if(strcmp(strUsername, sastrConnectedUsers[i]) == 0){
                        boIsConnected = true;
                    }
                }
                if(boIsConnected){
                    strcpy(strBuffer, "This user is already connected\n");
                    SockIO_send(iConnectionFD, strBuffer);
                    break;
                }

                if(ValidateUser(strUsername, strPassword, pfUsersFile) == 1){
                    strcpy(sastrConnectedUsers[siConnectionCounter], strUsername);
                    printf("%s\n",sastrConnectedUsers[siConnectionCounter]);
                    boIsAuthenticated = true;
                    boPermitMsg = true;
                    strcpy(strBuffer, "\nHello, ");
                    sprintf(strBuffer, "%s%s\n\n", strBuffer, strUsername);
                    SockIO_send(iConnectionFD, strBuffer);

                }
                else{
                    printf("Invalid user - username or password are incorrect\n");
                    strcpy(strBuffer,"Username or password are incorrect\n");
                    SockIO_send(iConnectionFD, strBuffer);
                    boUserIsInvalid = true;
                    break;
                }
            }
        }
        /* handle messages if it is a vaild user*/
        if(boIsAuthenticated && boPermitMsg){
            if(read(iConnectionFD, &iNoOfPackages, 1) < 0){
                printf("Error\n");
                exit(EXIT_FAILURE);
            }
            char *strMsgToBeSent        = (char*)calloc(iNoOfPackages * PKG_SIZE, sizeof(char));
            char *strResponse           = (char*)calloc(PKG_SIZE * iNoOfPackages + 15, sizeof(char));
            while((read(iConnectionFD, strBuffer, PKG_SIZE)) > 0){
                strcat(strMsgToBeSent, strBuffer);
                iNoOfPackages--;
                
                if(iNoOfPackages == 0){
                    /*checking the end request*/
                    if(strcmp(strMsgToBeSent, "/exit") == 0){
                        boIsExitRequested = true;
                        break;
                    }

                    /* compose the response*/
                    sprintf(strResponse, "%s: %s\n", strUsername, strMsgToBeSent);
                    printf("%s\n", strResponse);

                    /*send the response to the other clients*/
                    for(int i = 0; i < siConnectionCounter; i++){
                        if(saiConnectionFds[i] != iConnectionFD){
                            printf("Sending data to: %d\n", saiConnectionFds[i]);
                            SockIO_send(saiConnectionFds[i], strResponse);
                        }
                    }
                    break; //break after sending the message because there are no more packages
                }
            }

            /*close the connection if requested*/
            if(boIsExitRequested == true){
                printf("Finishing %d connection..\n", iConnectionFD);
                break;
            }

            if(boUserIsInvalid == true){
                break;
            }
            free(strMsgToBeSent);
            free(strResponse);
        }
    }

    pthread_mutex_lock(&sMutexID);
    iThreadCounter--;
    fclose(pfUsersFile);
    printf("%d active threads\n", iThreadCounter);
    removeConnection(iConnectionFD);
    pthread_mutex_unlock(&sMutexID);

    close(iConnectionFD);
}

int main(){
    int                 iSocketFD;
    int                 iConnectionFD;
    socklen_t           receiveLen;
    struct sockaddr_in  localAddr, remoteAddr;
    pthread_t           threadID;

    if((iSocketFD = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&localAddr, 0, sizeof(localAddr));

    localAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    localAddr.sin_port         = htons(SERVER_PORT);
    localAddr.sin_family       = AF_INET;


    if(bind(iSocketFD, (struct sockaddr *)&localAddr, sizeof(localAddr)) == -1){
        printf("%s\n", strerror(errno));
        printf("ERROR @ BIND\n");
        exit(EXIT_FAILURE);
    }

    if(listen(iSocketFD, LISTEN_BACKLOG) == -1){
        printf("%s", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        printf("\nSERVER LISTENING ON PORT: %d\n", SERVER_PORT);
    }

    if(pthread_mutex_init(&sMutexID, NULL) != 0){
        printf("Mutex initialization: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while(1){
        receiveLen = sizeof(remoteAddr);
        if((iConnectionFD = accept(iSocketFD, (struct sockaddr *)&remoteAddr, &receiveLen)) == -1){
            printf("%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(pthread_create(&threadID, NULL, (void *)handleConnections, &iConnectionFD) != 0){
            printf("Thread: %s\n", strerror(errno));
        }

        pthread_mutex_lock(&sMutexID);
        iThreadCounter++;
        addConnection(iConnectionFD);
        printf("%d active threads \n", iThreadCounter);
        pthread_mutex_unlock(&sMutexID);
    }

    close(iSocketFD);
    
    return 0;
}