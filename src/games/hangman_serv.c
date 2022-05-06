#include "hangman.h"
#include "message.h"

//Erreur SIGPIPE

unsigned int randInt(unsigned int max){
	uint32_t nbRand;
	if ((max-1) < RAND_MAX){
		int end = RAND_MAX / max;
    	end *= max;
    	while ((nbRand = rand()) >= end);
	}
	else if (max < RAND_MAX){
		nbRand = rand();
	}
	else {
		int end = UINT32_MAX / max;
    	end *= max;
		do {
			nbRand = 0;
			nbRand = nbRand | (rand() & 0xFF);
			nbRand = nbRand | ((rand() & 0xFF) << 8);
			nbRand = nbRand | ((rand() & 0xFF) << 16);
			nbRand = nbRand | ((rand() & 0xFF) << 24);
		} while(nbRand >= end);
	}
	return nbRand % max;
}

size_t count_file_lines(FILE * fp){
	int c;
	int nb_line = 0;
	while ((c = getc(fp)) != EOF){
    if (c == '\n')
        ++nb_line;
	}
	clearerr(fp);
	fseek(fp, SEEK_SET, 0);
	return nb_line;
}

char * get_random_word_from_file(FILE * fp){
	int c;
	int nb_line = count_file_lines(fp);


	int pos = 0;
	int lignePos = randInt(nb_line);
	while( pos < lignePos && (c = getc(fp)) != EOF) {
		if (c == '\n'){
			++pos;
		}
	}
	char * str = calloc(SERV_LONGEST_FRENCH_WORD_LENGHT, sizeof(char));
	if (fgets(str, SERV_LONGEST_FRENCH_WORD_LENGHT-1, fp) == NULL){
		//error
	}
	int lasPosChar = strlen(str)-1;
	if (str[lasPosChar] == '\n'){
		str[lasPosChar] = '\0';
	}
	fseek(fp, SEEK_SET, 0);
	return str;
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

int clientPID;
int alarmRaise = 0;
void handlerALRM(int sig){
	#ifdef DEBUG
	fprintf(stderr, "Too long sendig usr1 signal\n");
	#endif
	kill(clientPID, SIGUSR1);
	alarmRaise = 1;
}

int inGame = 0;
void handKILLSERV(int sig){
	inGame = SERVER_QUIT;
	#ifdef DEBUG
	fprintf(stderr, "Hangman need to stop\n");
	#endif
}

void record_result(int nb, int nb_errors, char *word, char *pseudo, time_t *now){
	FILE * fp = fopen(RESULT_FILE, "a");
	chmod(RESULT_FILE, 0622);

	struct tm *local = localtime(now);
	fprintf(fp, "%d;%d;%s;%s;%d-%02d-%02d at %02d:%02d:%02d\n", nb, nb_errors, word, pseudo,
		local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	fclose(fp);
}

void save_result(int nb, int nb_errors, char * word, time_t *now) {
	char * name = NULL;
	int correct = 0;
	do {
		if (name != NULL){
			send_string(1, SERV_NOT_ARGEE_WITH_NAME);
			free(name);
		}
		alarm(RESP_TIMEOUT);
		name = recv_string(0);
		if (name != NULL){
			int nameLength = strlen(name);
			if (nameLength >= 4 && !have_space(name) && name[nameLength-1] != '\n'){
				correct = 1;
			}
			if (strcmp(name, STRING_QUIT) == 0){
				free(name);
				return;
			}
		}
	}while((!correct && inGame != SERVER_QUIT) && !alarmRaise);
	alarm(0);
	if (inGame == SERVER_QUIT){
		kill(clientPID, SIGUSR2);
		free(name);
		return;
	}
	if (alarmRaise){
		free(name);
		return;
	}

	record_result(nb, nb_errors, word, name, now);
	free(name);
	send_string(1, STRING_QUIT);
}

int main(int argc, char *argv[]){

	srand(time(NULL));
	
	struct sigaction action2;
    action2.sa_flags = 0;
    sigemptyset(&action2.sa_mask);
    action2.sa_handler = handKILLSERV;
    sigaction(SIGINT, &action2, NULL);
    sigaction(SIGQUIT, &action2, NULL);
    sigaction(SIGTERM, &action2, NULL);


	char* initialisationInfo;
	int lentInfo;
	if (argc != 1 && argc != 3){
		lentInfo = strlen(SERV_ARGS_NUMBER_ERROR) + strlen(SERV_USAGE_STRING);
		initialisationInfo = calloc(lentInfo+1, sizeof(char));
		strcat(initialisationInfo, SERV_ARGS_NUMBER_ERROR);
		strcat(initialisationInfo, SERV_USAGE_STRING);
		send_string(1, initialisationInfo);
		free(initialisationInfo);
		exit(1);
	}

	if (argc == 3 && strcmp(argv[1], "-n") != 0){
		lentInfo = strlen(SERV_OPTION_NAME_ERROR) + strlen(SERV_USAGE_STRING) - 2 + strlen(argv[1]);
		initialisationInfo = calloc(lentInfo+1, sizeof(char));
		sprintf(initialisationInfo, SERV_OPTION_NAME_ERROR, argv[1]);
		strcat(initialisationInfo, SERV_USAGE_STRING);
		send_string(1, initialisationInfo);
		free(initialisationInfo);
		exit(2);
	}

	int maxTry = -1;
	if (argc == 3){
		maxTry = atoi(argv[2]);
		if (maxTry < 0){
			lentInfo = strlen(SERV_TRY_NUMBER_ERROR) + strlen(SERV_USAGE_STRING);
			initialisationInfo = calloc(lentInfo+1, sizeof(char));
			strcat(initialisationInfo, SERV_TRY_NUMBER_ERROR);
			strcat(initialisationInfo, SERV_USAGE_STRING);
			send_string(1, initialisationInfo);
			free(initialisationInfo);
			exit(3);
		}
	}

	FILE *fp = fopen(SERV_WORD_FILE_PATH, "r");
	if (fp == NULL){
		lentInfo = strlen(SERV_WORD_FILE_ERROR) - 2 + strlen(SERV_WORD_FILE_PATH);
		initialisationInfo = calloc(lentInfo+1, sizeof(char));
		sprintf(initialisationInfo, SERV_WORD_FILE_ERROR, SERV_WORD_FILE_PATH);
		send_string(1, initialisationInfo);
		free(initialisationInfo);
		exit(4);
	}

	send_string(1, "OK");
	char * word = NULL;
	word = get_random_word_from_file(fp);
	fclose(fp);

	if (read(0, &clientPID, sizeof(int)) == -1){
		dperror("read");
		exit(5);
	}

	if (inGame == SERVER_QUIT){
		kill(clientPID, SIGUSR2);
		free(word);
		exit(0);
	}

	if (write(1, &maxTry, sizeof(int)) == -1){
		dperror("write");
		exit(6);
	}

	

	int lenWord = strlen(word);

	#ifdef DEBUG
	fprintf(stderr, "string(%d) %s\n", lenWord, word);
	#endif

	char * discoverWord = NULL;
	discoverWord = calloc(lenWord+1, sizeof(char));
	for (int i=0; i<lenWord; i++){
		discoverWord[i]= '-';
	}

	struct sigaction actionALRM;
	sigemptyset(&actionALRM.sa_mask);
	actionALRM.sa_flags = 0;
	actionALRM.sa_handler = handlerALRM;
	sigaction(SIGALRM, &actionALRM, NULL);

	int tryUsed = 0;
	char c;
	while (!inGame && !alarmRaise){
		send_string(1, discoverWord);
		alarm(RESP_TIMEOUT);
		if (read(0, &c, sizeof(char)) == -1){
			dperror("read");
		}

		if (!inGame && !alarmRaise){
		int nbCharFound = 0;
			for (int i=0; i < lenWord; i++){
				if (word[i] == c){
					discoverWord[i] = word[i];
					nbCharFound++;
				}
			}

			if (nbCharFound == 0){
				tryUsed++;
			}
			if (maxTry >=0 && tryUsed > maxTry){
				inGame = LOSE_TOO_MUCH_TRY;
			}
			if (strcmp(discoverWord, word) == 0){
				inGame = WIN_HANGMAN;
				
			}
		}
	}
	alarm(0);
	if (inGame == SERVER_QUIT || alarmRaise){
		kill(clientPID, SIGUSR2);
		free(discoverWord);
		free(word);
		exit(0);
	}

	time_t now;
	time(&now);

	send_string(1, STRING_STOP);
	if (write(1, &inGame, sizeof(int)) == -1){
		dperror("write");
		exit(8);
	}

	send_string(1, word);
	if (write(1, &tryUsed, sizeof(int)) == -1){
		dperror("write");
		exit(9);
	}

	if (inGame == WIN_HANGMAN){
		save_result(lenWord, tryUsed, word, &now);
	}

	free(discoverWord);
	free(word);

	return 0;
}
