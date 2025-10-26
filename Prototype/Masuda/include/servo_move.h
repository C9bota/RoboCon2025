# ifdef SERVO_MOVE_H
# define SERVO_MOVE_H

#include "servo_command.h"

struct ServoCommandInfo {
    SERVO_COMMAND command;
    int send_buffer_size;
    int receive_buffer_size;
};


#endif