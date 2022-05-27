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
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>

#include "message.h"

/////////////////////////////////////////////////////////////////////////////

#define PID_AD_FILE "/tmp/game_server.pid"
#define SERVER_FIFO "/tmp/game_server.fifo"
#define FIFO_DIR "/tmp/game_server"
#define FIFO_MODE 0755

/////////////////////////////////////////////////////////////////////////////

#define PATH_PID_FILE_SERVER "/tmp/game_server.pid"
#define PATH_FIFO_GAME_SERVER "/tmp/game_server.fifo"
#define PATH_DIR_GAME_SERVER "/tmp/game_server"

#define LENGTH_OF_PATH_DIR 16
#define LENGTH_OF_CLIENT_FIFOS 64
#define LENGTH_OF_PID_BUFFER 16
#define SERV_EXT "_serv"
#define LENGTH_OF_SERV_EXT 5

#define SERV_PID_MODE 0664
#define DIR_MODE 0755
#define SERVER_PID_SIZE 16

void handSIGUSR1(int sig);
void handENDSRV(int sig);
void initialization(void);
void define_signals_actions(void);
void create_fifo_for_client(pid_t clientPID);
void send_sigusr1_to_client(pid_t clientPID);
void send_sigusr2_to_client(pid_t clientPID);
void free_tabs(void);
void delete_childs(void);
void remove_fifos(pid_t clientPID);
void deinitialization(void);
void get_client_informations(pid_t *clientPID);
bool game_exists(char *game);
bool edit_table(void);
pid_t start_game(void);
void interpret_end_of_game(const int childEnding, const pid_t childPID);

#ifdef DEBUG
#define dperror(a) perror(a)

#else
#define dperror(a)

#endif