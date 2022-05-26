#ifndef H_HANGMAN_SERV
#define H_HANGMAN_SERV

#include <time.h>
#include <sys/stat.h>

//Server
#define SERV_USAGE_STRING "Usage : ./client hangman [-n N]\n"
#define SERV_ARGS_NUMBER_ERROR "Number of args is incorrect\n"
#define SERV_OPTION_NAME_ERROR "Error : the option %s is not supported\n"
#define SERV_TRY_NUMBER_ERROR "You can't have max try under 0\n"
#define SERV_WORD_FILE_ERROR "File '%s' can't be found. please verify your configuration\n"

#define SERV_WORD_FILE_PATH "./data/dictionnaire.txt"
#define SERV_LONGEST_FRENCH_WORD_LENGHT 28 //intergouvernementalisations\0 pârceque je veux pas respecter le sujet
#define SERV_NOT_ARGEE_WITH_NAME "I don't mind, your name is bad\n"

#endif //H_HANGMAN_SERV