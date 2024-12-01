// TODO_1 
// declaration the struct corresponding to the exchanged messages
enum message_type {
    CONNECTION = 0, 
    MOVEMENT
};

enum DIRECTION {
    LEFT = 0,
    RIGHT,
    DOWN,
    UP
};

typedef struct message {
    enum message_type msg_type; 
    char selected_charater;

    // This is keepted as a union not to save save space, but for possible expansion for future messages where extra data may be needed
    union {
        enum DIRECTION direction;
    }data;
}message;

// TODO_2
//declaration of the FIFO location
#define FIFO_LOCATION "/tmp/fifo"