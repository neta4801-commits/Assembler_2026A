/* parser header file */

#ifndef PARSER_H
#define PARSER_H

#include "../globals/constant.h"


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

/* This function checks if the addressing mode for the command is  legal.
 * If it does, return true. Otherwise, false. */
boolean is_valid_addressing_mode(int modes[], int mode);

/* This function gets an operand from command and checks the addressing mode for him and returns it. */
boolean get_addressing_mode(char *operand);


/* Functions that help us with the first pass */

/* This function extract the string and put it in the data array- data_image.
 * If we have extra text or missing quote we print error to the user and return false, otherwise, true. */
boolean extract_string_data(char **line_ptr,  AssemblerState *state);

/* This function extract the data (from the .data sentence) and put it in the data array- data_image (if the data is legal).
 * If we are missing number or aren't putting ',' between numbers,
 * we print error to the user and return false. Otherwise, true. */
boolean extract_data(char **line_ptr,  AssemblerState *state);

/* This function check if we got all the operands we expected in the command and return true, otherwise false. */
boolean extract_operands(char **line_ptr, char *src, char *dst, int expected_ops, int line_number);

#endif
