#define _DEFAULT_SOURCE

#include "message.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define PATH_PID_FILE_SERVER "/tmp/game_server.pid"
#define PATH_FIFO_GAME_SERVER "/tmp/game_server.fifo"
#define PATH_DIR_GAME_SERVER "/tmp/game_server"

#define LENGTH_OF_CLIENT_FIFOS 64

#define PID_BUFFER 16

#define CLI_EXT "_cli"
#define LENGTH_OF_CLI_EXT 4

// CLIENT DEFS

/*
	Handler of SIGUSR1, initialize a volatile variable named usr1_recieve.
*/

void handSIGUSR1(int sig);

/*
	Handler of SIGUSR1, call exit(3).
*/

void handSIGUSR2(int sig);

/*
	Check if the game is accessible.
*/

bool game_exists(char *game);

/*
	Read the server PID in the file "/tmp/game_server.pid".
*/

pid_t read_server_pid(void);

/*
	Define the actions of SIGUSR1 and SIGUSR2.
*/

void define_signals_actions(void);

/*
	Call kill and send SIGUSR1 to server.
*/

void send_sigusr1_to_server(void);

/*
	Send argv to the server.
*/

void send_client_informations(char **args);

/*
	Create fifos name to open it.
*/

void init_fifos(char *clientFifo0Buffer, char *clientFifo1Buffer);

// CLIENT DEFS

#ifdef DEBUG
#define dperror(a) perror(a)

#else
#define dperror(a)

#endif
