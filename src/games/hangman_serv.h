#ifndef H_HANGMAN_SERV
#define H_HANGMAN_SERV

#include <time.h>
#include <sys/stat.h>

/* String for showing infos */
#define SERV_USAGE_STRING "Usage : ./client hangman [-n N]\n"
#define SERV_ARGS_NUMBER_ERROR "Number of args is incorrect\n"
#define SERV_OPTION_NAME_ERROR "Error : the option %s is not supported\n"
#define SERV_TRY_NUMBER_ERROR "You can't have max try under 0\n"
#define SERV_WORD_FILE_ERROR "File '%s' can't be found. please verify your configuration\n"

/* File where to find word list */
#define SERV_WORD_FILE_PATH "./data/dictionnaire.txt"
/* The file where write results */
#define RESULT_FILE "/tmp/game_server/hangman_results"
/* Length of max of word in dict */
#define SERV_LONGEST_FRENCH_WORD_LENGHT 28 //intergouvernementalisations\0 pârceque je veux pas respecter le sujet
/* Responcer for a bad name to client */
#define SERV_NOT_ARGEE_WITH_NAME "I don't mind, your name is bad\n"

/*
Get a random word in an open file
The file must contain only 1 word by rows.
Return a random word choosen in fp
*/
char * get_random_word_from_file(FILE * fp);
/*
Handler to catch the sigalarm.
It's stop the process and send a SIGURS1 to the client
*/
void handlerALRM(int sig);
/*
Handler to catch the SIGTERM, SIGQUIT, SIGINT.
Free memory send SIGUSR2 to client and end it's process
*/
void handKILLSERV(int sig);
/* 
Register results of the game in the RESULT_FILE.
Only record if a pseudo is send.
It record in file as nb;nb_errors;word;pseudo;now
*/
void record_result(int nb, int nb_errors, char *word, char *pseudo, time_t *now);
/*
Wait the client to receive the pseudo.
If recieve, register the result with record result and return 
else return directly
*/
void save_result(int nb, int nb_errors, char * word, time_t *now);

#endif //H_HANGMAN_SERV