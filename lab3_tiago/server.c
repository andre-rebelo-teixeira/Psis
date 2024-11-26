
#include <ncurses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>

#include "remote-char.h"


#define WINDOW_SIZE 15

direction_t random_direction(){
    return  random()%4;
}
void new_position(int* x, int *y, direction_t direction){
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
    int fd;
    message_t msg;
    int n;
    player_t players[10];
    int num_players = 0;

	// TODO_3
    // create and open the FIFO for reading
    while((fd = open(FIFO_PATH, O_RDONLY))== -1){
	  if(mkfifo(FIFO_PATH, 0666)!=0){
			printf("problem creating the fifo_lab3\n");
			exit(-1);
	  }else{
		  printf("fifo_lab3 created\n");
	  }
	}

    printf("FIFO communicating\n");
    
    // ncurses initialization
	initscr();	
	cbreak();
    keypad(stdscr, TRUE); 
	noecho();


    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);

    direction_t  direction;

    while (1)
    {
        // TODO_7
        // receive message from the clients
        n = read(fd, &msg, sizeof(msg));
        if (n <= 0) {
            perror("read");
            printf("Error reading from FIFO\n");
            break;
        }
        
        //TODO_8
        // process connection messages
        if(msg.type == CONNECT){
            players[num_players].character = msg.character;
            players[num_players].pos_x = WINDOW_SIZE/2;
            players[num_players].pos_y = WINDOW_SIZE/2;
            direction = random_direction();
            new_position(&players[num_players].pos_x, &players[num_players].pos_y, direction);
            /* draw mark on new position */
            wmove(my_win, players[num_players].pos_x, players[num_players].pos_y);
            waddch(my_win,players[num_players].character| A_BOLD);
            wrefresh(my_win);
            num_players++;
        }
        
        // TODO_11
        // process the movement message
        if(msg.type == MOVE){
            direction = msg.direction;
            for (int i = 0; i < num_players; i++){
                if(players[i].character == msg.character){
                    new_position(&players[i].pos_x, &players[i].pos_y, direction);
                    /* draw mark on new position */
                    wmove(my_win, players[i].pos_x, players[i].pos_y);
                    waddch(my_win,players[i].character| A_BOLD);
                    wrefresh(my_win);
                }
            }
        }			
    }

  	endwin();			/* End curses mode		  */

	return 0;
}