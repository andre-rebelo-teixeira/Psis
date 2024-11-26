
#include <complex.h>
#include <ncurses.h>
#include "remote-char.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>  
#define WINDOW_SIZE 15

int create_fifo() {
    int fd = -1;
    
    while((fd = open(FIFO_LOCATION, O_RDONLY)) == -1) {
        if (mkfifo(FIFO_LOCATION, 0666) != 0) {
            perror("Error creating FIFO");
            exit(EXIT_FAILURE);
        } else {
            printf("FIFO created\n");
        }
    }

    return fd;
}

enum DIRECTION random_direction(){
    return  random()%4;

}
void new_position(int* x, int *y, enum DIRECTION direction){
    switch (direction)
    {
    case UP:
        (*x) --;
        if(*x ==0)
            *x = 2;
        break;
    case DOWN:
        (*x) ++;
        if(*x ==WINDOW_SIZE-1)
            *x = WINDOW_SIZE-3;
        break;
    case LEFT:
        (*y) --;
        if(*y ==0)
            *y = 2;
        break;
    case RIGHT:
        (*y) ++;
        if(*y ==WINDOW_SIZE-1)
            *y = WINDOW_SIZE-3;
        break;
    default:
        break;
    }
}

int main()
{	
	// TODO_3
    // create and open the FIFO for reading
    int fd = create_fifo();


    // ncurses initialization
	initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();			    


    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    /* information about the character */
    int ch;
    int pos_x;
    int pos_y;



    enum DIRECTION  direction;



    while (1)
    {
        // TODO_7
        // receive message from the clients
        message m;
        if (read(fd, &m, sizeof(message)) == -1) {
            perror("Error reading from FIFO");
            exit(EXIT_FAILURE);
        }

        if (m.msg_type == CONNECTION) {
            ch = m.selected_charater;
            pos_x = WINDOW_SIZE/2;
            pos_y = WINDOW_SIZE/2;
        }




        
        //TODO_8
        // process connection messages

        // TODO_11
        // process the movement message
        
        /* draw mark on new position */
        wmove(my_win, pos_x, pos_y);
        waddch(my_win,ch| A_BOLD);
        wrefresh(my_win);			
    }
  	endwin();			/* End curses mode		  */

	return 0;
}