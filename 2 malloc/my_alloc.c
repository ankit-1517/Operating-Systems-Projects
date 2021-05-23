#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>

int magic_num = 12345;
size_t pageSize = 4096;

struct free_list_node{
    int size;
    struct free_list_node* next;
};
// takes up 16 bytes due to padding
int free_list_node_size = sizeof(struct free_list_node);

struct malloc_header{
    int size, magic_num;
};
// 8 bytes
int malloc_header_size = sizeof(struct malloc_header);

/**
 * Allocator description:
 * 
 * HEAP INFO
 * 0  - max size
 * 4  - current size
 * 8  - free memory
 * 12 - blocks allocated
 * 16 - smallest available chunk
 * 20 - largest available chunk
 * 
 * FREE LIST
 * 24 - linked list head
 * 40 - first node for free list
 * 
 * 56 - free region start
 */
char* allocator = NULL;

int my_init(){
    allocator = mmap(NULL, pageSize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANON|MAP_PRIVATE, 0, 0);
    if(allocator == MAP_FAILED){
        return errno;
    }

    // max size = 4096 - space used to store heap + other variables
    int x = 4096 - 40;
    memcpy(allocator, &x, 4);
    // head + first node (curr size = free list usage + allotted)
    x = 16;
    memcpy(allocator + 4, &x, 4);
    // free memory = max size - curr size
    x = 4040;
    memcpy(allocator + 8, &x, 4);
    // blocks allocated
    x = 0;
    memcpy(allocator + 12, &x, 4);
    // smallest avail chunk
    x = 4040;
    memcpy(allocator + 16, &x, 4);
    // largest avail chunk -- same as smallest for now
    memcpy(allocator + 20, &x, 4);

    // initialise free list
    // head
    struct free_list_node* temp = (struct free_list_node*)(allocator + 24);
    temp->next = (struct free_list_node*)(allocator + 40);
    // first node
    struct free_list_node temp_node;
    temp_node.size = x;
    temp_node.next = NULL;
    memcpy(allocator + 40, &temp_node, 16);
    return 0;
}

struct free_list_node* get_free_list_head(){
    struct free_list_node* head = (struct free_list_node*)(allocator + 24);
    return head;
}

int smallest_chunk_size(){
    int x;
    memcpy(&x, allocator + 16, 4);
    return x;
}

void find_smallest_chunk_size(){
    struct free_list_node* temp = get_free_list_head();
    temp = temp->next;
    int x = 4096;
    while(temp != NULL){
        if(x > temp->size && temp->size > 0)
            x = temp->size;
        temp = temp->next;
    }
    if(x == 4096)
        x = 0;
    memcpy(allocator + 16, &x, 4);
}

void reassign_smallest_chunk_size(int x){
    if(x == 0)
        find_smallest_chunk_size();
    else
        memcpy(allocator + 16, &x, 4);
}

int largest_chunk_size(){
    int x;
    memcpy(&x, allocator + 20, 4);
    return x;
}

void reassign_largest_chunk_size(int x){
    memcpy(allocator + 20, &x, 4);
}

void find_largest_chunk_size(){
    struct free_list_node* temp = get_free_list_head();
    int x = 0;
    while(temp != NULL){
        if(x < temp->size)
            x = temp->size;
        temp = temp->next;
    }
    memcpy(allocator + 20, &x, 4);
}

void inc_blocks_allocated(){
    int x;
    memcpy(&x, allocator + 12, 4);
    x++;
    memcpy(allocator + 12, &x, 4);
}

void dec_blocks_allocated(){
    int x;
    memcpy(&x, allocator + 12, 4);
    x--;
    memcpy(allocator + 12, &x, 4);
}

void update_current_size(int extra_alloted, bool allocated){
    int y;
    memcpy(&y, allocator + 4, 4);
    if(allocated)
        y += extra_alloted;
    else
        y -= extra_alloted;
    memcpy(allocator + 4, &y, 4);
}

void update_free_memory(int extra_alloted, bool allocated){
    int y;
    memcpy(&y, allocator + 8, 4);
    if(allocated)
        y -= extra_alloted;
    else
        y += extra_alloted;
    memcpy(allocator + 8, &y, 4);
}

