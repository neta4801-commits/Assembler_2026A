/* symbol_tables header file */

#ifndef SYMBOL_TABLES_H
#define SYMBOL_TABLES_H

#include "src/globals/constant.h"

/* This function gets a pointer to the head of the symbol list, and symbol details (name,address and the label type-code,data or extern).
 * The function adds the new symbol to the symbol list and returns true if it success, otherwise false. */
boolean add_symbol(symbol_ptr *head, char *label_name, int label_address, int is_code, int is_data, int is_extern);

/* This function gets a pointer to the head of the symbol list and a symbol name.
 * The function searches symbol on the list by his name.
 * Returns a pointer to this symbol with his details. */
symbol_ptr get_symbol(symbol_ptr head, char *label_name);

/* This function gets a pointer to the head of the symbol list and safely free all allocated memory */
void free_symbols(symbol_ptr head);

#endif