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

#define CLI_EXT "_cli"
#define LENGTH_OF_CLI_EXT 4
#define PID_BUFFER 16

#define LENGTH_OF_PATH_DIR 16
#define LENGTH_OF_CLIENT_FIFOS 64
#define LENGTH_OF_PID_BUFFER 16
#define SERV_EXT "_serv"
#define LENGTH_OF_SERV_EXT 5
#define FIFO_MODE 0755

#define SERV_PID_MODE 0664
#define DIR_MODE 0755
#define SERVER_PID_SIZE 16

// SERVER DEFS

/*
	Handler of SIGUSR1, initialize a volatile variable named usr1_recieve.
*/

void handSIGUSR1(int sig);

/*
	Handler of SIGINT, SIGQUIT, SIGTERM, write in the standard output,
	the signal that closed the program. Then call exit(3).
*/

void handENDSRV(int sig);

/*
	Create the file "/tmp/game_server.pid", to store the current PID. Create
	the FIFO "/tmp/game_server.fifo" to initialize connection with the
	clients and create the directory "/tmp/game_server". 
*/

void initialization(void);

/*
	Define the actions of SIGUSR1, SIGINT, SIGQUIT, SIGTERM.
*/

void define_signals_actions(void);

/*
	Create the 2 FIOS in the file "/tmp/game_server".
*/

void create_fifo_for_client(pid_t clientPID);

/*
	Call kill and send SIGUSR1 to client.
*/

void send_sigusr1_to_client(pid_t clientPID);

/*
	Call kill and send SIGUSR2 to client.
*/

void send_sigusr2_to_client(pid_t clientPID);

/*
	Free the args table (table of arguments to transmit to the games).
*/

void free_tabs(void);

/*
	Delete all the games successively.
*/

void delete_childs(void);

/*
	Remove the 2 FIFOS in "/tmp/game_server" linked to the clientPID in parameter.
*/

void remove_fifos(pid_t clientPID);

/*
	Delete all the files created in "void initialization(void);".
*/

void deinitialization(void);

/*
	Get the information in a char ** after receiving SIGUSR1 from client.
*/

void get_client_informations(pid_t *clientPID);

/*
	Check if the game is accessible.
*/

bool game_exists(char *game);

/*
	Edit the table args by adding "_serv" at the end.
*/

bool edit_table(void);

/*
	Create a child and cover the process.
*/

pid_t start_game(void);

/*
	Interpret the end of the game after calling wait(2).
*/

void interpret_end_of_game(const int childEnding, const pid_t childPID);

// SERVER DEFS

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