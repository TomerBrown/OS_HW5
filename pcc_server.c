# include <string.h>
# include <stdio.h>
# include <stdlib.h>

#define PROBLEM -1

/*The Global Data structure to hold the number of times each printable char has appeared*/
unsigned int pcc_total [127] = {0};

/*Prints the data inside pcc_total in the requested format*/
void print_pcc_total(){
    for (int i =32; i<=126 ; i++){
        printf("char '%c' : %u times\n",(char) i , pcc_total[i]);
    }
}

/*A function to count how many printables*/
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


void test_printable_counter(void){
    print_pcc_total();
    char* tomer = "Tomer";
    char* all = "!#$&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    printf ("String is: %s | Counter is: %u (Suppose to be 5)\n", all , count_printable(all));
    print_pcc_total();
    printf ("String is: %s | Counter is: %u (Suppose to be 5)\n", tomer , count_printable(tomer));
    print_pcc_total();
}


/*A function that validates the input given as arguments to the program
    On Success: returns the port number as an int
    On Failure: prints an error message to stderr and returns PROBLEM

*/
int validate_input(int argc, char* argv []){
    int port_num = 55;
    if (argc <=1 || argc >2){
        fprintf(stderr,"Error: not enough or too many arguments given\n");
        return PROBLEM;
    }
    char* port_str = argv[1];
    for (int i=0; i< strlen(port_str);i++){
        if (port_str[i] < 48 || port_str[i] > 57){
            fprintf(stderr,"Error: Non numeric (Integer) port number given as input\n");
            return PROBLEM;
        }
    }
    port_num = atoi(port_str);
    if (port_num < 1024){
        fprintf(stderr,"Error: Port inserted is reserved for root only\n");
        return PROBLEM;
    }
    if (port_num >= 65536){
        fprintf(stderr,"Error: Port inserted is too large (more than 16 bits)\n");
        return PROBLEM;
    }

    return atoi(port_str);

}

int main(int argc, char* argv []){
    int port_num = validate_input(argc,argv);
    if (port_num == PROBLEM){
        return 1;
    }
    //test_printable_counter();
}