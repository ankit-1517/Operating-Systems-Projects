#ifndef SRC
#define SRC

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1000
#define MAX_NUM_PAGES 1<<20

struct frame{
    int page_num;
    int query_index;
};
typedef struct frame frame;

frame frames[MAX_BUFFER_SIZE];

int page_table[MAX_NUM_PAGES] = {0};

// buffer size is actual available size of buffer
// curr_size points to first free location in array
int buffer_size = -1, strategy = -1, curr_size = -1;
// print extra stuff if true
bool verbose = false;

// QUERY FILE RELATED STUFF
// total_queries stores num of queries in trace file
int total_queries = -1, curr_query_index = -1;
// stores query address
int* query;
// stores true if query is read for all queries
bool* query_is_write;

// OUTPUT STATS RELATED STUFF
int misses = 0, writes = 0, drops = 0;

int min(int a, int b){
    return (a < b ? a: b);
}

// last bit of the number
int get_dirty_bit(int page_num){
    return (page_table[page_num] & 0x00000001);
}

void set_dirty_bit(int page_num, bool dirty){
    if(dirty == false){
        page_table[page_num] = (page_table[page_num] & 0xfffffffe);
    }
    else{
        page_table[page_num] = (page_table[page_num] | 0x00000001);
    }
}

// second last bit of the number
int get_valid_bit(int page_num){
    return (page_table[page_num] & 0x00000002);
}

void set_valid_bit(int page_num, bool valid){
    if(valid == false){
        page_table[page_num] = (page_table[page_num] & 0xfffffffd);
    }
    else{
        page_table[page_num] = (page_table[page_num] | 0x00000002);
    }
}

int address_to_page_num(int address){
    int page = (address & 0xfffff000);
    page = page>>12;
    return page;
}

int address_to_frame_num(int address){
    /**
     * max nu pages = 1000 < 4096 -- first 12 bits for frame num
     */
    int index = (address & 0xfff00000);
    index = index>>20;
    return index;
}

void set_frame_num(int index, int page_num){
    int x = page_table[page_num];
    index = index<<20;
    // initialise starting bits to 0
    x = (x & 0x000fffff);
    x = (x | index);
    page_table[page_num] = x;
}

void evict_page(int rem_index, int page_added){
    int page_removed = frames[rem_index].page_num;
    bool is_dirty = get_dirty_bit(page_removed); 
    if(is_dirty){
        writes++;
    }
    else{
        drops++;
    }
    set_valid_bit(page_removed, false);
    if(verbose){
        if(is_dirty)
            printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", page_added, page_removed);
        else
            printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", page_added, page_removed);
    }
}

//////////////////////////////////////////////
// CODE FOR REPLACEMENT POLICIES STARTS
//////////////////////////////////////////////

int opt_policy(int page){
    for(int fn=0; fn<buffer_size; fn++){
        int i=curr_query_index+1;
        if(frames[fn].query_index >= i)
            continue;
        frames[fn].query_index = total_queries;
        for(; i<total_queries; i++){
            if(frames[fn].page_num == address_to_page_num(query[i])){
                frames[fn].query_index = i;
                break;
            }
        }
    }
    int max_val = frames[0].query_index, max_ind = 0;
    for(int i=1; i<buffer_size; i++){
        if(frames[i].query_index > max_val){
            max_val = frames[i].query_index;
            max_ind = i;
        }
    }
    evict_page(max_ind, page);
    return max_ind;
}

int random_policy(int page){
    int index = rand()%buffer_size;
    evict_page(index, page);
    return index;
}

// specific only to FIFO
int fifo_rem_index = 0;

int fifo_policy(int page){
    int index = fifo_rem_index;
    evict_page(index, page);
    fifo_rem_index = (fifo_rem_index+1)%buffer_size;
    return index;
}

int lru_policy(int page){
    int min_val = frames[0].query_index, min_ind = 0;
    for(int i=1; i<buffer_size; i++){
        if(frames[i].query_index < min_val){
            min_val = frames[i].query_index;
            min_ind = i;
        }
    }
    evict_page(min_ind, page);
    return min_ind;
}

// specific only to CLOCK
int clock_hand_index = 0;

int clock_policy(int page){
    while(true){
        if(clock_hand_index == buffer_size)
            clock_hand_index = 0;
        if(frames[clock_hand_index].query_index == 0){
            break;
        }
        frames[clock_hand_index++].query_index = 0;
    }
    evict_page(clock_hand_index, page);
    int index = clock_hand_index;
    clock_hand_index++;
    clock_hand_index %= buffer_size;
    return index;
}

//////////////////////////////////////////////
// CODE FOR REPLACEMENT POLICIES ENDS
//////////////////////////////////////////////

// get frame index for the given page
int get_index(int page){
    if(get_valid_bit(page))
        return -1;
    // else, if page is not in memory
    misses++;
    // if buffer is full
    if(curr_size == buffer_size){
        switch(strategy){
            case 0:
                // opt
                return opt_policy(page);
            case 1:
                // fifo
                return fifo_policy(page);
            case 2:
                // clock
                return clock_policy(page);
            case 3:
                // lru
                return lru_policy(page);
            case 4:
                // random
                return random_policy(page);
            default:
                printf("Invalid strategy. Exiting...\n");
                exit(1);
        }
        exit(1);
        return -1;
    }
    // if buffer has space, return current index
    curr_size++;
    return curr_size-1;
}

void load_page(int address, bool is_write){
    int page_num = address_to_page_num(address);
    int index = get_index(page_num);
    // printf("%d %d %d\n", page_num, is_write, index);
    bool already_in_memory = false;
    if(index == -1){
        index = address_to_frame_num(page_table[page_num]);
        already_in_memory = true;
    }
    frames[index].page_num = page_num;
    frames[index].query_index = curr_query_index;
    set_valid_bit(page_num, 1);
    set_frame_num(index, page_num);
    
    // to handle the case the page is already in memory
    // it was already dirty, but currently it has only been read
    if(already_in_memory){
        if(get_dirty_bit(page_num) == 0)
            goto z;
        // dont need to do anything is already marked dirty
    }
    else{
        z: if(is_write)
            set_dirty_bit(page_num, 1);
        else
            set_dirty_bit(page_num, 0);
    }
}

#endif
