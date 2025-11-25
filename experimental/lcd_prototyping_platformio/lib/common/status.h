#ifndef STATUS_H
#define STATUS_H

typedef enum
{
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_BUSY = 2,
    STATUS_TIMEOUT = 3,
} Status;

#endif // STATUS_H