#include "remote-char.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

int main()
{	
    int fd;
    char str[2];
    message_t msg;

     //TODO_4
	// create and open the FIFO for writing
    while((fd= open(FIFO_PATH, O_WRONLY))== -1){
	  if(mkfifo(FIFO_PATH, 0666)!=0){
		    printf("problem creating the fifo_lab3\n");
			exit(-1);
	  }else{
            printf("fifo_lab3 created\n");
	  }
	}

    printf("FIFO communicating\n");

    //TODO_5
    printf("choose the character for the machine:");
	fgets(str, 2, stdin);

    // TODO_6
    // send connection message
    msg.type = CONNECT;
    msg.character = str[0];
    write(fd, &msg, sizeof(message_t));

    

    int sleep_delay;
    direction_t direction;
    int n = 0;
    while (1)
    {
        sleep_delay = random()%700000;
        usleep(sleep_delay);
        direction = random()%4;
        n++;
        switch (direction)
        {
        case LEFT:
           printf("%d Going Left   ", n);
            break;
        case RIGHT:
            printf("%d Going Right   ", n);
           break;
        case DOWN:
            printf("%d Going Down   ", n);
            break;
        case UP:
            printf("%d Going Up    ", n);
            break;
        }
        //TODO_9
        // prepare the movement message
        msg.type = MOVE;
        msg.direction = direction;

        //TODO_10
        //send the movement message
        write(fd, &msg, sizeof(message_t));
    }

 
	return 0;
}