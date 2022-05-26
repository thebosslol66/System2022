#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef DEBUG
#define dperror(a) perror(a)

#else
#define dperror(a)

#endif

//Server
#define SERV_USAGE_STRING "Usage : ./client hangman [-n N]\n"
#define SERV_ARGS_NUMBER_ERROR "Number of args is incorrect\n"
#define SERV_OPTION_NAME_ERROR "Error : the option %s is not supported\n"
#define SERV_TRY_NUMBER_ERROR "You can't have max try under 0\n"
#define SERV_WORD_FILE_ERROR "File '%s' can't be found. please verify your configuration\n"

#define SERV_WORD_FILE_PATH "./data/dictionnaire.txt"
#define SERV_LONGEST_FRENCH_WORD_LENGHT 28 //intergouvernementalisations\0 pârceque je veux pas respecter le sujet
#define SERV_NOT_ARGEE_WITH_NAME "I don't mind, your name is bad\n"


//Client
//Intro
#define CLI_WELCOME_TEXT "Welcome, you have to guess a word\n"
#define CLI_NB_ERROR_ALLOWED "You are allowed %d errors\n"
#define CLI_INFINITE_ERROR_ALLOWED "There is no limit number of errors\n"
#define CLI_TIME_ALLOWED "With each request, you have %d seconds to answer\n"
#define CLI_BEGIN_GAME "Begin of the game. Good luck !!!\n"
#define CLI_LIMIT_TEXT "With each request, you have %d seconds to answer\n"

//Game
#define CLI_TEXT_WORD_FIND "Word to find : \"%s\"\n"
#define CLI_ENTER_TEXT "Choice %d, enter a letter : "

#define CLI_GOOD_CHOICE_WITH_ERROR_ALLOWED "Good choice, you are still entitled to %d errors\n"
#define CLI_BAD_CHOICE_WITH_ERROR_ALLOWED "Error, you are still entitled to %d errors\n"
#define CLI_GOOD_CHOICE_WITHOUT_ERROR_ALLOWED "Good choice\n"
#define CLI_BAD_CHOICE_WITHOUT_ERROR_ALLOWED "Error\n"

//END
#define CLI_WIN_TEXT_WITH_ERROR_ALLOWED "You won, you found the word \"%s\", you have made %d errors\n"
#define CLI_WIN_TEXT_WITHOUT_ERROR_ALLOWED "You won, you found the word \"%s\"\n"
#define CLI_LOSE_TEXT "You lose, the word was \"%s\"\n"
#define CLI_OUT_OF_TIME "\nYou are out of time.\n"

//Save result
#define CLI_ASK_SAVE "Do you want save your result ? [Y/n] "
#define CLI_ASK_SAVE_NOT_UNDERSTAND "I don't understand. Can you repeat please ? [Y/n] "
#define CLI_ASK_SAVE_OK "You have chosen to save your result,\n"
#define CLI_ASK_SAVE_NOTOK "You have chosen to not save your result.\n"

#define CLI_INFO_PSEUDO "you must choose a pseudo with no space and minimum length 4\n"
#define CLI_ASK_PSEUDO "Enter your pseudo : "
#define CLI_ASK_PSEUDO_NOT_UNDERSTAND "Error : the pseudo is to short, or it contains space(s)\n"
#define CLI_SAVE_EXIT "Bye, save the result was successful\n"
#define CLI_EXIT "Bye\n"

#define CLI_SERVER_CLOSE "The serve has suddently close\n"


#define WIN_HANGMAN 1
#define LOSE_TOO_MUCH_TRY 2
#define LOSE_OUT_OF_TIME 3
#define SERVER_QUIT 4

#define MAX_NAME_LENGTH 50
#define RESP_TIMEOUT 10
#define RESULT_FILE "/tmp/game_server/hangman_results"
#define STRING_STOP "STOP GAME"
#define STRING_QUIT "QUIT GAME"