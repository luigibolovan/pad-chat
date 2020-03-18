#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "send.h"

#define PKG_SIZE 256

/**
 * Function that handles the sending of messages
 **/
int SockIO_send(int fd, char *strBuffer){
    
    int iBufferSize         = strlen(strBuffer);
    int iNoOfFullPackages   = 0;
    char *strPack           = (char*)calloc(PKG_SIZE, sizeof(char));
    int iNoOfPackages       = 0;
    iNoOfPackages           = iBufferSize / PKG_SIZE + ((iBufferSize % PKG_SIZE) ? 1 : 0);

    write(fd, &iNoOfPackages, 1);

    if(iBufferSize == PKG_SIZE){
        if(write(fd, strBuffer, PKG_SIZE) < 0){
            printf("Error @ writing: %s\n", strerror(errno));
            return -1;
        }
    }
    else if(iBufferSize < PKG_SIZE){
        for(int i = 0; i < PKG_SIZE - iBufferSize; i++){
            *(strBuffer + iBufferSize + i) = '\0';
        }
        
        if(write(fd, strBuffer, PKG_SIZE) < 0){
            printf("Error @ writing: %s\n", strerror(errno));
            return -1;
        }
    }
    else{
        /* check how many full packages do we have */
        iNoOfFullPackages = iBufferSize / PKG_SIZE;

        /* write the full packages */
        for(int i = 0; i < iNoOfFullPackages; i++){
            if(write(fd, strBuffer + PKG_SIZE * i, PKG_SIZE) < 0){
                printf("Error @ writing: %s\n", strerror(errno));
                return -1;
            }
            iBufferSize -= PKG_SIZE;
        }
        /* check if any other chars remain after writing the packages*/
        if(iBufferSize > 0){
            strcpy(strPack, (strBuffer + iNoOfFullPackages * PKG_SIZE));
            for(int i = 0; i < PKG_SIZE - iBufferSize; i++){
                *(strPack + iBufferSize + i) = '\0';
            }

            if(write(fd, strPack, PKG_SIZE) < 0){
                printf("Error @ writing: %s\n", strerror(errno));
                return -1;
            }
            free(strPack);
        }
    }
    return 0;
}
