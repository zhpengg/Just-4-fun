#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "srvmode.h"
#include "network.h"

#define LSN_ADDRESS "127.0.0.1"
#define LSN_PORT 12321

void echo_select()
{
    int lsnfd = open_socket(LSN_ADDRESS, LSN_PORT);
    if (lsnfd < 0) {
        return;
    }
}

void echo_poll()
{

}

void echo_epoll()
{

}
