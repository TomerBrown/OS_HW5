// -----------------------------------
// By: Tomer Brown
// ID: 318338415
// -----------------------------------

# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/types.h>
# define PROBLEM -1
# define SUCCESSFUL 0

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

/*Given the command line arguments returns whether the is a problem or that everything is fine*/
int validate_arguments(int argc, char* argv[]){  
    if (argc != 4){
        fprintf(stderr, "Error: Expected exactly 3 argumnets but %d was given.\n",argc-1);
        return PROBLEM;
    }
    char* port_str = argv[2];
    if (validate_port(port_str)==PROBLEM){
        return PROBLEM;
    }
    return SUCCESSFUL;
}

/*Given a path to a file returns the number of charachters in that file
On Failure - returns PROBLEM
*/
int file_len(char* file_path){
    int len;
    FILE* fp = fopen(file_path,"r");
     if (fp == NULL){
        perror("");
        return PROBLEM;
    }
    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    fclose(fp);
    return len;
}

/*Given a path to file and a initialized string with N chars the function copies the content from file into the string 
    On failure returns PROBLEM
*/
int file_to_string(char* file_path, char* string, int N){
    FILE* fp = fopen(file_path,"r");
     if (fp == NULL){
        perror("");
        fclose(fp);
        return PROBLEM;
    }
    if (fread(string,sizeof(char),N,fp)!=N){
        fclose(fp);
        return PROBLEM;
    }
    fclose(fp);
    return SUCCESSFUL;
}
//---------------------------------------------------------------------------
//                          Debugging helper functions                                    
//---------------------------------------------------------------------------
/*Given a path to file print all it content to standard output*/
int print_file(char* file_path){
    FILE* fptr = fopen(file_path,"r");
    if (fptr == NULL){
        perror("");
        return PROBLEM;
    }
    char c;
    while ((c=getc(fptr))!=EOF){
        printf("%c",c);
    }
    printf("\n");
    fclose(fptr);
    return SUCCESSFUL;
}
void error_exit(void){
    perror("");
    exit(1);
}
//---------------------------------------------------------------------------
//                                  Main
//---------------------------------------------------------------------------
int main (int argc, char* argv []){
    if (validate_arguments(argc,argv)==PROBLEM){
        return 1;
    }
    struct sockaddr_in serv_addr;
    char* ip_adr = argv[1];
    int port_num = atoi(argv[2]);
    char* file_path = argv[3];

    //Validate IP Adress
    int x = inet_pton(AF_INET,ip_adr,&(serv_addr.sin_addr));
    if (x==0){
        fprintf(stderr,"Error: Ip address is invalid \n");
        return 1;
    }
    if (x<0){
        perror("");
    }

    //Get info about the message we want to transmit to the server
    uint32_t length = file_len(file_path);
    printf("Trying to send %u bytes from client\n",length);
    char* content = calloc(length,sizeof(char));
    if (file_to_string(file_path,content,length)==PROBLEM){
        return 1;
    }

    //Create the socket and bind it to the given port number
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd<0){
        free(content);
        error_exit();
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);
    socklen_t addrsize = sizeof(struct sockaddr_in );

    if (connect (sockfd ,(struct sockaddr*) &serv_addr, addrsize)<0){
        free(content);
        close(sockfd);
        error_exit();
    }

    
    //printf("Content of file is:\n%s\n",content);

    close(sockfd);
    free(content);
    //printf ("Address is : %u | port number is : %d \n", serv_addr.sin_addr.s_addr,port_num);
    
}