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
#include <stdbool.h>
#include <signal.h>

#define PKG_SIZE                256
#define ALLOWED_DATA_LENGTH     32678
#define SERVER_IP_SIZE          16
#define PASSWORD_MAX_SIZE       16

static int iSocketFD;

/**
 * Handler for ctrl+c
 */
void handleSIGINT(int signal){
    char *strExit = (char*)calloc(PKG_SIZE, sizeof(char));
    strcpy(strExit, "/exit");
    SockIO_send(iSocketFD, strExit);
    printf("Goodbye!\n");
    free(strExit);

    exit(EXIT_SUCCESS);
}

/**
 * Handles the server connection by printing what the server sends back to the clients.
 */
void handleServerConnection(void *pvArg){
    int iNoOfPackages   = 0;
    char *strBuffer     = (char*)calloc(PKG_SIZE, sizeof(char));

    while(1){
        if(read(iSocketFD, &iNoOfPackages, 1) < 0){
            printf("Error\n");
            exit(EXIT_FAILURE);
        }
        
        char *strReceivedMsg = (char*)calloc(iNoOfPackages * PKG_SIZE, sizeof(char));
        while((read(iSocketFD, strBuffer, PKG_SIZE)) > 0){
            strcat(strReceivedMsg, strBuffer);
            iNoOfPackages--;

            if(iNoOfPackages == 0){
                if(strcmp(strReceivedMsg, "/invalid") == 0){
                    printf("Username or password incorrect\n");
                    exit(EXIT_FAILURE);
                }
                else{
                    printf("%s", strReceivedMsg);
                }
                break;                
            }
        }
        free(strReceivedMsg);
    }
}

/**
 * ARGV[1] = Server IP;
 * ARGV[2] = Server PORT;
 **/
int main(int argc, char **argv){

    if(argc != 3){
        printf("Invalid input. Use: ./client <server ip address> <port>\n");
        exit(EXIT_FAILURE);
    }
                     
    struct sockaddr_in  localAddr, remoteAddr;
    pthread_t           threadID;
    int                 iPort;
    struct sigaction    sigInterrupt;

    bool  boIsAuthenticated  = false;
    char *strServerAddress   = (char*)calloc(SERVER_IP_SIZE, sizeof(char));
    
    strcpy(strServerAddress, argv[1]);
    iPort = atoi(argv[2]);

    sigInterrupt.sa_handler = handleSIGINT;
    sigInterrupt.sa_flags   = 0;

    if(sigaction(SIGINT, &sigInterrupt, NULL) == -1){
        printf("Error - sigint could not be handled\n");
        exit(EXIT_FAILURE);
    }

    if((iSocketFD = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        printf("%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    localAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    localAddr.sin_port         = 0;
    localAddr.sin_family       = AF_INET;

    if(bind(iSocketFD, (struct sockaddr *)&localAddr, sizeof(localAddr)) == -1){
        printf("%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    remoteAddr.sin_addr.s_addr = inet_addr(strServerAddress);
    remoteAddr.sin_port        = htons(iPort);
    remoteAddr.sin_family      = AF_INET;

    if(connect(iSocketFD, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) == -1){
        printf("CONNECTION\n");
        printf("%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("CONNECTED TO %s:%d\n", strServerAddress, iPort);

    if(pthread_create(&threadID, NULL, (void *)handleServerConnection, &iSocketFD) != 0){
        printf("Thread: %s\n", strerror(errno));
    }

    while(1){

        if(!boIsAuthenticated){
            char *strUsername       = (char*)calloc(PKG_SIZE, sizeof(char));
            char *strPassword       = (char*)calloc(PASSWORD_MAX_SIZE, sizeof(char));      
            
            /*get the username and passwords from users*/
            printf("Username:"); scanf("%[^\n]s", strUsername);
            while((getchar()) != '\n');
            printf("Password:"); scanf("%[^\n]s", strPassword);
            while((getchar()) != '\n');
            
            /*send the username and password to the server*/
            SockIO_send(iSocketFD, strUsername);
            SockIO_send(iSocketFD, strPassword);
            boIsAuthenticated = true;

            free(strUsername);
            free(strPassword);
        }
        /*get the message from the user*/
        char *strInput           = (char*)calloc(ALLOWED_DATA_LENGTH, sizeof(char));
        scanf("%[^\n]s", strInput);
        while((getchar()) != '\n');

        /*send the message to the server*/
        SockIO_send(iSocketFD, strInput);
        if(strcmp(strInput, "/exit") == 0){
            printf("Goodbye!\n");
            break;
        }
        free(strInput);
    }

    free(strServerAddress);
    close(iSocketFD);
}
