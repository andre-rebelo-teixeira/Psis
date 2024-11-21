#include <cstdlib>
#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
 #include <ctype.h> 
 #include <stdlib.h>
 
int create_fifo() {
    int fd = -1;
    
    while((fd = open(FIFO_LOCATION,O_WRONLY)) == -1) {
        if (mkfifo(FIFO_LOCATION, 0666) != 0) {
            perror("Error creating FIFO");
            exit(EXIT_FAILURE);
        } else {
            printf("FIFO created\n");
        }
    }

    return fd;
}

int main()
{
    //TODO_4
    // create and open the FIFO for writing
    int fd = create_fifo();

    //TODO_5
    // read the character from the user
    char c; 
    printf("Enter the character: ");
    if (scanf("%c", &c) != 1) {
        perror("Error reading character");
        exit(EXIT_FAILURE);
    }


    // TODO_6
    // send connection message
    message m;
    m.msg_type = CONNECTION;
    m.selected_charater = c;

    if (write(fd, &m, sizeof(message)) == -1) {
        perror("Error writing to FIFO");
        exit(EXIT_FAILURE);
    }

	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */

    
    int ch;

    m.msg_type = 1;
    int n = 0;
    do
    {
    	ch = getch();		
        n++;
        switch (ch)
        {
            case KEY_LEFT:
                mvprintw(0,0,"%d Left arrow is pressed", n);
                break;
            case KEY_RIGHT:
                mvprintw(0,0,"%d Right arrow is pressed", n);
                break;
            case KEY_DOWN:
                mvprintw(0,0,"%d Down arrow is pressed", n);
                break;
            case KEY_UP:
                mvprintw(0,0,"%d :Up arrow is pressed", n);
                break;
            default:
                ch = 'x';
                    break;
        }
        refresh();			/* Print it on to the real screen */
        //TODO_9
        // prepare the movement message
        

        //TODO_10
        //send the movement message
        
    }while(ch != 27);
    
    
  	endwin();			/* End curses mode		  */

	return 0;
}