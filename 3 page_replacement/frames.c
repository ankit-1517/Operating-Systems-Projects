#include "src.h"
#include <ctype.h>

void run_queries(){
    for(int i=0; i < total_queries; i++){
        curr_query_index = i;
        load_page(query[i], query_is_write[i]);
    }
}

char *ltrim(char *s){
    while(isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s){
    char* back = s + strlen(s);
    if(strlen(s) == 0)
        return s;
    if(*(back-1) == '\n')
        back--;
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s){
    return rtrim(ltrim(s)); 
}

int get_cnt(char const file_path[]){
    FILE *stream = fopen(file_path, "r");
    if(!stream){
        puts("I/O error.\n");
        exit(1);
    }
    int cnt = 0;
    while(!feof(stream)){
        char c[20];
        fgets(c, sizeof(c), stream);
        char* x = trim(c);
        if(strlen(x) < 6)
            break;
        cnt++;
        c[0] = '\0';
    }
    fclose(stream);
    return cnt;
}

int str_to_address(const char* c){
    char x[20];
    for(int i=0; i<10; i++)
        x[i] = c[i];
    x[10] = '\0';
    return strtol(x, NULL, 16);
}

void read_file(char const file_path[]){
    FILE *stream = fopen(file_path, "r");
    if(!stream) {
        puts("I/O error.\n");
        exit(1);
    }
    for(int i=0; i<total_queries; i++){
        char c[20];
        fscanf(stream, "%s", c);
        int address = strtol(c, NULL, 16);
        fscanf(stream, "%s", c);
        query[i] = address;
        query_is_write[i] = (c[0] == 'W');
    }
    fclose(stream);
}

void initialise(int argc, char const *argv[]){
    if(argc < 4 || argc > 6){
        printf("Invalid usage. Exiting...\n");
        exit(1);
    }
    srand(5635);
    // ./frames trace.in 100 OPT -verbose
    buffer_size = atoi(argv[2]);
    curr_size = 0;
    char c[][10] = {"OPT", "FIFO", "CLOCK", "LRU", "RANDOM"};
    for(int i=0; i<5; i++){
        if(strcmp(c[i], argv[3]) == 0){
            strategy = i;
            break;
        }
    }
    if(argc == 5)
        verbose = true;
    
    // read query file
    total_queries = get_cnt(argv[1]);
    query = (int*)malloc(sizeof(int)*total_queries);
    query_is_write = (bool*)malloc(sizeof(bool)*total_queries);
    if(query == NULL || query_is_write == NULL){
        printf("Malloc failed. Exiting...\n");
        exit(1);
    }
    read_file(argv[1]);
}

void print_stats(){
    printf("Number of memory accesses: %d\n", total_queries);
    printf("Number of misses: %d\n", misses);
    printf("Number of writes: %d\n", writes);
    printf("Number of drops: %d\n", drops);
}

int main(int argc, char const *argv[]){
    initialise(argc, argv);
    run_queries();
    print_stats();
    free(query);
    free(query_is_write);
    return 0;
}


