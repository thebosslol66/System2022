#ifndef H_HANGMAN
#define H_HANGMAN

#define _DEFAULT_SOURCE

#include "message.h"
#include "../lib/utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#ifdef DEBUG
#define dperror(a) perror(a)
#else
#define dperror(a)
#endif

#define WIN_HANGMAN 1
#define LOSE_TOO_MUCH_TRY 2
#define LOSE_OUT_OF_TIME 3
#define SERVER_QUIT 4

#define MAX_NAME_LENGTH 50
#define RESP_TIMEOUT 10
#define RESULT_FILE "/tmp/game_server/hangman_results"
#define STRING_STOP "STOP GAME"
#define STRING_QUIT "QUIT GAME"
#define STRING_OK "OK GAME"

#endif //H_HANGMAN