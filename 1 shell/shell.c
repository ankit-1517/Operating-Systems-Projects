#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "cd.h"
#include "history.h"

#define INPUT_MAX 130

struct history *history, temp_hist;
struct cd *cd, temp_cd;

void initialise(){
    cd = &temp_cd;
    cd_initialise(cd);
    history = &temp_hist;
    history_initialise(history);
}

void preprocess(char* p){
    int size = get_size(p);
    p[size-1] = '\0';
    remove_space(p);
}

int run_command(char* ar[], int cnt){
    char* argv[cnt+1];
    for(int i=0; i<cnt; i++){
        argv[i] = ar[i];
    }
    argv[cnt] = NULL;
    int pid = fork();
    if(pid < 0){
        printf("Fork failed!\n");
        return -1;
    }
    else if(pid == 0){
        execvp(argv[0], argv);
    }
    else{
        int exit_code = wait(NULL);
        return exit_code;
    }
    return -1;
}

char* replace_home(char* p){
    bool flag = true;
    char* temp = (char*)malloc(sizeof(char)*PATH_SIZE);
    int i=0, j=0;
    for(; i<get_size(p); i++, j++){
        if(p[i] == '"'){
            flag = !flag;
            goto z;
        }
        else if(p[i] == '~' && flag){
            // printf("start: %s %d\n", cd->startPath, get_size(cd->startPath));
            for(int k=0; k<get_size(cd->startPath); k++)
                temp[j+k] = cd->startPath[k];
            j += get_size(cd->startPath)-1;
            continue;
        }
        z: temp[j] = p[i];
    }
    temp[j] = '\0';
    // printf("temp:    %s\n", temp);
    return temp;
}

void process_query(char* p){
    int size = get_size(p);
    if(size == 0){
        return;
    }
    if(size <= 1){
        printf("%s: Command not found\n", p);
        return;
    }
    char *tokens[100];
    int cnt = 0;
    char dup[INPUT_MAX];
    for(int i=0; i<size; i++)
        dup[i] = p[i];
    dup[size] = '\0';
    char *token = strtok(dup, " ");
    while(token != NULL){
        tokens[cnt++] = token;
        token = strtok(NULL, " ");
    }
    if(strcmp(tokens[0], "cd") == 0){
        if(cnt == 1){
            printf("Destination not provided in cd.\n");
            return;
        }
        p += 3;
        process_cd(cd, p);
        return;
    }
    else if(strcmp(tokens[0], "history") == 0){
        if(cnt > 1){
            printf("History doesn't take arguments.\n");
            return;
        }
        history_print(history);
    }
    else{
        for(int i=0; i<cnt; i++){
            tokens[i] = replace_home(tokens[i]);
            // char* temp_home = replace_home(tokens[i]);
            // if(strcmp(temp_home, tokens[i]) != 0){
            //     tokens[i] = (char*)malloc(PATH_SIZE*sizeof(char));
            //     tokens[i] = temp_home;
            // }
            // printf("final token: %s\n", tokens[i]);
        }
        int exit_code = run_command(tokens, cnt);
        // printf("home: %s\n", cd->startPath);
        if(exit_code == -1){
            printf("Failed to run command: %s\n", p);
            return;
        }
    }
}

int main(){
    initialise();
    
    while(true){
        size_t bufsize = INPUT_MAX;
        char *input = (char*)malloc(bufsize*sizeof(char));
        cd_printPrompt(cd);
        getline(&input,&bufsize,stdin);
        if(strlen(input) > 128){
            if(input[128] != '\n'){
                printf("Too long command. Ignored.\n");
                free(input);
                continue;
            }
        }
        preprocess(input);
        history_add(history, input);
        process_query(input);
        free(input);
    }
    printf("\n");
    return 0;
}

