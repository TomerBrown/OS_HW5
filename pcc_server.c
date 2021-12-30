// -----------------------------------
// By: Tomer Brown
// ID: 318338415
// -----------------------------------

# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <errno.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/types.h>
# define PROBLEM -1
# define SUCCESSFUL 0
# define QUEUE_LEN 10
//---------------------------------------------------------------------------
//                             Global Variables                                    
//---------------------------------------------------------------------------
/*The Global Data structure to hold the number of times each printable char has appeared*/

unsigned int pcc_total [127] = {0};
volatile sig_atomic_t is_last_request = 0;
int is_in_middle_of_request = 0;
int sockfd;

/*Prints the data inside pcc_total in the requested format*/
void print_pcc_total(){
    for (int i =32; i<=126 ; i++){
        printf("char '%c' : %u times\n",(char) i , pcc_total[i]);
    }
}

/*A function to count how many printables.*/
unsigned int count_printable(char* str){
    int n = strlen(str);
    unsigned int printables_counter = 0;
    for (int i=0 ; i<n ;i++){
        if (str[i] >=32  && str[i] <=126){
            printables_counter++;
        }
    }
    return printables_counter;
}

void update_pcc_total(char* str){
    int n = strlen(str);
    for (int i=0 ; i<n ;i++){
        if (str[i] >=32  && str[i] <=126){
            pcc_total[(int)str[i]] ++;
        }
    }
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
int finalize(int status){
    print_pcc_total();
    close(sockfd);
    exit(status);
}

/*The handler for sigint function
All it does is to raise a flag that the requeset being processed right now is the last one so the main
loop will know it suppose to exit now/
*/
void sigint_handler(int sig){
    is_last_request = 1;
    if (!is_in_middle_of_request){
        finalize(0);
    }
}

void error_exit(void){
    perror("");
    exit(1);
}

/*Given the file descriptor of the connections and the returned value from read and write
returns:
SUCCESFUL - if numebr of bytes writeen/read from connection is valid
PROBLEM - if a tcp error occured (e.i. errno == ETIMEDOUT/ ECONNRESET / EPIP )
EXITS -   if an error occured which is not one of the above mentioned*/
int handle_tcp_errors (int fd, int count){
    if (count < 1){
        if (!(errno==ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
        close (fd);
        perror("Error: ");
        exit(1);
        }
        else{
            perror("TCP Error:");
            close(fd);
            return PROBLEM;
        }
    }
    return SUCCESSFUL;
}


//---------------------------------------------------------------------------
//                                  Main                                    
//---------------------------------------------------------------------------

int main(int argc, char* argv []){
    int confd;
    uint32_t tmp, length;
    int total_read, count_read;
    int total_write, count_write;
    char* message_from_client;
    int printable_chars;
    uint32_t printable_chars_int;
    int error_before = 0;
    //int length;

    //Initialize and clear the structs needed to represent connectins
    struct sockaddr_in serv_addr;
    struct sockaddr_in my_addr;
    struct sockaddr_in peer_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    memset(&serv_addr,0,addr_size);
    memset(&my_addr,0,addr_size);
    memset(&peer_addr,0,addr_size);
    //Firstly : set the signal handler to be my signal handler (To make sure when finishing everything is done atomicly)
    signal(SIGINT,&sigint_handler);
    //Secondly: Validate the input to make sure is correct
    int port_num = validate_input(argc,argv);
    if (port_num == PROBLEM){
        return 1;
    }
    
    //1. Create the socket and check that completed successfuly
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0){
        error_exit();
    }
    unsigned int yes = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    
    //serv.addr Parameters for binding
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port_num);

    //2. Bind the socket to given port
    bind(sockfd, (struct sockaddr*)&serv_addr , addr_size);
    
    //3. Start listening
    if (listen(sockfd,QUEUE_LEN)<0){
            error_exit();
        }
    
    while (!is_last_request){
        confd = accept(sockfd, (struct sockaddr*) &peer_addr, &addr_size);
        is_in_middle_of_request = 1;
        error_before = 0;
        //Get the length of the message from the client
        total_read = 0;
        while (total_read < 4){
            count_read = read(confd,&tmp+total_read,4-total_read);
            if (count_read<0){
                is_in_middle_of_request=0;
                if (handle_tcp_errors(confd,count_read)==PROBLEM){}
                
            }
            if (handle_tcp_errors (confd,count_read)==PROBLEM){
                is_in_middle_of_request=0;
                error_before = 1;
                break;
            }
            total_read+= count_read;
        }
        if (!error_before){
            length = ntohl(tmp);
            message_from_client = calloc(length,1);
            //printf("Length recivied is: %d\n",length);
            total_read = 0;
            while (total_read < length){
                count_read = read(confd,message_from_client+total_read,length-total_read);
                if (handle_tcp_errors(confd,count_read)==PROBLEM){
                    is_in_middle_of_request=0;
                    free (message_from_client);
                    error_before = 1;
                    break;
                }
                total_read+= count_read;
            }
        }
        if (!error_before){
            printable_chars = count_printable(message_from_client);
            printable_chars_int = htonl(printable_chars);
            total_write = 0;
            while (total_write < 4){
                count_write = write(confd,(char*) (&printable_chars_int)+total_write ,4-total_write);
                if (count_write<1){
                    is_in_middle_of_request=0;
                    close (confd);
                    exit(1);
                }
                if (handle_tcp_errors(confd,count_write)==PROBLEM){
                    error_before = 1;
                    free(message_from_client);
                    break;
                }
                total_write += count_write;
            }
        }
        if (!error_before){
            update_pcc_total(message_from_client);
            free(message_from_client);
        }
        
        is_in_middle_of_request=0;
        close (confd);
    }
    return finalize(0);
}