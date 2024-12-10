typedef enum move_t {UP, DOWN, LEFT, RIGHT} move_t; // Possible movements
typedef enum msg_t {
    ASTRONAUT_CONNECT,
    ASTRONAUT_MOVEMENT,
    ASTRONAUT_ZAP,
    ASTRONAUT_DISCONNECT, 
    OUTER_SPACE_UPDATE,
    RESPONSE
} msg_t; // Types of messages

typedef struct message
{
    msg_t type; // Type of message
    move_t move; // Movement (for astronaut_movement messages)
    char character; // Character of the player (for astronaut_disconnect messages)
} message;


