typedef struct account{ // used to hold the mutex of each account and balance
	pthread_mutex_t mutex;
	int balance;
} account; 
typedef struct trans{
	int acc_id;
	int amount;
}trans

typedef struct operation{
	struct operation* nextop;
	int ID;
	int check_op;
	struct trans* transcations;
	int num_trans;
	struct timeval starttime;
}operation;

typedef struct LinkedList{
	struct operation *head, *end;
	int id_op;
	int num_op;
}LinkedList;

typedef struct initialArgs{
	int num_workers;
	int num_accounts;
}intialArgs;

 


