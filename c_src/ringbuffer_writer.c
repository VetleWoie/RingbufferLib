#include "ringbuffer.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_READ_LEN 30

static void usage(int argc, char **argv)
{
	fprintf(stderr,
	"Ringbuffer\n"
	"Pipe stdin to a ringbuffer file\n\n"
	"Usage:\n"
    "\t %s <filename> <size>\n"
	"\t\t<filename>\t\t Path to ringbuffer file location [string];\n"
	"\t\t<size>\t\t\t Size of ringbuffer in bytes [int > 0]\n",
    argv[0]
	);
}

static void error_msg(char *err){
    fprintf(stderr,"%s",err);
}


static volatile int keepRunning = 1;

void intHandler(int dummy){
    printf("\n\n Shutting down cleanly");
    keepRunning = 0;
}

int main(int argc, char **argv){

    ringbuffer_t *ring = NULL;

    signal(SIGINT, intHandler);

    char *err_msg;
    if(argc < 3){
        goto usage;
    }
    int size = atoi(argv[2]);
    if(size <= 0){
        sprintf(err_msg, "Error: Size has to be an integer greater than 0, got %d", size);
        goto err;
    }
    char *filename = argv[1];
    ring = init_ringbuffer(size,filename,'w');
    if(ring == NULL){
        sprintf(err_msg, "Unexpected error: Could not create ringbuffer");
        goto err;
    }
    
    char buf[MAX_READ_LEN];
    //Reopen STDIN as binary stream
    if(freopen(NULL, "rb", stdin) == NULL){
        sprintf(err_msg, "Unexpected error: Could not reopen stdin as binary stream");
        goto err;
    };
    while(keepRunning){
        ssize_t bytes_read = read(STDIN_FILENO,buf,MAX_READ_LEN);
        if(bytes_read < 0){
            sprintf(err_msg, "Error: Could not read stdin");
            goto err;
        }else if(buf[bytes_read-1] == EOF){
            printf("Got EOF shutting down");
            goto shutdown;
        }
        ringbuffer_write(ring, buf, bytes_read);
    }

    shutdown:
        destroy_ringbuffer(ring);
        return 1;
    usage:
        usage(argc, argv);
        return 1;
    err:
        if(ring != NULL){
            destroy_ringbuffer(ring);
        }
        error_msg(err_msg);
        usage(argc, argv);
        return 0;
}