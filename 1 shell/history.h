#ifndef HISTORY
#define HISTORY

struct history{
    int currIndex, currSize, maxSize, strLength;
    char *commands[5];
};

void history_initialise(struct history* hist){
    hist->currIndex = 0;
    hist->currSize = 0;
    hist->maxSize = 5;
    hist->strLength = 129;
    for(int i=0; i<hist->maxSize; i++){
        hist->commands[i] = (char*)malloc(sizeof(char)*hist->strLength);
    }
}

void history_add_internal(struct history* hist, char c[]){
    for(int i=0; i<hist->strLength; i++){
        hist->commands[hist->currIndex][i] = c[i];
        if(c[i] == '\0')
            break;
    }
    hist->currIndex = (hist->currIndex+1)%hist->maxSize;
    if(hist->currSize < hist->maxSize)
        hist->currSize++;
}

void history_add(struct history* hist, char c[]){
    if(hist->currSize == 0)
        history_add_internal(hist, c);
    else
        if(strcmp(hist->commands[(hist->currIndex-1+hist->maxSize)%hist->maxSize], c) != 0)
            history_add_internal(hist, c);
}

void history_print(struct history* hist){
    for(int i=0; i<hist->currSize; i++){
        printf("%s\n", hist->commands[(hist->currIndex+i-hist->currSize + hist->maxSize)%hist->maxSize]);
    }
}

#endif
