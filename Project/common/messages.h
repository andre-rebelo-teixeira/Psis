#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdbool.h>

/**
 *
 * This file contains the definition of the messages that will be sent between the client and server 
 * 
 * This file is share amoung all the components of the system. This means only minimal information should be included here
 */


/**
 * @brief This enum represents the possible types of messages that the client will send to the server
 * 
 * ASTRONAUT_CONNECT: The client is connecting to the server
 * ASTRONAUT_MOVEMENT: The client is moving
 * ASTRONAUT_ZAP: The client is zapping
 * ASTRONAUT_DISCONNECT: The client is disconnecting
 * TICK: This client is sending a tick to run the game loop logic. This needs to be sent periodically for the game to run smoothly
 */
typedef enum msg_t
{
    ASTRONAUT_CONNECT,
    ASTRONAUT_MOVEMENT,
    ASTRONAUT_ZAP,
    ASTRONAUT_DISCONNECT,
    TICK,
    LAST_TICK
} msg_t; // Types of messages

/**
 * @brief This enum represents the possible movements that a player can make
 * 
 * UP: The player moves up
 * DOWN: The player moves down
 * LEFT: The player moves left
 * RIGHT: The player moves right
 * 
 */
typedef enum move_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} move_t; 

/**
 * @brief This struct represents a message that will be sent from the client to the server
 * 
 * type: The type of message
 * move: The move that the player is making
 * character: The character that the player is controlling
 * 
 */
typedef struct
{
    msg_t type;
    move_t move;
    char character;
} client_to_server_message;

/**
 * @brief This struct represents a message that will be sent from the server to the client
 * 
 * character: The character that the player is controlling 
 * game over: A boolean that is true if the game has ended -> This will mean the astronauts can no longer move or shot
 * score: The score of the player
 * status: A boolean that is true if the message received was accepted by the server or not
 */
typedef struct
{
    char character;
    bool game_over;
    unsigned int score;
    bool status;
} server_to_client_message;

/**
 * @brief This struct represents a message that will be sent from the server to update the display of each client
 * 
 * server_shutdown: A boolean that is true if the server is shutting down
 * grid: The grid that will be displayed to the client
 * scores: The scores of all the players
 * current_players: The characters of the current players
 */
typedef struct
{
    bool server_shutdown;
    bool game_over;
    char grid[20][20];
    unsigned int scores[8];
    char current_players[8];
} display_update_message;

#endif