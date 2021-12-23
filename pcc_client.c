// -----------------------------------
// By: Tomer Brown
// ID: 318338415
// -----------------------------------

# include <string.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <arpa/inet.h>
# define PROBLEM -1
# define SUCCESSFUL 0


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

int main (int argc, char* argv []){
    if (validate_arguments(argc,argv)==PROBLEM){
        return 1;
    }
    struct sockaddr_in sa;
    char* ip_adr = argv[1];
    int port_num = atoi(argv[2]);
    //char* file_path = argv[3];

    if (inet_pton(AF_INET,ip_adr,&(sa.sin_addr))==0){
        fprintf(stderr,"Error: Ip address is invalid \n");
        return 1;
    }
    if (inet_pton<0){
        perror("");
    }

    printf ("Adress is : %u | port number is : %d \n ", sa.sin_addr.s_addr,port_num);

}