// -----------------------------------
// By: Tomer Brown
// ID: 318338415
// -----------------------------------

# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# define PROBLEM -1
# define SUCCESSFUL 0

//---------------------------------------------------------------------------
//                             Global Variables                                    
//---------------------------------------------------------------------------
/*The Global Data structure to hold the number of times each printable char has appeared*/

unsigned int pcc_total [127] = {0};
volatile sig_atomic_t is_last_request = 0;
int is_in_middle_of_request = 0;

//---------------------------------------------------------------------------
//                        Request Struct related functions                                    
//---------------------------------------------------------------------------

/*A struct to hold the input and output of the requeseted needs to be proccessed in the server
Has the following fields:
1) int len (input)
2) char* string (input)
3) int printables_number (output)
*/
typedef struct Request {
    unsigned int len;
    char* string;
    int printables_number;
} Request;

/*A function that initializes the request and allocates memory according to the given n
If failed allocating memory - returns NULL
*/
Request* init_request(unsigned int n){
    Request* req = malloc(sizeof(Request));
    if (req==NULL){
        return NULL;
    }
    req -> len = n ;
    req ->string = calloc(n,sizeof(char));
    req ->printables_number = 0;
    return req;
}

/*Deep free the request struct*/
void free_request (Request* req){
    free(req->string);
    free (req);
}

/*Prints the data inside pcc_total in the requested format*/
void print_pcc_total(){
    for (int i =32; i<=126 ; i++){
        printf("char '%c' : %u times\n",(char) i , pcc_total[i]);
    }
}

/*A function to count how many printables. It updates the global array pcc_total*/
unsigned int count_printable(char* str){
    int n = strlen(str);
    unsigned int printables_counter = 0;
    for (int i=0 ; i<n ;i++){
        if (str[i] >=32  && str[i] <=126){
            pcc_total [(int) str[i]] ++ ;
            printables_counter++;
        }
    }
    return printables_counter;
}

//---------------------------------------------------------------------------
//                          Helper Functinos for main                                    
//---------------------------------------------------------------------------

/*Given a string representing the port number requester returns SUCCESSFUL if it is valid and PROBLEM if it isn't*/
int validate_port(char* port_str){
    for (int i=0; i< strlen(port_str);i++){
        if (port_str[i] < 48 || port_str[i] > 57){
            fprintf(stderr,"Error: Non numeric (Integer) port number given as input\n");
            return PROBLEM;
        }
    }
    int port_num = atoi(port_str);
    if (port_num < 1024){
        fprintf(stderr,"Error: Port inserted is reserved for root only\n");
        return PROBLEM;
    }
    if (port_num >= 65536){ //Check if it is not more than 16 bits
        fprintf(stderr,"Error: Port inserted is too large (more than 16 bits)\n");
        return PROBLEM;
    }
    return SUCCESSFUL;
}

/*A function that validates the input given as arguments to the program
    On Success: returns the port number as an int
    On Failure: prints an error message to stderr and returns PROBLEM
*/
int validate_input(int argc, char* argv []){
    if (argc!= 2){
        fprintf(stderr,"Error: not enough or too many arguments given\n");
        return PROBLEM;
    }
    char* port_str = argv[1];
    if (validate_port(port_str) == PROBLEM){
        return PROBLEM;
    }
    return atoi(port_str);
}

/*A function to run upon terminating*/
int finalize(void){
    print_pcc_total();
    exit(0);
}

/*The handler for sigint function
All it does is to raise a flag that the requeset being processed right now is the last one so the main
loop will know it suppose to exit now/
*/
void sigint_handler(int sig){
    is_last_request = 1;
    if (!is_in_middle_of_request){
        finalize();
    }
}


//---------------------------------------------------------------------------
//                                  Main                                    
//---------------------------------------------------------------------------

int main(int argc, char* argv []){
    //Firstly : set the signal handler to be my signal handler (To make sure when finishing everything is done atomicly)
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = &sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,NULL);
    
    //Secondly: Validate the input to make sure is correct
    int port_num = validate_input(argc,argv);
    if (port_num == PROBLEM){
        return 1;
    }
    while (!is_last_request){
        printf("Hello\n");
        sleep(1);
    }
    return finalize();
}