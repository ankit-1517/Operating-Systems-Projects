#ifndef CD
#define CD

#include <dirent.h>
#include <errno.h>

#define PATH_SIZE 1024

struct cd{
    char startPath[PATH_SIZE];
    char currentPath[PATH_SIZE];
};

int get_size(const char* p){
    int size = 0;
    for(; p[size] != '\0'; size++);
    return size;
}

void remove_space(char* p){
    int size = get_size(p);
    for(int i=size-1; i>-1; i--){
        if(p[i] != ' ')
            break;
        p[i] = '\0';
    }
}

void cd_initialise(struct cd* cd){
    char* p = getcwd(cd->startPath, sizeof(cd->startPath));
    // printf("%s\n", cd->startPath);
    if(p != NULL){
        int i=0;
        for(; p[i] != '\0'; i++){
            cd->currentPath[i] = p[i];
        }
        cd->currentPath[i] = '\0';
        return;
    }
    printf("Unexpected error: Unable to get CWD\n");
    exit(0);
}

bool directoryExists(const char* p){
    DIR* dir = opendir(p);
    if(dir){
        closedir(dir);
        return true;
    }
    return false;
}

int cd_jumpDirectory(struct  cd* cd, const char* p){
    if(directoryExists(p) == false){
        printf("Directory does not exist: %s\n", p);
        return 0;
    }
    int i=0;
    for(; p[i] != '\0'; i++){
        cd->currentPath[i] = p[i];
    }
    cd->currentPath[i] = '\0';
    return 1;
}

int cd_backToHome(struct cd* cd){
    return cd_jumpDirectory(cd, cd->startPath);
}

void cd_printPath(struct cd* cd){
    bool full = false;
    if(get_size(cd->currentPath) < get_size(cd->startPath)){
        full = true;
    }
    else{
        for(int i=0; i < get_size(cd->startPath); i++){
            if(cd->currentPath[i] != cd->startPath[i]){
                full = true;
                break;
            }
        }
    }
    if(full){
        printf("%s", cd->currentPath);
    }
    else{
        printf("~");
        for(int i=get_size(cd->startPath); i < get_size(cd->currentPath); i++){
            printf("%c", cd->currentPath[i]);
        }
    }
}

void cd_printPrompt(struct cd* cd){
    printf("MTL458:");
    cd_printPath(cd);
    printf("$ ");
}

void change_directory(int x, struct cd* cd){
    if(x == 1)
        chdir(cd->currentPath);
}

bool balanced_quotes(const char* p, int size){
    bool flag = true;
    for(int i=0; i<size; i++){
        if(p[i] == '/' && !flag)
            return false;
        if(p[i] == '"')
            flag = !flag;
    }
    return flag;
}

bool balanced_space(const char* p, int size){
    bool flag = true;
    for(int i=0; i<size; i++){
        if(p[i] == '"')
            flag = !flag;
        if(p[i] == ' '){
            if(flag)
                return false;
        }
    }
    return true;
}

int has_prev(const char* p){
    int size = get_size(p);
    if(size < 3)
        return -1;
    for(int i=2; i<size; i++){
        if(p[i] == '.' && p[i-1] == '.' && p[i-2] == '/')
            return i;
    }
    return -1;
}

char* multiple_backs(struct cd* cd, char* p, bool append){
    char* temp = (char*)malloc(PATH_SIZE*sizeof(PATH_SIZE));
    int size = get_size(cd->currentPath);
    if(append){
        for(int i=0; i < size; i++){
            temp[i] = cd->currentPath[i];
        }
        temp[size] = '/';
    }
    else{
        size = -1;
    }
    for(int i=0; p[i]!='\0'; i++){
        temp[size+1+i] = p[i];
    }
    while(true){
        int index_end = has_prev(temp);
        if(index_end == -1)
            break;
        int i = index_end-3;
        for(; i>-1; i--){
            if(temp[i] == '/')
                break;
        }
        if(i < 0)
            return NULL;
        index_end++;
        for(;index_end < get_size(temp);i++, index_end++){
            temp[i] = temp[index_end];
        }
        temp[i] = '\0';
    }
    return temp;
}

void go_topmost(struct cd* cd){
    cd->currentPath[0] = '/';
    cd->currentPath[1] = '\0';
    change_directory(1, cd);
}

void process_cd(struct cd* cd, const char* p){
    char temp[PATH_SIZE] = {'\0'};
    int index = 0, size = get_size(p);
    while(index < size && p[index] == ' ')index++;

    if(index == size){
        printf("No arguments passed in cd. Invalid command.\n");
        return;
    }
    for(int i=0; index+i<size; i++)
        temp[i] = p[index + i];
    size = get_size(temp);

    // process quotes
    if(!balanced_quotes(p, size)){
        printf("Can't have \" in folder name.\n");
        return;
    }
    if(!balanced_space(p, size)){
        printf("cd: too many arguments.\n");
        return;
    }

    int j=0;
    for(int i=0; i < size; i++, j++){
        if(temp[i] == '"')
            j--;
        else
            temp[j] = temp[i];
    }
    temp[j] = '\0';
    size = j;
    
    int x;
    if(size == 0){
        printf("No arguments passed in cd. Invalid command.\n");
        return;
    }
    while(size > 1 && temp[size-1] == '/'){
        temp[size-1] = '\0';
        size--;
    }
    if(size == 1 && temp[0] == '~'){
        x = cd_backToHome(cd);
        change_directory(x, cd);
        return;
    }
    if(temp[0] == '/'){
        x = cd_jumpDirectory(cd, temp);
        change_directory(x, cd);
        if(x == 1){
            char *temp_new = multiple_backs(cd, temp, false);
            if(temp_new == NULL){
                go_topmost(cd);
                return;
            }
            size = get_size(temp_new);
            for(int i=0; i<size; i++)
                cd->currentPath[i] = temp_new[i];
            cd->currentPath[size] = '\0';
            change_directory(x, cd);
        }
        return;
    }
    if(temp[0] == '~'){ // and size > 1
        if(temp[1] != '/'){
            printf("Directory does not exist: %s\n", p);
            return;
        }
        x = cd_backToHome(cd);
        change_directory(x, cd);
        int i=2;
        for(; i<get_size(temp); i++)
            temp[i-2] = temp[i];
        temp[i-2] = '\0';
    }
    char *temp_new = multiple_backs(cd, temp, true);
    if(temp_new == NULL){
        go_topmost(cd);
        return;
    }
    if(temp_new[0] == '\0'){
        go_topmost(cd);
        return;
    }
    x = cd_jumpDirectory(cd, temp_new);
    change_directory(x, cd);
}

#endif
