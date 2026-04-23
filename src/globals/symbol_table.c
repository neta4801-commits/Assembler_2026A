/* symbol_table.c file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

/* This function gets a pointer to the head of the symbol list, and symbol details (name,address and the label type-code,data or extern).
 * The function adds the new symbol to the symbol list and returns true if it success, otherwise false. */
boolean add_symbol(symbol_ptr *head, char *label_name, int label_address, int is_code, int is_data, int is_extern) {
    symbol_ptr new_symbol;
    symbol_ptr current = *head;

    while (current != NULL) {
        /* if the symbol name is already exists, we return false */
        if (strcmp(current->label_name, label_name) == NUMBER_ZERO) {
            return FALSE;
        }
        current = current->next;
    }


    /* We add the new symbol and his details.*/
    new_symbol = (symbol_ptr)check_malloc(sizeof(symbol_node));
    strcpy(new_symbol->label_name, label_name);
    new_symbol->label_address = label_address;
    new_symbol->is_code = is_code;
    new_symbol->is_data = is_data;
    new_symbol->is_extern = is_extern;
    /* for now, we don't care if the label type is entry so we reset to false (the bit zero). */
    new_symbol->is_entry = FALSE;
    new_symbol->next = NULL;

    /* The first symbol on the list */
    if (*head == NULL) {
        *head = new_symbol;

    } else {
        current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_symbol;
    }
    return TRUE;
}

/* This function gets a pointer to the head of the symbol list and a symbol name.
 * The function searches symbol on the list by his name.
 * Returns a pointer to this symbol with his details. */
symbol_ptr get_symbol(symbol_ptr head, char *label_name) {
    symbol_ptr current = head;
    while (current != NULL) {
        /* if the target name is the same return current */
        if (strcmp(current->label_name, label_name) == NUMBER_ZERO) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* This function gets a pointer to the head of the symbol list and safely free all allocated memory */
void free_symbols(symbol_ptr head) {
    symbol_ptr temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}
