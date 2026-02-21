// TODO: IMPLEMENT
#include "labletable.h"
#include "main.h"
#include <string.h>
#include <stdlib.h>

void insertLabel(char * label, uint64_t address, ltable *table) {
    if (table->count >= MAX_LABELS) {
        fprintf(stderr, "Error: Too many labels!\n");
        error();
    }
    
    strncpy(table->labels[table->count], label, 63);
    table->labels[table->count][63] = '\0';
    table->addresses[table->count] = address;
    table->count++;
}

uint64_t getintAddress(char *label, ltable *table) {
    for (int i = 0; i < table->count; i++)
        if (strcmp(table->labels[i], label) == 0)
			return table->addresses[i];
    
    fprintf(stderr, "Error: Label '%s' not found!\n", label);
    error();
	return 1;
}
