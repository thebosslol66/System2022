#include "utils.h"

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

void flush(FILE * fp){
    int c;
    while ((c = getc(fp)) != '\n' && c != EOF);
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