/* This file handles macros and their lines of code with lists */

#include <stdlib.h>
#include <string.h>
#include "macro_table.h"
#include "helpers.h"

/* This function gets a pointer to the head of the macros list and a macro name.
 * The function searches for macros on the list by its name,
 * and returns a pointer to this macro with its code lines.  */
macro_ptr get_macro(macro_ptr head,  char *macro_name) {
    macro_ptr current = head;
    if(macro_name == NULL) {
        return NULL;
    }
    while(current != NULL) {
    /* if the target name is the same return current */
        if(strcmp(current->macro_name, macro_name)== NUMBER_ZERO)  {
            return current; 
        }
        current = current->next;
    }
    return NULL;
}

/*This function gets a pointer to macro and the value of line.
* The function adds a new line to the macro lines. */
void add_macro_line(macro_ptr current_macro,  char *macro_line) {
    macro_line_node *new_macro_line;

    /* If the macro values are empty, we need to exit from the function and not add anything. */
    if(current_macro == NULL || macro_line == NULL) {
        return ;
    }

    new_macro_line = (macro_line_node *)check_malloc(sizeof(macro_line_node)); /*helper function */

    /* copies the context macro line to the list of lines including '\n'. */
    strncpy(new_macro_line->line, macro_line, MAX_LINE_LENGTH + NUMBER_ONE);
    new_macro_line->line[MAX_LINE_LENGTH + NUMBER_ONE] = '\0';
    new_macro_line->next = NULL;

    /* This is our fist line in this macro. */
    if(current_macro->lines_head == NULL){
        current_macro->lines_head = new_macro_line;
        current_macro->lines_tail = new_macro_line;
    }

    /* adds a new line to our list of lines for this macro. */
    else {
        current_macro->lines_tail->next = new_macro_line;
        current_macro->lines_tail = new_macro_line;
    }
}


/* This function gets a pointer to the head of the macros list and macro name.
 * The function adds the new macro to the macros list and returns a pointer to this macro with his code lines.
 * We add the new macro on the head of the list (O(1)), because we don't want to go through the entire list in order to add new macro.
 * So, we need to change the head for the list. */
macro_ptr add_macro(macro_ptr *head,  char *macro_name) {

    macro_ptr new_macro;
    if (head == NULL || macro_name == NULL) {
        return NULL;
    }

    new_macro = (macro_ptr)check_malloc(sizeof(macro_node));
    strncpy(new_macro->macro_name, macro_name, MAX_LABEL_LENGTH);
    new_macro->macro_name[MAX_LABEL_LENGTH] = '\0';
    new_macro->lines_head = NULL;
    new_macro->lines_tail = NULL;

    /* Adding the new macro to the head of the list, so we change the head pointer. */
    new_macro->next = *head;
    *head = new_macro;

    return new_macro;
}

/* This function gets a pointer to the head of the macros list and safely free all allocated memory */
void free_macro_table(macro_ptr head) {
    macro_ptr current_macro = head;
    macro_ptr next_macro;
    macro_line_node *current_line;
    macro_line_node *next_line;

    /* We need to free the memory for each line in the lines list for each macro. */
    while (current_macro != NULL) {
        current_line = current_macro->lines_head;
        while (current_line != NULL) {
            next_line = current_line->next;
            free(current_line);
            current_line = next_line;
        }
        next_macro = current_macro->next;
        free(current_macro);
        current_macro = next_macro;
    }
}
