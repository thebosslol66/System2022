#define _DEFAULT_SOURCE

#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

#define PATH_PID_FILE_SERVER "/tmp/game_server.pid"
#define PATH_FIFO_GAME_SERVER "/tmp/game_server.fifo"
#define PATH_DIR_GAME_SERVER "/tmp/game_server"

#define LENGTH_OF_PATH_DIR 16
#define LENGTH_OF_CLIENT_FIFOS 64
#define LENGTH_OF_PID_BUFFER 16
#define SERV_EXT "_serv"
#define LENGTH_OF_SERV_EXT 5

#define SERV_PID_MODE 0664
#define FIFO_MODE 0644
#define DIR_MODE 0755
#define SERVER_PID_SIZE 16

struct list_node {
  pid_t clientPID;
  pid_t serverPID;
  struct list_node *next;
};

struct list {
  struct list_node *first;
};

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


size_t list_size(const struct list *self);
size_t list_search_clientPID(const struct list *self, pid_t servPID, pid_t *cliPID);
void list_remove(struct list *self, size_t index);
void list_create(struct list *self);
void list_destroy(struct list *self);
void list_push_front(struct list *self, pid_t cliPID, pid_t servPID);


volatile int usr1_receive = 0;
static char **args = NULL;
static char clientFifo0Buffer[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo1Buffer[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo0BufferDel[LENGTH_OF_CLIENT_FIFOS];
static char clientFifo1BufferDel[LENGTH_OF_CLIENT_FIFOS];
static struct list pids;

void handSIGUSR1(int sig) {
	usr1_receive = 1;
}

void handENDSRV(int sig) {
	switch (sig) {
		case SIGINT: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGINT)\n");
			break;
		}
		case SIGQUIT: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGQUIT)\n");
			break;
		}
		case SIGTERM: {
			fprintf(stderr, "Server terminated due to signal interrupt (SIGTERM)\n");
			break;
		}
	}

	exit(2);
}

