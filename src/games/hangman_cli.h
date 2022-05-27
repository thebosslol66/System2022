#ifndef H_HANGMAN_CLI
#define H_HANGMAN_CLI

/* MACRO for pipe to communicate with serv */
#define SERV_IN_FILENO 4
#define SERV_OUT_FILENO 3

/* MACRO for begin text the game */
#define CLI_WELCOME_TEXT "Welcome, you have to guess a word\n"
#define CLI_NB_ERROR_ALLOWED "You are allowed %d errors\n"
#define CLI_INFINITE_ERROR_ALLOWED "There is no limit number of errors\n"
#define CLI_TIME_ALLOWED "With each request, you have %d seconds to answer\n"
#define CLI_BEGIN_GAME "Begin of the game. Good luck !!!\n"
#define CLI_LIMIT_TEXT "With each request, you have %d seconds to answer\n"

/* MACRO for the text in game */
#define CLI_TEXT_WORD_FIND "Word to find : \"%s\"\n"
#define CLI_ENTER_TEXT "Choice %d, enter a letter : "

#define CLI_GOOD_CHOICE_WITH_ERROR_ALLOWED "Good choice, you are still entitled to %d errors\n"
#define CLI_BAD_CHOICE_WITH_ERROR_ALLOWED "Error, you are still entitled to %d errors\n"
#define CLI_GOOD_CHOICE_WITHOUT_ERROR_ALLOWED "Good choice\n"
#define CLI_BAD_CHOICE_WITHOUT_ERROR_ALLOWED "Error\n"

/* MACRO for end of the text in game */
#define CLI_WIN_TEXT_WITH_ERROR_ALLOWED "You won, you found the word \"%s\", you have made %d errors\n"
#define CLI_WIN_TEXT_WITHOUT_ERROR_ALLOWED "You won, you found the word \"%s\"\n"
#define CLI_LOSE_TEXT "You lose, the word was \"%s\"\n"
#define CLI_OUT_OF_TIME "\nYou are out of time.\n"

/* MACRO for saving results in the game */
#define CLI_ASK_SAVE "Do you want save your result ? [Y/n] "
#define CLI_ASK_SAVE_NOT_UNDERSTAND "I don't understand. Can you repeat please ? [Y/n] "
#define CLI_ASK_SAVE_OK "You have chosen to save your result,\n"
#define CLI_ASK_SAVE_NOTOK "You have chosen to not save your result.\n"

#define CLI_INFO_PSEUDO "you must choose a pseudo with no space and minimum length 4\n"
#define CLI_ASK_PSEUDO "Enter your pseudo : "
#define CLI_ASK_PSEUDO_NOT_UNDERSTAND "Error : the pseudo is to short, or it contains space(s)\n"
#define CLI_SAVE_EXIT "Bye, save the result was successful\n"
#define CLI_EXIT "Bye\n"

/* MACRO for saying the server stopped */
#define CLI_SERVER_CLOSE "The serve has suddently close\n"

/*
Handler for SIGUSR1 
It say to the user CLI_OUT_OF_TIME and close the process
*/
void handSIGUSR1(int sig);
/*
Handler for SIGUSR2
It say to the user CLI_SERVER_CLOSE and close the process
*/
void handSIGUSR2(int sig);
/*
Ask user a pseudo and send it to server if it's without space and countain
at loeast 4 char, then save result  in the sserver side and close process
in client size return 1 if the client save it's result else 0
*/
int save_result();


#endif //H_HANGMAN_CLI