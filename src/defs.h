#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "message.h"

#define PID_AD_FILE "/tmp/game_server.pid"
#define SERVER_FIFO "/tmp/game_server.fifo"
#define FIFO_DIR "/tmp/game_server"

#ifdef DEBUG
#define dperror(a) perror(a)

#else
#define dperror(a)

#endif