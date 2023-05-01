#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"

struct ringbuffer{
    int head;
    int maxSize;
    char *path;
    int fd;
    void *buf;
};

typedef struct ringbuffer ringbuffer_t;

ringbuffer_t *init_ringbuffer(int maxSize, char *path){
    ringbuffer_t *ringbuffer;

    //Allocate new ringbuffer
    ringbuffer = (ringbuffer_t *) malloc(sizeof(ringbuffer_t));
    if(ringbuffer == NULL){
        return NULL;
    }
    ringbuffer->head = 0;
    ringbuffer->maxSize = maxSize;
    ringbuffer->path = path;

    //Allocate memory for the ringbuffer
    ringbuffer->buf = calloc(1, maxSize);
    if(ringbuffer->buf == NULL){
        return NULL;
    }

    //If path is NULL then we do not need create a file
    if(path != NULL){
        //Open new file with read write access, create new one if file does not exist
        ringbuffer->fd = open(path, O_RDWR | O_CREAT, 0666);
        if(ringbuffer->fd < 0){
            free(ringbuffer->buf);
            return NULL;
        }
        //Fill the file up to maxsize with zeros
        if(write(ringbuffer->fd,ringbuffer->buf,maxSize)<0){
            free(ringbuffer->buf);
            return NULL;
        }
        //We're gonna map our memory to the new file so now we doesn't need the memory we allocated earlier
        free(ringbuffer->buf);
        //Map memory to file 
        ringbuffer->buf = mmap(NULL, maxSize, PROT_WRITE, MAP_SHARED, ringbuffer->fd,0);
        if(ringbuffer->buf == MAP_FAILED){
            return NULL;
        }
    }
    return ringbuffer;
}

int destroy_ringbuffer(ringbuffer_t *ringbuffer){
    if(ringbuffer->path != NULL){
        if(munmap(ringbuffer->buf, ringbuffer->maxSize) < 0){
            return -1;
        }
    }else{
        free(ringbuffer->buf);
    }
    free(ringbuffer);
    return 0;
}

void ringbuffer_write(ringbuffer_t *ringbuffer, const void *buf, size_t __n){
    //Find how much place is left in the buffer
    size_t size_left = ringbuffer->maxSize - ringbuffer->head;
    //Get the memory position of the head position
    void *curr_pos = &((char *) ringbuffer->buf)[ringbuffer->head];

    if(__n <= size_left){
        //If its more space left than we need to write, then write and move head
        memcpy(curr_pos, buf, __n);
        ringbuffer->head += __n;
    }else{
        //Write to end of ring
        memcpy(curr_pos, buf, size_left);
        //Find memory position of remaining buffer data
        curr_pos = &((char *) buf)[size_left];
        //Set head to start of ring
        ringbuffer->head = 0; 
        //Write remaining buffer data to the ring
        //Do this recursivley. PS: Might be faster to do it iterativley depending on the compiler.
        ringbuffer_write(ringbuffer, curr_pos, __n-size_left);
    }
}

void ringbuffer_read(ringbuffer_t *ringbuffer, void *buf, size_t __n){
    //Find out how much is left to read in the buffer
    size_t size_left = ringbuffer->maxSize - ringbuffer->head;
    //Get the memory position of the head position
    void *curr_pos = &((char *) ringbuffer->buf)[ringbuffer->head];

    if(__n <= size_left){
        //If its more space left than we want to read, then read and move head
        memcpy(buf, curr_pos, __n);
        ringbuffer->head += __n;
    }else{
        //Read to end of ring
        memcpy(buf, curr_pos, size_left);
        //Find memory position of remaining buffer data
        curr_pos = &((char *) buf)[size_left];
        //Set head to start of ring
        ringbuffer->head = 0; 
        //Read remaining buffer data to the ring
        //Do this recursivley. PS: Might be faster to do it iterativley depending on the compiler.
        ringbuffer_write(ringbuffer, curr_pos, __n-size_left);
    }
}