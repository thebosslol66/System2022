#define _DEFAULT_SOURCE

#include <message.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define PATH_PID_FILE_SERVER "/tmp/game_server.pid"
#define PATH_FIFO_GAME_SERVER "/tmp/game_server.fifo"

#define PID_BUFFER 16

#define USAGE "USAGE : %s hangman [-n N]", argv[0] // ???????????

static pid_t serverPID = 0;

void handSIGUSR1(int sig) {

}

void handSIGUSR2(int sig) {
	fprintf(stderr, "End of the program due to a server dysfunction\n");
	exit(2);
}

/*
	Je ne comprends pas comment utiliser cette fonction sachant que les arguments varient en fonction du jeu.
*/

void verify_arguments(int argc, char **argv) {
	/*bool validArgs = false;

	if (!validArgs) {
		fprintf(stderr, "Error : invalid arguments\n");
		fprintf(stderr, USAGE);
		exit(1);
	}*/
}

bool game_exists(char *game) {
	return false; // fichier existe et est executable
}

pid_t read_server_pid(void) {
	int fdPidServer = open(PATH_PID_FILE_SERVER, O_RDONLY);

	if (fdPidServer == -1) {
		fprintf(stderr, "Error while opening %s\n", PATH_PID_FILE_SERVER);
		perror("open");
		exit(1);
	}

	char buffer[PID_BUFFER];

	ssize_t retOfRead = read(fdPidServer, buffer, PID_BUFFER * sizeof(char));

	if (retOfRead == -1) {
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

	return (pid_t)(atoi(buffer));
}

void define_signals_actions(void) {
	struct sigaction actionUsr1;
	sigemptyset(&actionUsr1.sa_mask);
	actionUsr1.sa_handler = handSIGUSR1;
	actionUsr1.sa_flags = 0;
	sigaction(SIGUSR1, &actionUsr1, NULL);

	struct sigaction actionUsr2;
	sigemptyset(&actionUsr2.sa_mask);
	actionUsr2.sa_handler = handSIGUSR2;
	actionUsr2.sa_flags = 0;
	sigaction(SIGUSR2, &actionUsr2, NULL);
}

void send_sigusr1_to_server(void) {
	int retOfKill = kill(serverPID, SIGUSR1);

	if (retOfKill == -1) {
		fprintf(stderr, "Client error : can't send signal SIGUSR1 to server\n");
		perror("kill");
		exit(1);
	}
}

void send_client_informations(char **args) {
	int fdFifo = open(PATH_FIFO_GAME_SERVER, O_WRONLY);

	if (fdFifo == -1) {
		fprintf(stderr, "Error while opening %s\n", PATH_FIFO_GAME_SERVER);
		perror("open");
		exit(1);
	}

	char myPIDStr[PID_BUFFER];

	sprintf(myPIDStr, "%d", getpid());

	int resSendString = send_string(fdFifo, myPIDStr);

	if (resSendString == -1) {
		fprintf(stderr, "Error while sending the client informations\n");
		exit(1);
	}

	int resSendArgv = send_argv(fdFifo, args);

	if (resSendArgv == -1) {
		fprintf(stderr, "Error while sending the client informations\n");
		exit(1);
	}

	int resClose = close(fdFifo);
	
	if (resClose == -1) {
		fprintf(stderr, "Error while closing %s\n", PATH_FIFO_GAME_SERVER);
		perror("close");
		exit(1);
	}
}

int main(int argc, char **argv) {
	/*if (argc < 3) {
		fprintf(stderr, "Error : invalid arguments\n");
		fprintf(stderr, USAGE);
		exit(1);
	}

	verify_arguments(argc, argv);

	if (!game_exists(argv[1])) {
		fprintf(stderr, "Error : the game does not exists\n");
		fprintf(stderr, USAGE);
		exit(1);
	}*/

	serverPID = read_server_pid();
	define_signals_actions();
	send_sigusr1_to_server();

	char **args = argv + 1;

	send_client_informations(args);

	return 0;
}
