#ifndef MESSAGES_H
#define MESSAGES_H

typedef enum move_t {UP, DOWN, LEFT, RIGHT} move_t; // Possible movements

typedef enum msg_t {
    ASTRONAUT_CONNECT,
    ASTRONAUT_MOVEMENT,
    ASTRONAUT_ZAP,
    ASTRONAUT_DISCONNECT, 
    OUTER_SPACE_UPDATE,
    RESPONSE, 
    TICK
} msg_t; // Types of messages

typedef struct message
{
    msg_t type; // Type of message
    move_t move; // Movement (for movement messages)
    char character; // Character of the player (for connect and disconnect messages)
    char current_players[8]; // Current players in the game (for outer space update messages)
    unsigned int scores[8]; // Scores of the players (for outer space update messages)
    char grid[20][20]; // Grid of the game (for outer space update messages)
    bool game_over; // Game over flag (for outer space update messages)
} message;

#endif