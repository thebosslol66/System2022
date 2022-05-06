#include "hangman.h"
#include "message.h"

#define SERV_IN_FILENO 4
#define SERV_OUT_FILENO 3

void flush(FILE * fp){
    int c;
    while ((c = getc(fp)) != '\n' && c != EOF);
}

int have_space(char * str){
	int strLength = strlen(str);
	int haveSpace = 0;
	int pos = 0;
	while (pos < strLength && haveSpace == 0){
		if (str[pos] == ' '){
			haveSpace = 1;
		}
		pos++;
	}
	return haveSpace;
}

int inGame = 0;
/* handler du signal SIGUSR1 */
void handSIGUSR1(int sig) {
	#ifdef DEBUG
	fprintf(stderr, "Recieve sig usr1\n");
	#endif
	printf(CLI_OUT_OF_TIME);
	printf(CLI_EXIT);
	inGame = LOSE_OUT_OF_TIME;
}

void handSIGUSR2(int sig){
	inGame = SERVER_QUIT;
	printf(CLI_SERVER_CLOSE);
	#ifdef DEBUG
	fprintf(stderr, "hangman_serv has stopped\n");
	#endif
}



int save_result(){
	printf(CLI_ASK_SAVE);
	char * entry = calloc(MAX_NAME_LENGTH, sizeof(char));
	do {
		if (entry[0] != '\0'){
			printf(CLI_ASK_SAVE_NOT_UNDERSTAND);
		}
		if (fgets(entry, 2, stdin) == NULL){
			dperror("fgets");
		}
		if (inGame != SERVER_QUIT && inGame != LOSE_OUT_OF_TIME){
			flush(stdin);
		}
	} while(entry[0] != 'Y' && entry[0] != 'y' &&
		entry[0] != 'N' && entry[0] != 'n' &&
		inGame != SERVER_QUIT && inGame != LOSE_OUT_OF_TIME);

	if (inGame == SERVER_QUIT || inGame == LOSE_OUT_OF_TIME){
		free(entry);
		return 0;
	}

	if (entry[0] == 'N' || entry[0] == 'n'){
		send_string(SERV_OUT_FILENO, STRING_QUIT);
		printf(CLI_ASK_SAVE_NOTOK);
		printf("\n");
		printf(CLI_EXIT);
		free(entry);
		return 0;
	}

	printf(CLI_ASK_SAVE_OK);
	printf(CLI_INFO_PSEUDO);
	printf("\n");
	printf(CLI_ASK_PSEUDO);

	entry[0] = '\0';
	char * servResp = NULL;
	do  {
		if (servResp != NULL){
			printf("%s", servResp);
		}
		else{
			free(servResp);
		}
		do {
			if (entry[0] != '\0'){
				printf(CLI_ASK_PSEUDO_NOT_UNDERSTAND);
				printf(CLI_ASK_PSEUDO);
			}
			if (fgets(entry, MAX_NAME_LENGTH, stdin) == NULL){
				dperror("fgets");
			}
			if (inGame != SERVER_QUIT){
				if (entry[strlen(entry)-1] != '\n' && inGame != SERVER_QUIT){
					flush(stdin);
				}
			}
		} while((strlen(entry) <= 4 ||
			have_space(entry)) &&
			inGame != SERVER_QUIT);//\n is written in entry

		if (inGame == SERVER_QUIT){
			free(servResp);
			free(entry);
			return 0;
		}
		
		int entryLength = strlen(entry);
		if (entry[entryLength-1] == '\n'){
			entry[entryLength-1] = '\0';
		}
		if (send_string(SERV_OUT_FILENO, entry) == -1){
			dperror("send_string");
			exit(1);
		}
		servResp = recv_string(SERV_IN_FILENO);
		if (servResp == NULL){
			dperror("recv_string");
			exit(1);
		}
		
	} while(strcmp(servResp, STRING_QUIT) != 0);
	free(servResp);
	free(entry);
	return 1;
}


