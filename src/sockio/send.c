#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "send.h"

#define PKG_SIZE 255

/**
 * 
 * */
int SockIO_send(int fd, char *buf){
    
    int buf_size    = strlen(buf);
    int pos         = 0;
    int no_of_pkgs  = 0;
    char *pack      = (char*)calloc(PKG_SIZE, sizeof(char));

    if(buf_size == PKG_SIZE){
        if(write(fd, buf, PKG_SIZE) < 0){
            printf("Error @ writing: %s\n", strerror(errno));
            return -1;
        }
    }
    else if(buf_size < PKG_SIZE){
        for(int i = 0; i < PKG_SIZE - buf_size; i++){
            *(buf + buf_size + i) = '\0';
        }
        
        if(write(fd, buf, PKG_SIZE) < 0){
            printf("Error @ writing: %s\n", strerror(errno));
            return -1;
        }
    }
    else{
        /* check how many full packages do we have */
        no_of_pkgs = buf_size / PKG_SIZE;

        /* write the full packages */
        for(int i = 0; i < no_of_pkgs; i++){
            if(write(fd, buf + PKG_SIZE * i, PKG_SIZE) < 0){
                printf("Error @ writing: %s\n", strerror(errno));
                return -1;
            }
            buf_size -= PKG_SIZE;
        }
        /* check if any other chars remain after writing the packages*/
        if(buf_size > 0){
            strcpy(pack, (buf + no_of_pkgs * PKG_SIZE));
            for(int i = 0; i < PKG_SIZE - buf_size; i++){
                *(pack + buf_size + i) = '\0';
            }

            if(write(fd, pack, PKG_SIZE) < 0){
                printf("Error @ writing: %s\n", strerror(errno));
                return -1;
            }
            free(pack);
        }
    }
    return 0;
}
