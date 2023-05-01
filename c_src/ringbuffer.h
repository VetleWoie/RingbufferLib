#ifndef RINGBUFFERH
#define RINGBUFFERH

#include <string.h>

typedef struct ringbuffer ringbuffer_t;


/*
Initilizes a new ringbuffer.
@Params
    - int maxSize:
        The size in bytes of the ringbuffer
    - char *path:
        Path to file where the ringbuffer should be mapped.
        If NULL no file is created. If file exists the it will override the contents
        of the file. If not then it will create a new file.

@Returns
    - NULL on failure, pointer to new ringbuffer on success
*/
ringbuffer_t *init_ringbuffer(int maxSize, char *path, char *mode);

/*
Deallocates any memory used by the ringbuffer. If ringbuffer is mapped to a file it will not touch the 
content of that file.

@Params
    - ringbuffer_t *ringbuffer:
        Pointer to ringbuffer to be destroyed

@Returns
    - 0 on success and -1 failure.
*/
int destroy_ringbuffer(ringbuffer_t *ringbuffer);

/*
Write __n bytes of buf to ringbuffer.

@Paramaters
    - ringbuffer_t *ringbuffer:
        Pointer to ringbuffer to be written to
    - const void *buf:
        Buffer of data to be written to ringbuffer
    - size_t __n:
        Amount of data in bytes to write from buf to ringbuffer
*/
void ringbuffer_write(ringbuffer_t *ringbuffer, const void *buf, size_t __n);
/*
Read __n bytes of ringbuffer into buf.

@Paramaters
    - ringbuffer_t *ringbuffer:
        Pointer to ringbuffer to be read from
    - const void *buf:
        Buffer to hold data read from ringbuffer, has to be allocated memory of size __n
    - size_t __n:
        Amount of data in bytes to read from ringbuffer to buf
*/
void ringbuffer_read(ringbuffer_t *ringbuffer, void *buf, size_t __n);

#endif