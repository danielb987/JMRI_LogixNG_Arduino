#include <stdlib.h>
#include "loconet_sv.h"


// The maximum number of SV string items we can handle.

#define MAX_SV_ADDR_LIST 10


typedef struct
{
	receive_string callback;
	int sv_addr;
	int size;
	char *buffer;
	int index;
} sv_string_struct;


sv_string_struct *sv_string_list[MAX_SV_ADDR_LIST];

int sv_string_list_size = 0;



int register_sv_string(receive_string callback, int sv_addr, int size) {

	if (sv_string_list_size+1 >= MAX_SV_ADDR_LIST)
	{
		// Too many sv_strings are already registered
		return 1;
	}

	sv_string_struct *sv_struct = (sv_string_struct*) malloc(sizeof(sv_string_struct));
	sv_struct->callback = callback;
	sv_struct->sv_addr = sv_addr;
	sv_struct->size = size;
	sv_struct->buffer = (char*) malloc(size+1);		// The asciiz 0 is not included in size
	sv_struct->index = 0;
	sv_string_list[++sv_string_list_size] = sv_struct;

	// Success
	return 0;
}
