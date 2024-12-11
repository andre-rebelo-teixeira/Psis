#include <ncurses.h>

#include "utils.h"

void draw_border_with_numbers() {
    // Enable the desired color pair for the border
    attron(COLOR_PAIR(4));
    char border_number[] = "01234567890123456789";
    char vertical_border[] = "+--------------------+";

    // Top border with column numbers
    mvprintw(0, 2, "%s", border_number); // Print column numbers above the top border
    mvprintw(1, 1, "%s", vertical_border);                // Top-left corner

    // Left and right borders with row numbers
    for (unsigned int row = 0; row < GRID_SIZE; row++) {
        mvprintw(row + 2, 0, "%c", border_number[row]); // Print row numbers to the left
        mvprintw(row + 2, 1, "|");                     // Draw left border
        mvprintw(row + 2, GRID_SIZE + 2, "|");         // Draw right border
    }

    // Bottom border with column numbers
    mvprintw(GRID_SIZE + 2, 1, "%s", vertical_border); // Bottom border line
    mvprintw(GRID_SIZE + 3, 2, "%s", border_number); // Print column numbers below the bottom border

    // Turn off the color pair
    attroff(COLOR_PAIR(4));
}

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    curs_set(0); // Hide cursor
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Astronauts
    init_pair(2, COLOR_RED, COLOR_BLACK);   // Aliens
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);// Laser beams
    init_pair(4, COLOR_WHITE, COLOR_BLACK); // Borders and text
}