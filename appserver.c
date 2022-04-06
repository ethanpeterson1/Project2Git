#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include "Bank.c"
#include "appserver.h"
#include <sys/time.h>

int num_tokens;
char** get_args(char* line);
char* read_line();
void init_app(char** argv, int argc);
void user_loop();
void* mainThreadMethod();
void* workerThread();
void* init_queue();
int argIdentification(char* args[]);
const char DELIMS[11] = " \n\t\r\v\f";
int num_workers;
int num_accounts; 
account* accounts;
LinkedList* queueOfOperations;
int status =1; 
int op_id = 1;
pthread_mutex_t queueLocker;
char* filename;
FILE* fileOut; 
int main (int argc, char** argv){
	queueOfOperations = malloc(20 * sizeof(LinkedList));
	init_app(argv, argc);
	init_queue();
	pthread_mutex_init(&queueLocker,NULL);
	int i;  
	/*initialArgs* args;
	args.num_workers = num_workers;
	args.num_accounts = num_accounts;
	initialArgs* temp = malloc(sizeof(*temp));
	*temp = args;
	pthread_t mainThread; 
	pthread_create(&mainThread,NULL,mainThreadMethod,temp);*/
	//pthread_join(mainThreadMethod, NULL);
	
	accounts = (account*) malloc(num_accounts*sizeof(account));
	for(i =0; i < num_accounts; i++){
		pthread_mutex_init(&(accounts[i].mutex),NULL);
		accounts[i].balance = 0;
	}
	printf("%d accounts created\n", i);
	pthread_t mainThread;
	pthread_create(&mainThread,NULL,mainThreadMethod,NULL);
	pthread_join(mainThread,NULL);
	return 0;
}

void* mainThreadMethod(){
	printf("Main Thread Created\n");
	int i;
	pthread_t workers[num_workers];
	for(i = 0; i<num_workers; i++){
		pthread_create(&workers[i],NULL, workerThread,NULL);
	}
	char* prompt = malloc(strlen(">"));
	prompt = ">";
	char* line;
	char** args;
	while(status){
		printf("%s", prompt);
		line = read_line(); //read the line inputted by user
		args = get_args(line); //split line into args 
		if(strcmp(args[0],"END")==0){ 
			status = 0;
			exit(0);
		}
		status = argIdentification(args); //execute user input
		free(line);
		free(args);
	}
	free(prompt);
}
void* init_queue(){
	queueOfOperations->head = NULL;
	queueOfOperations->end = NULL;
	queueOfOperations->id_op =0;
	queueOfOperations->num_op = 0;
}
void* workerThread(){
	while(status){
		if(queueOfOperations->head != NULL){
			operation* currop; 
			currop = malloc(sizeof(operation));
			currop = queueOfOperations->head;
			if(currop-> check_op){
				//flockfile(fileOut);
				//fprintf(fileOut, "CHECK test");
				//funlockfile(fileOut);
				printf("Check Value of account = %d\n", read_account(currop->ID));
				//pthread_mutex_lock(&queueLocker);
				struct timeval time;
				gettimeofday(&time,NULL);
				currop-> endtime = time;
				queueOfOperations-> head = currop -> nextop;
				queueOfOperations->num_op--;
				//pthread_mutex_unlock(&queueLocker);
			}
		}
	}
}

int argIdentification(char* args[]){ //arg identification and adds to queue
	if((strcmp(args[0],"CHECK") == 0) && args[1] != NULL){
		printf("CHECK Operation detected\n");
		//pthread_mutex_lock(&queueLocker);
		if(queueOfOperations->num_op == 0){
			printf("< ID %d\n", op_id);
			operation* tempop;
			tempop = malloc(sizeof(operation));
			tempop->ID = op_id;
			op_id++;
			struct timeval time; 
			gettimeofday(&time,NULL);
			tempop->starttime = time;
			tempop-> check_op = 1;
			tempop -> num_trans = 0;
			queueOfOperations->head = tempop;
			queueOfOperations->end = tempop;
			queueOfOperations->num_op++;
		}
		else{
			printf("< ID %d\n", op_id);
			operation* tempop;
			tempop = malloc(sizeof(operation));
			tempop->ID = op_id;
			op_id++;
			struct timeval time;
			gettimeofday(&time,NULL);
			tempop->starttime = time;
			tempop-> check_op = 1;
			tempop-> num_trans = 0;
			queueOfOperations->end->nextop = tempop;
			queueOfOperations->end = tempop;
			queueOfOperations->num_op++;
		}
		//pthread_mutex_unlock(&queueLocker);
	}
	if((strcmp(args[0],"TRANS") == 0) && (num_tokens-1 % 2) && num_tokens > 1){
		printf("TRANS Operation detected\n");
		//pthread_mutex_lock(&queueLocker);
		//if(queueOfOperations->num_op == 0){
			//int i;
			//for (int i = 1; i<num_tokens-1 ; i++){
	}
				
}
char* read_line(){ //reads inputted line from user
	size_t lineMaxSize = 64;
	int i =0; // used for position in line
	char *input = malloc(lineMaxSize*sizeof(char));
	size_t numChars;
	char nextChar;
	if(input == NULL){
		perror("Memory could not be allocated for buffer");
    		exit(1);
  	}
  	while(1){
		nextChar = getchar();
		if(nextChar == '\n'){
			input[i] = '\0'; //end with null char
			return input;
		}
		else{
			input[i] = nextChar;
		 }
		i++;
	}
}

char** get_args(char* line){
	char *arg;
  	int buffSize = 64;
  	num_tokens = 0; //used for position in tokens 
	char **args = malloc(buffSize * sizeof(char*));
	arg = strtok(line,DELIMS);
	if(!args){
		printf("Memory could not be allocated");
		exit(EXIT_FAILURE);
  	}
  	while(arg != NULL){
    		args[num_tokens] = arg;
    		num_tokens++;
    		arg = strtok(NULL,DELIMS);
      		if(!args){
			printf("Memory could not be allocated");
			exit(EXIT_FAILURE);
      		}
  	}
  	args[num_tokens]= NULL;
  	return args;
}
void init_app(char** argv, int argc){ //format should be appserver <# of worker threads> <# of accounts> <output file>
	// check if argc == 4
	num_workers = strtol(argv[1], NULL, 0);
	num_accounts = strtol(argv[2], NULL, 0);
	/*filename = malloc(64* sizeof(char*));
	strcmp(filename,argv[3]);
	fileOut = fopen(filename, "w");
	printf("Filename: %s\n", filename);*/
	if(initialize_accounts(num_accounts) == 1){
		printf("Init Works!\n");
	}
}