void handSIGCHLD(int sig) {
	int childEnding = 0;
	pid_t childTerminated = wait(&childEnding);
	interpret_end_of_game(childEnding, childTerminated);

	pid_t cliPID = 0;
	size_t index = list_search_clientPID(&pids, childTerminated, &cliPID);
	printf("INDEX : %zu\n", index);
	printf("SIZE : %zu\n", list_size(&pids));
	//list_remove(&pids, index);

	remove_fifos(cliPID);
	printf("ECRASE %d OF %d\n", cliPID, childTerminated);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t list_size(const struct list *self) {    
    struct list_node *current = self->first;
    size_t size = 0;

    while (current != NULL) {
        size++;
        current = current->next;
    }

    return size;
}

size_t list_search_clientPID(const struct list *self, pid_t servPID, pid_t *cliPID) {
    struct list_node *current = self->first;
    size_t counter = 0;

    while (current != NULL) {
        if (current->serverPID == servPID) {
            *cliPID = current->clientPID;
            return counter;
        }
        counter++;
        current = current->next;
    }

    return counter;
}

void list_remove(struct list *self, size_t index) {
    if (self == NULL || self->first == NULL) {
        return;
    }

    if (index == 0) {
        struct list_node *storage = self->first;

        if (self->first->next != NULL) {
            self->first = self->first->next;
        } else {
            self->first = NULL;
        }
        
        free(storage);
        return;
    }

    struct list_node *current = self->first;
    struct list_node *storage = NULL;    
    size_t counter = 0;

    while (counter != (index - 1)) {
        counter++;
        current = current->next;
    }

    storage = current->next;

    if (current->next->next == NULL) {
    	current->next = NULL;
    } else {
    	current->next = current->next->next;
    }

    free(storage);
}

void list_create(struct list *self) {
	self->first = NULL;
}

void list_destroy(struct list *self) {
	if (self == NULL || self-> first == NULL) {
        return;
    }

    struct list_node *storage = NULL;

    while (self->first != NULL) {
        storage = self->first;

        if (self->first->next != NULL) {
            self->first = self->first->next;
        } else {
            self->first = NULL;
        }
        
        free(storage);
    }
}

void list_push_front(struct list *self, pid_t cliPID, pid_t servPID) {
	if (self == NULL) {
		return;
	}

    struct list_node *node = malloc(sizeof(struct list_node));
    node->clientPID = cliPID;
    node->serverPID = servPID;

    if (self->first == NULL) {
        node->next = NULL;
        self->first = node;
        printf("\n\nSIZE : %p\n\n", self->first);
        return;
    }

    node->next = self->first;
    self->first = node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initialization(void) {
	list_create(&pids);

	int fdPidServer = open(PATH_PID_FILE_SERVER, O_CREAT | O_EXCL | O_WRONLY, SERV_PID_MODE);

	if (fdPidServer == -1) {
		fprintf(stderr, "Error while opening %s\nThe server may be already running\n", PATH_PID_FILE_SERVER);
		perror("open");
		exit(1);
	}

	pid_t serverPid = getpid();
	char stringServerPid[SERVER_PID_SIZE];

	sprintf(stringServerPid, "%d", serverPid);

	ssize_t retOfWrite = write(fdPidServer, stringServerPid, strlen(stringServerPid) * sizeof(char));

	if (retOfWrite == -1) {
		fprintf(stderr, "Error while writing the server pid\n");
		perror("write");
		exit(1);
	}

	int resClose = close(fdPidServer);
	
	if (resClose == -1) {
		fprintf(stderr, "Error while closing %s\n", PATH_PID_FILE_SERVER);
		perror("close");
		exit(1);
	}

	struct stat statFifo;
    int statFifoExists = stat(PATH_FIFO_GAME_SERVER, &statFifo);

    if (statFifoExists == -1) {
    	int resMkfifo = mkfifo(PATH_FIFO_GAME_SERVER, FIFO_MODE);

    	if (resMkfifo == -1) {
    		fprintf(stderr, "Error while creating FIFO %s\n", PATH_FIFO_GAME_SERVER);
			perror("mkfifo");
			exit(1);
    	}
    }

    int resMkdir = mkdir(PATH_DIR_GAME_SERVER, DIR_MODE);

    if (resMkdir == -1) {
    	fprintf(stderr, "Error while creating directory %s\n", PATH_DIR_GAME_SERVER);
		perror("mkdir");
		exit(1);
    }
}

void define_signals_actions(void) {
	struct sigaction actionUsr1;
	sigemptyset(&actionUsr1.sa_mask);
	actionUsr1.sa_handler = handSIGUSR1;
	actionUsr1.sa_flags = 0;
	sigaction(SIGUSR1, &actionUsr1, NULL);

	struct sigaction actionChld;
	sigemptyset(&actionChld.sa_mask);
	actionChld.sa_handler = handSIGCHLD;
	actionChld.sa_flags = 0;
	sigaction(SIGCHLD, &actionChld, NULL);

	struct sigaction actionEndServ;
	sigemptyset(&actionEndServ.sa_mask);
	actionEndServ.sa_handler = handENDSRV;
	actionEndServ.sa_flags = 0;
	sigaction(SIGINT, &actionEndServ, NULL);
	sigaction(SIGQUIT, &actionEndServ, NULL);
	sigaction(SIGTERM, &actionEndServ, NULL);
}

void create_fifo_for_client(pid_t clientPID) {
	sprintf(clientFifo0Buffer, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_0.fifo");

	int resMkfifo = mkfifo(clientFifo0Buffer, FIFO_MODE);

   	if (resMkfifo == -1) {
    	fprintf(stderr, "Error while creating FIFO %s\n", clientFifo0Buffer);
		perror("mkfifo");
		exit(1);
    }

    sprintf(clientFifo1Buffer, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_1.fifo");

    resMkfifo = mkfifo(clientFifo1Buffer, FIFO_MODE);

   	if (resMkfifo == -1) {
    	fprintf(stderr, "Error while creating FIFO %s\n", clientFifo1Buffer);
		perror("mkfifo");
		exit(1);
    }
}

void send_sigusr1_to_client(pid_t clientPID) {
	int retOfKill = kill(clientPID, SIGUSR1);

	if (retOfKill == -1) {
		fprintf(stderr, "Server error : can't send signal SIGUSR1 to client\n");
		perror("kill");
	}
}

void send_sigusr2_to_client(pid_t clientPID) {
	int retOfKill = kill(clientPID, SIGUSR2);

	if (retOfKill == -1) {
		fprintf(stderr, "Server error : can't send signal SIGUSR2 to client\n");
		perror("kill");
	}
}

void free_tabs(void) {
	list_destroy(&pids);

	if (args != NULL) {
		size_t counter = 0;

		while (args[counter] != NULL) {
			free(args[counter]);
			counter++;
		}

		free(args);
	}

	fprintf(stdout, "MEMORY FREE COMPLETED SUCCESSFULLY\n");
}

void delete_childs(void) {
	while (pids.first != NULL) {
		send_sigusr2_to_client(pids.first->clientPID);
	}
}

void remove_fifos(pid_t clientPID) {
	sprintf(clientFifo0BufferDel, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_0.fifo");

	int resUnlink = unlink(clientFifo0BufferDel);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", clientFifo0BufferDel);
		perror("unlink");
	}

	sprintf(clientFifo1BufferDel, "%s%s%d%s", PATH_DIR_GAME_SERVER, "/cli", clientPID, "_1.fifo");

	resUnlink = unlink(clientFifo1BufferDel);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", clientFifo1BufferDel);
		perror("unlink");
	}
}

void deinitialization(void) {
	int resUnlink = unlink(PATH_PID_FILE_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing %s\n", PATH_PID_FILE_SERVER);
		perror("unlink");
	}

	resUnlink = unlink(PATH_FIFO_GAME_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", PATH_FIFO_GAME_SERVER);
		perror("unlink");
	}

	DIR *dir = opendir(PATH_DIR_GAME_SERVER);

    if (dir != NULL) {
		struct dirent *info = readdir(dir);

		while (info != NULL) {
			if (info->d_name == NULL) {
				break;
			}

			if (info->d_name[0] != '.') {
				size_t sizeOfNameFifo = LENGTH_OF_PATH_DIR + strlen(info->d_name) + 2;
				char *nameOfFifo = calloc(sizeOfNameFifo, sizeof(char));
				sprintf(nameOfFifo, "%s%c%s", PATH_DIR_GAME_SERVER, '/', info->d_name);

				resUnlink = unlink(nameOfFifo);

				if (resUnlink == -1 && errno != ENOENT) {
					fprintf(stderr, "Error while removing FIFO %s\n", info->d_name);
					perror("unlink");
					free(nameOfFifo);
					exit(1);
				}

				free(nameOfFifo);
			}

			info = readdir(dir);
		}

		int retOfCosedir = closedir(dir);

		if (retOfCosedir == -1) {
			fprintf(stderr, "Error while closing directory %s\n", PATH_DIR_GAME_SERVER);
			perror("closedir");
		}
	}

	int resOfRmdir = rmdir(PATH_DIR_GAME_SERVER);

	if (resOfRmdir == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing directory %s\n", PATH_DIR_GAME_SERVER);
		perror("rmdir");
	}

	fprintf(stdout, "FILES SUCCESSFULLY REMOVED\n");
}

void get_client_informations(pid_t *clientPID) {
	int fdFifo = open(PATH_FIFO_GAME_SERVER, O_RDONLY);

	if (fdFifo == -1) {
		fprintf(stderr, "Error while opening %s\n", PATH_FIFO_GAME_SERVER);
		perror("open");
		exit(1);
	}

	char *clientPIDBuffer = recv_string(fdFifo);

	if (clientPIDBuffer == NULL) {
		fprintf(stderr, "Error while getting the client informations\n");
		free(clientPIDBuffer);
		return;
	}

	*clientPID = atoi(clientPIDBuffer);
	free(clientPIDBuffer);

	if (args != NULL) {
		free_tabs();
	}

	args = recv_argv(fdFifo);

	if (args == NULL) {
		fprintf(stderr, "Error while getting the client informations\n");
	}

	int resClose = close(fdFifo);
	
	if (resClose == -1) {
		fprintf(stderr, "Error while closing %s\n", PATH_FIFO_GAME_SERVER);
		perror("close");
		exit(1);
	}
}

bool game_exists(char *game) {
	return access(game, X_OK) != -1;
}

pid_t start_game() {
	pid_t pid = fork();

	switch (pid) {
		case -1: {
			fprintf(stderr, "Error while creating child to start game\n");
			perror("fork");
			exit(1);
		}
		case 0: {
			struct sigaction actionIgn;
			sigemptyset(&actionIgn.sa_mask);
			actionIgn.sa_handler = SIG_IGN;
			actionIgn.sa_flags = 0;
			sigaction(SIGINT, &actionIgn, NULL);
			sigaction(SIGQUIT, &actionIgn, NULL);
			sigaction(SIGTERM, &actionIgn, NULL);
			sigaction(SIGUSR1, &actionIgn, NULL);

			execv(args[0], args);

			perror("execv");
			_exit(3);
		}
		default: {
			return pid;
		}
	}
}

void interpret_end_of_game(const int childEnding, const pid_t childPID) {
	if (WIFEXITED(childEnding)) {
		printf("Game %d exited normally, status=%d\n", childPID, WEXITSTATUS(childEnding));
	} else if (WIFSIGNALED(childEnding)) {
		printf("Game %d killed by signal %d\n", childPID, WTERMSIG(childEnding));
	}
}

bool edit_table() {
	if (args == NULL || args[0] == NULL) {
		return true;
	}

	size_t lengthOfGame = strlen(args[0]) + LENGTH_OF_SERV_EXT + 1;

	char *gameName = calloc(lengthOfGame, sizeof(char));
	sprintf(gameName, "%s%s", args[0], SERV_EXT);

	free(args[0]);
	args[0] = gameName;

	return false;
}

int main(int argc, char **argv) {
	initialization();
	atexit(deinitialization);
	atexit(free_tabs);
	atexit(delete_childs);

	define_signals_actions();

	bool argsError = false;

	pid_t childPID = 0;
	
	while(1){
		if (usr1_receive){
			pid_t clientPID = 0;

			get_client_informations(&clientPID);

			argsError = edit_table();

			if (!argsError && game_exists(args[0])) {
				create_fifo_for_client(clientPID);
				send_sigusr1_to_client(clientPID);
				childPID = start_game();
				list_push_front(&pids, clientPID, childPID);
			} else {
				send_sigusr2_to_client(clientPID);
			}

			usr1_receive = 0;
		}
	}

	return 0;
}
