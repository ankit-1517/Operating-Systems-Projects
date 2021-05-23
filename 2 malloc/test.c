#include "my_alloc.c"

int main(int argc, char const *argv[]){
    my_init();
	
	my_heapinfo();
	void* a1 = my_alloc(16);
	my_heapinfo();
	void* a2 = my_alloc(8);
	my_heapinfo();
	void* a3 = my_alloc(24);
	my_heapinfo();
	print_free_list();

	my_free(NULL);
	my_free(a1);
	my_heapinfo();
	print_free_list();

	my_free(a3);
	my_heapinfo();
	print_free_list();

	my_alloc(32);
	my_heapinfo();
	print_free_list();

	my_free(a2);
	my_heapinfo();
	print_free_list();

	char* c1 = my_alloc(24);
	my_heapinfo();
	print_free_list();

	char* c2 = my_alloc(32);
	my_heapinfo();
	print_free_list();

	my_free(c1);
	my_heapinfo();
	print_free_list();

	my_free(c2);
	my_heapinfo();
	print_free_list();

	char* c3 = my_alloc(40);
	my_heapinfo();
	print_free_list();

	char* c4 = my_alloc(40);
	my_heapinfo();
	print_free_list();

	my_free(c3);
	my_heapinfo();
	print_free_list();

	char* c5 = my_alloc(16);
	my_heapinfo();
	print_free_list();

	char* c6 = my_alloc(24);
	my_heapinfo();
	print_free_list();

	my_free(c6);
	my_heapinfo();
	print_free_list();

	char* z1 = my_alloc(3840-8);
	my_heapinfo();
	print_free_list();

	char* z2 = my_alloc(32-8);
	my_heapinfo();
	print_free_list();

	char* z3 = my_alloc(24-8);
	my_heapinfo();
	print_free_list();
	
	my_free(z1);
	my_heapinfo();
	print_free_list();

	my_free(z2);
	my_heapinfo();
	print_free_list();

	my_free(z3);
	my_heapinfo();
	print_free_list();
	
	my_clean();
}


