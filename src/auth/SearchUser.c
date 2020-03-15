#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  
#include "SearchUser.h"

static char* concat(int count, ...)
{
    va_list ap;
    int i;

    int len = 1;
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

int validateUser(char *userName, char *password, FILE *file)
{
    char *line = (char*)calloc(512, sizeof(char));
    size_t len = 0;
    int read;
    char *userData = concat(4, userName, " ", password, "\n");

    if(file == NULL)
    {
        perror("Failed to open the file");
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, file)) != -1)
    {
        if(strcmp(line, userData) == 0)
        {
            printf("Valid user\n");
            return 1;
        }
    }

    if(line)
    {
        free(line);
    }

    return 0;
}

