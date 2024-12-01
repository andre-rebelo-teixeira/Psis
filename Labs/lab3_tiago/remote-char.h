// TODO_1 
// declaration the struct corresponding to the exchanged messages
typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;
typedef enum type_t {CONNECT, MOVE} type_t;

typedef struct message_t {
    type_t type;
    char character;
    direction_t direction;
} message_t;

typedef struct player_t {
    char character;
    int pos_x;
    int pos_y;
} player_t;

// TODO_2
//declaration of the FIFO location
#define FIFO_PATH "/tmp/fifo_lab3"