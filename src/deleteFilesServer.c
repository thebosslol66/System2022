#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#define PATH_PID_FILE_SERVER "/tmp/game_server.pid"
#define PATH_FIFO_GAME_SERVER "/tmp/game_server.fifo"
#define PATH_DIR_GAME_SERVER "/tmp/game_server"
#define LENGTH_OF_PATH_DIR 16

int main(int argc, char const *argv[]) {
	int resUnlink = unlink(PATH_PID_FILE_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing %s\n", PATH_PID_FILE_SERVER);
		perror("unlink");
		return 1;
	}

	resUnlink = unlink(PATH_FIFO_GAME_SERVER);

	if (resUnlink == -1 && errno != ENOENT) {
		fprintf(stderr, "Error while removing FIFO %s\n", PATH_FIFO_GAME_SERVER);
		perror("unlink");
		return 1;
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

				printf("%s\n", nameOfFifo);

				resUnlink = unlink(nameOfFifo);

				if (resUnlink == -1 && errno != ENOENT) {
					fprintf(stderr, "Error while removing FIFO %s\n", info->d_name);
					perror("unlink");
					free(nameOfFifo);
					return 1;
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
		return 1;
	}

	return 0;
}