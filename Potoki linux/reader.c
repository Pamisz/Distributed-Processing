#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "config.h"

int main(){
    int state = mkfifo(FIFO_PATH, 0666);
    if (state == -1){
        if(errno != EEXIST){
            perror("Cannot create FIFO!");
            return 1;
        }
    }
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1){
        perror("FIFO was created, but cannot be opened for reading!");
        return 2;
    }
    char buf[BUFFER_SIZE];
    while(1){
        read(fd, buf, BUFFER_SIZE);
        printf("Received: %s\n", buf);
        if(!strcmp(buf, "quit")) {
            break;
        }
    }
    close(fd);
    unlink(FIFO_PATH);
    return 0;
}