int main(int argc, char *argv[]){
	char * serverWord = NULL;

	struct sigaction action2;
    action2.sa_flags = 0;
    sigemptyset(&action2.sa_mask);
    action2.sa_handler = handSIGUSR2;
    sigaction(SIGUSR2, &action2, NULL);
	

	serverWord = recv_string(SERV_IN_FILENO);
	if (serverWord == NULL){
		fprintf(stderr, "Error connecting to server\n");
		exit(2);
	}
	if (strcmp(serverWord, "OK") != 0){
		fprintf(stderr, "%s", serverWord);
		free(serverWord);
		exit(1);
	}
	free(serverWord);
	serverWord = NULL;

	struct sigaction actionUSR1;
    actionUSR1.sa_flags = 0;
    sigemptyset(&actionUSR1.sa_mask);
    actionUSR1.sa_handler = handSIGUSR1;
	sigaction(SIGUSR1, &actionUSR1, NULL);

	int pid = getpid();
	if (write(SERV_OUT_FILENO, &pid, sizeof(int)) == -1){
		dperror("write");
		exit(1);
	}

	int maxTry;
	if (read(SERV_IN_FILENO, &maxTry, sizeof(int)) == -1){
		dperror("read");
		exit(1);
	}

	int choiceNb = 0;
	char * entry = calloc(2, sizeof(char));

	printf(CLI_WELCOME_TEXT);
	if (maxTry > 0){
		printf(CLI_NB_ERROR_ALLOWED, maxTry);
	}
	printf(CLI_LIMIT_TEXT, RESP_TIMEOUT);
	printf("\n");
	printf(CLI_BEGIN_GAME);
	printf("\n");

	serverWord = recv_string(SERV_IN_FILENO);
	
	if (strcmp(serverWord, STRING_STOP) == 0){
		if (read(SERV_IN_FILENO, &inGame, sizeof(int)) == -1){
			dperror("read");
			exit(1);
		}
	}

	char * currentWord = "";
	currentWord = serverWord;

	while(!inGame){
		printf(CLI_TEXT_WORD_FIND, serverWord);
		printf(CLI_ENTER_TEXT, ++choiceNb);
		if (fgets(entry, 2, stdin) == NULL){
			if (inGame == LOSE_OUT_OF_TIME || inGame == SERVER_QUIT){
				if (serverWord != currentWord){
					free(serverWord);
				}
				free(currentWord);
				free(entry);
				exit(0);
			}
			dperror("fgets");
			exit(1);
		}
		if (write(SERV_OUT_FILENO, entry, sizeof(char)) == -1){
			dperror("write");
			exit(1);
		}
		flush(stdin);
		serverWord = recv_string(SERV_IN_FILENO);
		
		if (serverWord != NULL){
			if (strcmp(serverWord, STRING_STOP) == 0){
				//get the exit status of game
				if (read(SERV_IN_FILENO, &inGame, sizeof(int)) == -1){
					dperror("read");
					exit(1);
				}
			}
			else if (strcmp(serverWord, currentWord) == 0) {
				maxTry--;
				if (maxTry >=0){
					printf(CLI_BAD_CHOICE_WITH_ERROR_ALLOWED, maxTry);
				}
				else {
					printf(CLI_BAD_CHOICE_WITHOUT_ERROR_ALLOWED);
				}
			}
			else {
				if (maxTry >=0){
					printf(CLI_GOOD_CHOICE_WITH_ERROR_ALLOWED, maxTry);
				}
				else {
					printf(CLI_GOOD_CHOICE_WITHOUT_ERROR_ALLOWED);
				}
			}
			free(currentWord);
			currentWord = serverWord;
			printf("\n");
		}

	}
	if (inGame == LOSE_OUT_OF_TIME || inGame == SERVER_QUIT){
		if(serverWord != NULL){
			free(serverWord);
		}
		free(currentWord);
		free(entry);
		exit(0);
	}

	serverWord = recv_string(SERV_IN_FILENO); //Get final word
	int nbError;
	if (read(SERV_IN_FILENO, &nbError, sizeof(int)) == -1){
		dperror("read");
	}

	int hasSave = 0;
	if (inGame == WIN_HANGMAN){
		if (maxTry >=0){
			printf(CLI_WIN_TEXT_WITH_ERROR_ALLOWED, serverWord, nbError);
		}
		else {
			printf(CLI_WIN_TEXT_WITHOUT_ERROR_ALLOWED, serverWord);
		}
		hasSave = save_result();
	}
	else if (inGame == LOSE_TOO_MUCH_TRY) {
		printf(CLI_LOSE_TEXT, serverWord);
	}

	free(serverWord);
	free(currentWord);
	free(entry);
	
	if (inGame == SERVER_QUIT || inGame == SERVER_QUIT){
		return 0;
	}
	
	if (hasSave){
		printf(CLI_SAVE_EXIT);
	}
	else {
		printf(CLI_EXIT);
	}
	return 0;
}
