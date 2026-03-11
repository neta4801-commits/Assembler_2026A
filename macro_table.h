/* macro_table header file */
/* This file includes constants and structs that used in macro_table.c file that builds the macro table.  */

#ifndef MACRO_TABLE_H
#define MACRO_TABLE_H
#include "constant.h"

/* Constant numbers */
#define NUMBER_TWO 2

/* represents a single line from the macro the user was writing (including '\n' and '\0'). */
typedef struct macro_line_node {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    struct macro_line_node *next;
} macro_line_node;


/* represents all macros that the user was writing (his name, lines).
 * In order to add macros, we have a pointer to the head and the tail of the list. */
typedef struct macro_node {
    char macro_name[MAX_LABEL_LENGTH + NUMBER_ONE];
    macro_line_node *lines_head;
    macro_line_node *lines_tail;
    struct macro_node *next;
} macro_node;


/* A pointer to the macros list. */
typedef macro_node *macro_ptr;

/* This function gets a pointer to the head of the macros list and a macro name.
 * The function searches macro on the list by his name.
 * Returns a pointer to this macro with his code lines.  */
macro_ptr get_macro(macro_ptr head,  char *macro_name);

/*This function gets a pointer to macro and the value of line.
* The function adds the new line to the macro lines. */
void add_macro_line(macro_ptr current_macro,  char *macro_line);


/* This function gets a pointer to the head of the macros list and macro name.
 * The function adds the new macro to the macros list and returns a pointer to this macro with his code lines. */
macro_ptr add_macro(macro_ptr *head,  char *macro_name);

/* This function gets a pointer to the head of the macros list and safely free all allocated memory */
void free_macro_table(macro_ptr head);


#endif