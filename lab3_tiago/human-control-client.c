#include <ncurses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
 #include <ctype.h> 
 #include <stdlib.h>

 #include "remote-char.h"

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
    // read the character from the user
    printf("choose your character:");
	fgets(str, 2, stdin);

    // TODO_6
    // send connection message
    msg.type = CONNECT;
    msg.character = str[0];
    write(fd, &msg, sizeof(message_t));

	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */

    
    int ch;

    int n = 0;
    do
    {
    	ch = getch();		
        n++;
        switch (ch)
        {
            case KEY_LEFT:
                mvprintw(0,0,"%d Left arrow is pressed", n);
                msg.direction = LEFT;
                break;
            case KEY_RIGHT:
                mvprintw(0,0,"%d Right arrow is pressed", n);
                msg.direction = RIGHT;
                break;
            case KEY_DOWN:
                mvprintw(0,0,"%d Down arrow is pressed", n);
                msg.direction = DOWN;
                break;
            case KEY_UP:
                mvprintw(0,0,"%d :Up arrow is pressed", n);
                msg.direction = UP;
                break;
            default:
                ch = 'x';
                    break;
        }
        refresh();			/* Print it on to the real screen */

        //TODO_9
        // prepare the movement message
        msg.type = MOVE;

        //TODO_10
        //send the movement message
        write(fd, &msg, sizeof(message_t));
        
    }while(ch != 27);
    
    
  	endwin();			/* End curses mode		  */

	return 0;
}