/* Helpers.c file was developed in order to prevent or minimize code repetition in the project,
 * and to help readability, easily change and control our code.
 */
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "helpers.h"

void *check_malloc(size_t size) { /* must have malloc safety check.*/
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n"); /* sending error output to stderr not stdout */ 
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void skip_spaces(char **str) {
    if (str == NULL || *str == NULL) return;
    while (**str == ' ' || **str == '\t') { 
        (*str)++; /* skip if space or \t  */
    }
}

boolean is_empty_or_comment(char *line) {
    char *ptr = line;
    if (line == NULL) 
      return TRUE; /* if line is empty returns true  */
    skip_spaces(&ptr); /* skips all spaces and \t until we reach our first char*/
    if (*ptr == '\0' || *ptr == '\n' || *ptr == ';') 
      return TRUE; /* return true for comments, newlines, and end of string */
    return FALSE;

}

char *create_file_name(char *original_name, char *extension) { 
    char *full_name;
    size_t length;
    if (original_name == NULL || extension == NULL)
      return NULL; /* check if either the name or the extension is empty. */
    length = strlen(original_name) + strlen(extension) + 1;
    full_name = (char *)check_malloc(length);
    strcpy(full_name, original_name); /* first copy original_name to full_name with \0 at the end of it*/
    strcat(full_name, extension); /* secondly concatenate the extension, once it finds \0 */
    return full_name;
}
