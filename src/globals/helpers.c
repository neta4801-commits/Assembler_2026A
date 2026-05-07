/* Helpers.c file was developed in order to prevent or minimize code repetition in the project,
 * and to help readability, easily change and control our code.
 */
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "helpers.h"

/*checks if memory allocation was successful. */
void *check_malloc(size_t size) { /* must have malloc safety check.*/
    void *ptr = malloc(size);
    if (ptr == NULL) {
      fprintf(stderr, "Memory allocation failed.\n"); /* sending error output to stderr not stdout */ 
      exit(EXIT_FAILURE);
    }
    return ptr;
}

/*skips spaces and '\t'. */
void skip_whitespaces(char **str) {
    if (str == NULL || *str == NULL)
      return;
    while (**str == ' ' || **str == '\t') { 
      (*str)++; /* skip if space or \t until first char found */
    }
}

/*returns true if line is empty, or is comment. */
boolean is_empty_or_comment(char *line) {
    char *ptr = line;
    if (line == NULL) 
      return TRUE; /* if line is empty returns true  */
    skip_whitespaces(&ptr); /* skips all spaces and \t until we reach our first char*/
    if (*ptr == '\0' || *ptr == '\n' || *ptr == ';') 
      return TRUE; /* return true for comments, newlines, and end of string */
    return FALSE;
}

/* combines file name with extension to create a full name for a file.
 * copies the original name of the file to file_name with \0 then concatenate the extension, once it finds \0
 * ('helpers' + '.c' = 'helpers.c') */
char *create_file_name(char *original_name, char *extension) { 
    char *file_name;
    size_t length;
    if (original_name == NULL || extension == NULL)
      return NULL; /* check if either the name or the extension is empty. */
    length = strlen(original_name) + strlen(extension) + NUMBER_ONE;
    file_name = (char *)check_malloc(length);
    strcpy(file_name, original_name);/*first copy original_name to file_name with \0 at the end of the word*/
    strcat(file_name, extension); /* secondly concatenate the extension, once it finds \0 */
    return file_name;
}
