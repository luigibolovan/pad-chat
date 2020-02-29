#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  

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

int ValidateUser(char *userName, char *password, FILE *file)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *userData = concat(4, userName, " ", password, "\n");
 
    if(file == NULL)
    {
        perror("Failed to open the file");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, file)) != -1)
    {

        printf("%d\n", strcmp(line, userData));
        if(strcmp(line, userData) == 0)
        {
            return 1;
        }
    }

    if(line)
    {
        free(line);
    }

    return 0;
}



int main(int argc, char *argv[])
{
       FILE *fd;
       char *userName = "IonutBorozan";
       char *userPassword = "Boropass";
       int result = 404;
     

       fd = fopen("/home/bogdan/Desktop/fileTest.txt", "r");

       result = ValidateUser(userName, userPassword, fd);

       if(result == 404)
            printf("Fuck!!!\n");
        else if(result == 1)
            printf("The user is Ok\n");
        else if(result == 0)
            printf("User don't exist\n");
}