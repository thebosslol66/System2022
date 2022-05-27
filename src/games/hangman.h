#ifndef H_HANGMAN
#define H_HANGMAN

#define _DEFAULT_SOURCE

#include "message.h"
#include "../lib/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>


/* Print errors if we set debug */
#ifdef DEBUG
#define dperror(a) perror(a)
#else
#define dperror(a)
#endif

/* Id for message between hangman client and server */
#define WIN_HANGMAN 1
#define LOSE_TOO_MUCH_TRY 2
#define LOSE_OUT_OF_TIME 3
#define SERVER_QUIT 4

/* The maximum name pseudo */
#define MAX_NAME_LENGTH 50
/* Time before closing server */
#define RESP_TIMEOUT 10

/* Strings to send informations between serv and cli */
#define STRING_STOP "STOP GAME"
#define STRING_QUIT "QUIT GAME"
#define STRING_OK "OK GAME"

#endif //H_HANGMAN