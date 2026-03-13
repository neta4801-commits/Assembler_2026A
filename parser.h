/* parser header file */

#ifndef PARSER_H
#define PARSER_H

#include "constant.h"

/* There are words that we can use then as a macro name or label name, for example registers name as a label.
 * This function gets a word from the user's source file code
 * and returns true if this is a word that the user can't use and false otherwise. */
boolean is_forbidden_word(char *word);

/* This function gets a macro or label name and checks if this is a legal name.
 * legal name - starts with letters, valid length- MAX_LABEL_LENGTH, not a forbidden word.
 * The function returns true is this is a legal name, otherwise false. */
boolean is_legal_name(char *name);

/* This function gets a pointer to the code line
 * and extracts the word until white space or ',' in target. For example, the command sub. */
void extract_word(char **line_ptr, char *target);

/* This function get a word and check if its label.
 * The function returns true if it does. otherwise, false. */
boolean is_label(char *word);

/* This function gets a string from the word that we extracted, and checks if this is a legal number.
 * The function returns true if it does. Otherwise, false. */
boolean is_legal_number(char *str);

/* This function gets a string from the word that we extracted,
 * and checks if this is a legal string - start with " and end with ".
 * The function returns true if it does. Otherwise, false. */
boolean is_legal_string(char *str);

#endif
