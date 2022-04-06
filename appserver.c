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
int num_transactions;
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
	fclose(fileOut);
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
				//printf("Check Value of account = %d\n", read_account(currop->ID));
				//pthread_mutex_lock(&queueLocker);
				queueOfOperations-> head = currop -> nextop;
				queueOfOperations->num_op--;
				int value = read_account(currop->ID);
				struct timeval time;
				gettimeofday(&time,NULL);
				currop-> endtime = time;
				flockfile(fileOut);
				fprintf(fileOut,"%d BAL %d TIME %ld.%06.ld %ld.%06.ld\n", currop->ID, value,currop->starttime.tv_sec,currop->starttime.tv_usec, currop->endtime.tv_sec, currop->endtime.tv_usec );
				funlockfile(fileOut);
				//pthread_mutex_unlock(&queueLocker);
			}

			else if(currop -> num_trans){
				int isFundsPoss = 1;
				queueOfOperations-> head = currop -> nextop;
				queueOfOperations->num_op--;
				int ISFaccount,currID,currAmount;
				int i;
				printf("NUM TRANSACTIONS:%d\n",num_transactions);
				for(i =0;i < num_transactions; i++){
					printf("in for:%d\n",i);
					currID= currop->transactions[i].acc_id;
					currAmount = currop->transactions[i].amount;
					int accBalance = read_account(currID); 
					printf("currAmount : %d\n",currAmount);
					printf("accBalance: %d\n", accBalance);
					if(currAmount + accBalance > 0){
						printf("TRANS worked");
						write_account(currID, accBalance+currAmount);
					}
					else{
						isFundsPoss = 0;
						ISFaccount = currID;
						break;
					}
				}
				if(isFundsPoss){
					struct timeval time;
					gettimeofday(&time,NULL);
					currop-> endtime = time;
					flockfile(fileOut);
					fprintf(fileOut,"%d OK TIME %ld.%06.ld %ld.%06.ld\n",currop->ID,currop->starttime.tv_sec,currop->starttime.tv_usec, currop->endtime.tv_sec, currop->endtime.tv_usec);
					funlockfile(fileOut);
				}
				else{
					struct timeval time;
					gettimeofday(&time,NULL);
					currop-> endtime = time;
					flockfile(fileOut);
					fprintf(fileOut,"%d ISF %d TIME %ld.%06.ld %ld.%06.ld\n",currop->ID,ISFaccount,currop->starttime.tv_sec,currop->starttime.tv_usec, currop->endtime.tv_sec, currop->endtime.tv_usec);
					funlockfile(fileOut);
				}
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
		printf("num tokens %d\n", num_tokens);
		printf("TRANS Operation detected\n");
		//pthread_mutex_lock(&queueLocker);
		num_transactions = (num_tokens -1)/2;
		if(queueOfOperations->num_op == 0){
			printf("< ID %d\n", op_id);
			operation* tempop;
			tempop = malloc(sizeof(operation));
			tempop->ID = op_id;
			op_id++;

			tempop->transactions = malloc(num_transactions*(sizeof(trans)));
			int i;
			int j = 1;
			for (i=0; i<num_transactions; i++){
				struct trans trans1;
				trans1.acc_id = strtol(args[j], NULL, 0);
				j++;
				trans1.amount = strtol(args[j], NULL, 0);
				j++;
				tempop->transactions[i] = trans1;	
			}

			struct timeval time; 
			gettimeofday(&time,NULL);
			tempop->starttime = time;
			tempop-> check_op = 0;
			tempop -> num_trans = num_transactions;
			queueOfOperations->head = tempop;
			queueOfOperations->end = tempop;
			queueOfOperations->num_op++;
			printf("ID CHECK:%d\n", tempop->transactions[0].acc_id);
		}
		else{
			printf("< ID %d\n", op_id);
			operation* tempop;
			tempop = malloc(sizeof(operation));
			tempop->ID = op_id;
			op_id++;

			tempop->transactions = malloc(num_transactions*(sizeof(trans)));
			int i;
			int j = 1;
			for (i=0; i<num_transactions; i++){
				struct trans trans1;
				trans1.acc_id = strtol(args[j], NULL, 0);
				j++;
				trans1.amount = strtol(args[j], NULL, 0);
				j++;
				tempop->transactions[i] = trans1;	
			}

			struct timeval time;
			gettimeofday(&time,NULL);
			tempop->starttime = time;
			tempop-> check_op = 0;
			tempop-> num_trans = num_transactions;
			queueOfOperations->end->nextop = tempop;
			queueOfOperations->end = tempop;
			queueOfOperations->num_op++;
		}
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
	filename = malloc(64* sizeof(char*));
	strcpy(filename,argv[3]);
	fileOut = fopen(filename, "w");
	printf("Filename: %s\n", filename);
	if(initialize_accounts(num_accounts) == 1){
		printf("Init Works!\n");
	}
}
