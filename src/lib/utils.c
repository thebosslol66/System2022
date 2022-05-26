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


void list_create(struct list *self) {
  self->first = NULL;
}

void list_destroy(struct list *self){
    struct list_node *node = self -> first;
    struct list_node *nextNode = NULL;
    if (node == NULL){
        return;
    }
    while(node!=NULL){
        nextNode = node -> next;
        free(node);
        node = nextNode;
    }
}

bool list_empty(const struct list *self) {
  return self -> first == NULL;
}

void list_push_back(struct list *self, struct list_node *newNode) {
  newNode->next = NULL;
  struct list_node *curr = self -> first;
  while (curr->next != NULL){
      curr = curr->next;
  }
  curr->next = newNode;
}

void list_remove(struct list *self, size_t index) {
  struct list_node *node = self -> first;
  if (index == 0){
      self->first = node -> next;
  }
  else {
      for (size_t i = 0; i < index; i++){
          node = node ->next;
      }
  }
  free(node);
}

bool list_kill_client(struct list *self, pid_t serverPID){
    struct list_node *node = self -> first;
    while (node != NULL && node -> serverPID != serverPID){
        node = node -> next;
    }
    if (node != NULL){
        node -> isAlive = false;
        return true;
    }
    return false;
}

size_t list_search_server(struct list *self, pid_t serverPID, pid_t *clientPID){
    size_t index = 0;
    struct list_node *node = self -> first;
    while (node != NULL && node -> serverPID != serverPID){
        node = node -> next;
        index++;
    }
    if (node != NULL){
        clientPID = &(node -> clientPID);
        return index;
    }
    clientPID = NULL;
    return -1;
}