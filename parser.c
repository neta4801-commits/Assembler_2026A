//This file checks legal words, macros and labels according to project instructions.

#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "tables.h"
#include "helpers.h"

/* There are words that we can use then as a macro name or label name, for example registers name as a label.
 * This function gets a word from the user's source file code
 * and returns true if this is a word that the user can't use and false otherwise. */
boolean is_forbidden_word(char *word) {
    /* The word is empty so it doesn't forbidden word */
    if (word == NULL) {
        return FALSE;
    }

    /* We need to check if the word is a register name or a commend name (forbidden words). */
    if (is_register(word) || name_command(word) != NULL) {
        return TRUE;
    }
    /* We need to check if the word is one of the words: .data, .string,
     * .entry, .extern,
     * or macro, mcroend.
     * Because all of those words are also forbidden words for a label or macro name. */
    if (strcmp(word, ".data") == NUMBER_ZERO   || strcmp(word, ".string") == NUMBER_ZERO ||
        strcmp(word, ".entry") == NUMBER_ZERO  || strcmp(word, ".extern") == NUMBER_ZERO ||
        strcmp(word, "mcro") == NUMBER_ZERO    || strcmp(word, "mcroend") == NUMBER_ZERO) {
        return TRUE;
    }
    return FALSE;
}


/* This function gets a macro or label name and checks if this is a legal name.
 * legal name - starts with letters, valid length- MAX_LABEL_LENGTH, not a forbidden word.
 * The function returns true is this is a legal name, otherwise false. */
boolean  is_legal_name(char *name) {
    int i=NUMBER_ZERO, name_length;
    /* The word is empty so this isn't a legal name. */
   if (name == NULL){
       return FALSE;
   }
   name_length = strlen(name);
   /* This is unlegal name if the length is zero or the length is more than the maximum length. */
    if (name_length == NUMBER_ZERO || name_length > MAX_LABEL_LENGTH) {
        return FALSE;
    }

    /* We need to check if the first char is a letter. Otherwise, this is unlegal name for label or macro. */
    if( !( (name[NUMBER_ZERO] >= 'a' && name[NUMBER_ZERO]<='z') ||
    (name[NUMBER_ZERO] >= 'A' && name[NUMBER_ZERO]<='Z') )) {
        return FALSE;
    }

    /* The rest of the name can include letters and numbers, so we need to check this.
     * For example MAIN@ isn't a legal name for label because @ isn't a letter or number.*/
    for (i = NUMBER_ONE; i < name_length; i++) {

        /* Checks if the char is a letter or number and if not, return false- unlegal name. */
        if(!( (name[i] >= 'a' && name[i]<='z') ||
              (name[i] >= 'A' && name[i]<='Z') ) ||
           (name[i] >= '0' && name[i]<='9') ) {
            return  FALSE;
        }
    }

    /* Checks if this is not a forbidden word. */
    if (is_forbidden_word(name)) {
        return FALSE;
    }
    return TRUE;
}


/* This function gets a pointer to the code line
 * and extracts the word until white space or ',' in target. For example, the command sub. */
void extract_word(char **line_ptr, char *target) {
    int i = NUMBER_ZERO;

    /* We need to check if the given addresses aren't NULL. */
    if (line_ptr == NULL || *line_ptr == NULL || target == NULL) {
        return;
    }
    /* We ignore the white spaces between the words in the line.*/
    skip_whitespaces(line_ptr);

    /* we extract the current word from the line to the target
     * until we got a space or '\0' or '\n' or ',' or '\t'.  */
    while (**line_ptr != '\0' && **line_ptr != '\n' &&
           **line_ptr != ' ' && **line_ptr!= '\t' && **line_ptr != ',') {
        target[i] = **line_ptr;
        i++;
        /* We need to move the lint pointer to the next char. */
        (*line_ptr)++;
    }
    target[i] = '\0';
}

/* This function get a word and check if its label.
 * The function returns true if it does. otherwise, false. */
boolean is_label(char *word) {
    int length;
    /* We need to add 2 cells - one for ':' (a must for label) and one for '\0'. */
    char clean_label[MAX_LABEL_LENGTH + NUMBER_TWO];
    if (word == NULL) {
        return FALSE;
    }
    length = strlen(word);
    /* Checks if the label longer then the maximum length including ':' */
    if(length>MAX_LABEL_LENGTH+NUMBER_ONE){
        return FALSE;
    }
    /* For a legal label, we need the label to end with ':' and have at least one letter.  */
    if (length < NUMBER_TWO || word[length - NUMBER_ONE] != ':') {
        return FALSE;
    }
    /* Copy the label name without ':' */
    strncpy(clean_label, word, length - NUMBER_ONE);
    clean_label[length - NUMBER_ONE] = '\0';
    /* We also need to check now if the label name is legal. */
    return is_legal_name (clean_label);
}


/* This function gets a string from the word that we extracted, and checks if this is a legal number.
 * The function returns true if it does. Otherwise, false. */
boolean is_legal_number(char *str) {
    int i = NUMBER_ZERO;
    if (str == NULL || str[NUMBER_ZERO] == '\0') {
        return FALSE;
    }
    if (str[NUMBER_ZERO] == '+' || str[NUMBER_ZERO] == '-') {
        i = NUMBER_ONE;
        if (str[i] == '\0') {
            return FALSE;
        }
    }
    while (str[i] != '\0') {
        if (!isdigit(str[i])) {
            return FALSE;
        }
        i++;
    }
    return TRUE;
}


/* This function gets a string from the word that we extracted,
 * and checks if this is a legal string - start with " and end with ".
 * The function returns true if it does. Otherwise, false. */
boolean is_legal_string(char *str) {
    int length, i=NUMBER_ZERO;
    /* We need to check if the given address isn't NULL. */
    if (str == NULL) {
        return FALSE;
    }

    length = strlen(str);

    /* We check if this is a legal string. The smallest one : "" */
    if (length < NUMBER_TWO || str[NUMBER_ZERO] != '"' || str[length - NUMBER_ONE] != '"') {
        return FALSE;
    }

    /* We need to check if all the chars in the string are legal.
     * We use the function isprint()- it returns true if the char
     * can be printed on the screen (by ASCII values), otherwise false. */
    for (i = NUMBER_ONE; i < length - NUMBER_ONE; i++) {
        if (!isprint(str[i])) {
            return FALSE;
        }
    }
    return TRUE;
}