// first fit based allocation
void* my_alloc(int block_size){
    if(allocator == NULL)
        return NULL;
    if(block_size <= 0 || block_size%8 > 0)
        return NULL;
    if(largest_chunk_size() == 0)
        return NULL;
    if(block_size > largest_chunk_size() + 8)
        return NULL;
    
    char* p;
    struct free_list_node* curr = get_free_list_head();
    struct free_list_node* prev = curr;
    int size_old, size_new, extra_alloted;
    curr = curr->next;
    while(curr != NULL){
        if(curr->size + free_list_node_size < malloc_header_size + block_size){
            goto z;
        }
        if(curr->size <= block_size){
            // rem curr node and allocate memory
            prev->next = curr->next;
            size_old = curr->size;
            size_new = free_list_node_size + curr->size - malloc_header_size;
            p = (char*)curr;
            struct malloc_header* temp = (struct malloc_header*) p;
            temp->magic_num = magic_num;
            temp->size = size_new;
            p += malloc_header_size;
            
            if(size_old == smallest_chunk_size())
                find_smallest_chunk_size();
            extra_alloted = curr->size + malloc_header_size - free_list_node_size;
            break;
        }
        else{
            // extra space available -- split
            size_old = curr->size;
            curr->size = curr->size - block_size - malloc_header_size;
            size_new = curr->size;
            p = (char*)curr;
            p += free_list_node_size + curr->size;
            struct malloc_header* temp = (struct malloc_header*) p;
            temp->magic_num = magic_num;
            temp->size = block_size;
            p += malloc_header_size;
            
            if(size_new < smallest_chunk_size())
                reassign_smallest_chunk_size(size_new);
            extra_alloted = block_size + malloc_header_size;
            break;
        }
        z: prev = curr;
        curr = curr->next;
    }
    
    // update heap
    if(size_old == largest_chunk_size())
        find_largest_chunk_size();
    inc_blocks_allocated();
    update_current_size(extra_alloted, true);
    update_free_memory(extra_alloted, true);
    return p;
}

void merge_nodes(struct free_list_node* left, struct free_list_node* right){
    left->size += right->size + free_list_node_size;
    left->next = right->next;
}

void print_free_list(){
    if(allocator == NULL)
        return;
    struct free_list_node* node = get_free_list_head();
    node = node->next;
    int cnt = 0;
    printf("------- FREE LIST -------\n");
    while(node != NULL){
        cnt++;
        printf("Node %d, size %d\n", cnt, node->size);
        node = node->next;
    }
    if(cnt == 0){
        printf("No node in free list\n");
    }
    printf("-------------------------\n");
}

int min(int a, int b){
    if(a < b)return a;
    return b;
}

void my_free(char *ptr){
    if(allocator == NULL)
        return;
    if(ptr == NULL)
        return;
    char* temp_ptr = (char*)ptr - malloc_header_size;
	struct malloc_header* block = (struct malloc_header*)(temp_ptr);
    if(block->magic_num != magic_num){
        // not a valid pointer
        return;
    }
    block->magic_num = 0;
    int size = block->size + malloc_header_size - free_list_node_size;
    int block_size = block->size;
    struct free_list_node* node = (struct free_list_node*)(temp_ptr);
    node->size = size;
    node->next = NULL;

    struct free_list_node* head = get_free_list_head();
    if(head->next == NULL){
        head->next = node;
        dec_blocks_allocated();
        reassign_largest_chunk_size(node->size);
        reassign_smallest_chunk_size(node->size);
        update_free_memory(block_size + malloc_header_size - free_list_node_size, false);
        update_current_size(block_size + malloc_header_size - free_list_node_size, false);
        return;
    }
    struct free_list_node *curr = head->next, *prev = head;
    while(true){
        if(curr == NULL){
            prev->next = node;
            break;
        }
        if((char*)curr - (char*)node > 0){
            prev->next = node;
            node->next = curr;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // merge
    int new_size = node->size;
    int new_small_size = node->size;
    int freed_memory = block_size + malloc_header_size - free_list_node_size;
    
    if(prev != head){
        if((char*)node == (char*)prev + free_list_node_size + prev->size){
            new_size += prev->size + free_list_node_size;
            new_small_size = min(new_small_size, prev->size);
            freed_memory += free_list_node_size;
            merge_nodes(prev, node);
            node = prev;
        }
    }
    if(curr != NULL){
        if((char*)node  + free_list_node_size + node->size == (char*)curr){
            new_size += curr->size + free_list_node_size;
            new_small_size = min(new_small_size, curr->size);
            freed_memory += free_list_node_size;
            merge_nodes(node, curr);
        }
    }

    // update heap
    dec_blocks_allocated();
    if(new_size > largest_chunk_size()){
        reassign_largest_chunk_size(new_size);
    }
    if(new_small_size <= smallest_chunk_size()){
        find_smallest_chunk_size();
    }
    update_current_size(freed_memory, false);
    update_free_memory(freed_memory, false);
    return;
}

void my_clean(){
    if(allocator == NULL)
        return;
    munmap(allocator, pageSize);
    return;
}

void my_heapinfo(){
    if(allocator == NULL)
        return;
    int a, b, c, d, e, f;
    memcpy(&a, allocator, 4);
    memcpy(&b, allocator + 4, 4);
    memcpy(&c, allocator + 8, 4);
    memcpy(&d, allocator + 12, 4);
    memcpy(&e, allocator + 16, 4);
    memcpy(&f, allocator + 20, 4);
    
	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", a);
	printf("Current Size: %d\n", b);
	printf("Free Memory: %d\n", c);
	printf("Blocks allocated: %d\n", d);
	printf("Smallest available chunk: %d\n", e);
	printf("Largest available chunk: %d\n", f);
	printf("==============================\n");
	return;
}

// int main(int argc, char const *argv[]){
//     my_init();
//     char* b = my_alloc(8);
//     char* a = my_alloc(16);
//     char* c = my_alloc(4000);
//     my_heapinfo();
//     my_free(c);
//     my_free(b);
//     my_heapinfo();
//     my_free(a);
//     my_heapinfo();
//     my_free(NULL);
//     my_clean();
//     return 0;
// }


