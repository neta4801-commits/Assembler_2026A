/* This file checks legal words, legal numbers macros and labels according to project instructions.
 * In addition, it includes functions that extract words from instruction lines (data, strings and operands)  for first pass */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "../../assembler_tables.h"
#include "../globals/helpers.h"

#define RANG_MIN_NUM -2048
#define  RANGE_MAX_NUM 2047


/* There are words that we can use then as a macro name or label name, for example registers name as a label.
 * This function gets a word from the user's source file code
 * and returns true if this is a word that the user can't use and false otherwise. */
boolean is_forbidden_word(char *word) {

    /* The word is empty so it doesn't forbidden word */
    if (word == NULL) {
        return FALSE;
    }

    /* We need to check if the word is a register name or a commend name (forbidden words). */
    if (is_register(word) || get_command(word) != NULL) {
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
              (name[i] >= 'A' && name[i]<='Z') ||
              (name[i] >= '0' && name[i]<='9') ) ) {
            return FALSE;
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

/* This function checks if a given number fits within a 12-bit signed integer range
 * (from -2048 to 2047). Returns true if it does, false otherwise. */
boolean is_number_range(int num) {
    if (num >= -2048 && num <= 2047) {
        return TRUE;
    }
    return FALSE;
}


/* This function checks if the addressing mode for the command is  legal.
 * If it does, return true. Otherwise, false. */
boolean is_valid_addressing_mode(const int modes[], int mode) {
    /* We need to check if we get a legal addressing mode. */
    if (mode < NUMBER_ZERO || mode > NUMBER_THREE) {
        return FALSE;
    }

    /* We check if the given addressing mode is legal according to the command's modes array.
     * If the bit is on- we return true because this is a legal addressing mode for the command. */
    return modes[mode] == NUMBER_ONE;
}

/* This function gets an operand from command and checks the addressing mode for him.
 * Returns the addressing mode for him:
 * immediate mode (0) ,direct mode (1), relative mode (2), register direct mode (3).
 * If we don't have a source or a destination operand for the command we return -1. */
int get_addressing_mode(char *operand) {
    if(operand == NULL || *operand =='\0') {
        return MISSING_OPERAND;
    }
    if(operand[NUMBER_ZERO] == '#') {
        return IMMEDIATE_MODE;
    }
    if (is_register(operand)) {
        return REGISTER_DIRECT_MODE;
    }
    if(operand[NUMBER_ZERO] == '%') {
        return RELATIVE_MODE;
    }
    return DIRECT_MODE;
}

/* This function extract the string, put it in the data array- data_image (if the string is legal) and update dc.
 * If we have extra text or missing quote we print error to the user and return false, otherwise, true. */
boolean extract_string_data(char **line_ptr,  AssemblerState *state) {
    skip_whitespaces(line_ptr);

    /* We check if the string start with " before the word.
     * If not, this is error, so we print an alert to the user. */
    if (**line_ptr != '\"') {
        fprintf(stderr, "Error line %d: Missing opening quote for .string directive\n", state->line_number);
        state->error_found = TRUE;
        return FALSE;
    }

    /* We continue to the next char - the first char on the string. */
    (*line_ptr)++;


     /* We put the string's chars on the data_image- each char is a new word in data_image.
     * so dc has to advance by one for each char. */
    while (**line_ptr != '\"' && **line_ptr != '\0' && **line_ptr != '\n') {

        /* We need to check if all the chars in the string are legal.
        * We use the function isprint() - it returns true if the char
        * can be printed on the screen (by ASCII values),
         * otherwise this is error, so we print an alert to the user. */
        if (!isprint(**line_ptr)) {
            fprintf(stderr, "Error line %d: Illegal (non-printable) character in string\n", state->line_number);
            state->error_found = TRUE;
            return FALSE;
        }

        /* We take the ASCII value for the char and make sure only the first 12 bits (0xFFF) will save. */
        state->data_image[state->dc].value = (unsigned int)**line_ptr & 0xFFF;

        /* .string is an instructional sentence so the type is A. */
        state->data_image[state->dc].are = ARE_ABSOLUTE;

        state->dc++;
        (*line_ptr)++;
    }

    /* We need to check if the last char on the string is: ".
     * If not, this is error, so we print an alert to the user. */
    if (**line_ptr != '\"') {
        fprintf(stderr, "Error line %d: Missing closing quote for .string directive\n", state->line_number);
        state->error_found = TRUE;
        return FALSE;
    }

    /* We need to close the last word for the string with zero. */
    state->data_image[state->dc].value = NUMBER_ZERO;
    state->data_image[state->dc].are = ARE_ABSOLUTE;
    state->dc++;

    (*line_ptr)++;
    skip_whitespaces(line_ptr);

    /* We check if we don't have text after the end of the string.
    * If we have, this is error, so we print an alert to the user. */
    if (**line_ptr != '\0' && **line_ptr != '\n') {
        fprintf(stderr, "Error line %d: Extraneous text after string in .string directive\n", state->line_number);
        state->error_found = TRUE;
        return FALSE;
    }

    return TRUE;
}

/* This function extract the data (from the .data sentence)
 * and put it in the data array- data_image (if the data is legal) and update dc.
 * If we are missing number or aren't putting ',' between numbers,
 * we print error to the user and return false. Otherwise, true. */
boolean extract_data(char **line_ptr,  AssemblerState *state) {
    char num_str[MAX_LINE_LENGTH];
    int num_value;
    /* There is at least one number in data sentence. */
    boolean expect_number = TRUE;

    boolean has_data = FALSE;


    while (**line_ptr != '\0' && **line_ptr != '\n') {
        skip_whitespaces(line_ptr);

        /* We need to check again if the line is over,
         * or if we need to move to the next line' because if we have a space,
         * we need to skip on it and then, we can be in the end of the line so we need to get out from the 'if'.  */
        if (**line_ptr == '\0' || **line_ptr == '\n') {
            break;
        }

        /* If we have ',' we need to check if it is on the right place-
         * if we are expecting to number and there is ','- this is an error:
         * we need to print to the user.*/
        if (**line_ptr == ',') {
            if (expect_number) {
                fprintf(stderr, "Error line %d: Unexpected comma or multiple commas in .data\n", state->line_number);
                state->error_found = TRUE;
                return FALSE;
            }
            (*line_ptr)++;

            // if we expected for ',' - in the next char we need to get a number.
            expect_number = TRUE;

            continue;
        }

        /* If we are expecting for ',' between the numbers.
         * If we don't have it- this is an error so we  need to print to the user. */
        if (!expect_number) {
            fprintf(stderr, "Error line %d: Expected comma between numbers in .data\n", state->line_number);
            state->error_found = TRUE;
            return FALSE;
        }

        /* Extracting the number */
        extract_word(line_ptr, num_str);

        if (num_str[NUMBER_ZERO] != '\0') {
            /* If the number isn't legal, this is an error - we need to print to the user.*/
            if (!is_legal_number(num_str)) {
                fprintf(stderr, "Error line %d: Invalid number '%s' in .data\n", state->line_number, num_str);
                state->error_found = TRUE;
                return FALSE;
            }
            num_value=atoi(num_str);
            if(!is_number_range(num_value)) {
                fprintf(stderr, "Error line %d: Number '%d' is out of the 12 bit range for number in .data\n", state->line_number, num_value);
            }

            /*  We put the number on the data_image- each number is a new word in data_image.
             * so dc has to advance by one for each number.
             * We take the number value for and make sure only the first 12 bits (0xFFF) will save. */
            state->data_image[state->dc].value = atoi(num_str) & 0xFFF;
            /* .data is an instructional sentence so the type is A. */
            state->data_image[state->dc].are = ARE_ABSOLUTE;
            state->dc++;

            expect_number = FALSE;
            has_data = TRUE;
        }
    }

    /* The line spouse to end with a number and not with ',' .
     * if we end the line with ',' - this is an error, and we need to print en alert to the user. */
    if (expect_number && has_data) {
        fprintf(stderr, "Error line %d: Trailing comma at the end of .data directive\n", state->line_number);
        state->error_found = TRUE;
        return FALSE;
    }
    return has_data;
}

/* This function check if we got all the operands we expected in the command and return true, otherwise false. */
boolean extract_operands(char **line_ptr, char *src, char *dst, int expected_ops, int line_number) {
    int index;
    skip_whitespaces(line_ptr);

    /* Expecting for two operands in the command */
    if (expected_ops == NUMBER_TWO) {
        /* We put the source operand in the src array char by char. */
        index = NUMBER_ZERO;
        while (**line_ptr && **line_ptr != ',' && **line_ptr != ' ' && **line_ptr != '\t' && **line_ptr != '\n')) {
            src[index++] = *(*line_ptr)++;
        }
        src[index] = '\0';

        skip_whitespaces(line_ptr);

        /* We check if we have ',' between the two operands.
        * If we don't- this is an error, we print an alert to the user. */
        if (**line_ptr != ',') {
            fprintf(stderr, "Error line %d: Missing comma between operands\n", line_number);
            return FALSE;
        }

        /* We continue to the next operand. */
        (*line_ptr)++;
        skip_whitespaces(line_ptr);

        /* We put the destination operand in the dst array char by char. */
        index= NUMBER_ZERO;
        while (**line_ptr && **line_ptr != ',' && **line_ptr != ' ' && **line_ptr != '\t' && **line_ptr != '\n') {
            dst[index++] = *(*line_ptr)++;
        }
        dst[index] = '\0';

        /* Missing operand - this is an error, we print an alert to the user. */
        if (strlen(src) == NUMBER_ZERO || strlen(dst) == NUMBER_ZERO) {
            fprintf(stderr, "Error line %d: Missing operand(s)\n", line_number);
            return FALSE;
        }
    }
    /* Expecting for one operand in the command */
    else if (expected_ops == NUMBER_ONE) {
        index = NUMBER_ZERO;
        while (**line_ptr && **line_ptr != ',' && **line_ptr != ' ' && **line_ptr != '\t' && **line_ptr != '\n')) {
            dst[index++] = *(*line_ptr)++;
        }
        dst[index] = '\0';

        /* Missing operand - this is an error, we print an alert to the user. */
        if (strlen(dst) == NUMBER_ZERO) {
            fprintf(stderr, "Error line %d: Missing operand\n", line_number);
            return FALSE;
        }
    }
    skip_whitespaces(line_ptr);

    /* We check if after the operands, we don't have other text.
     * if we have - this is an error, we print an alert to the user. */
    if (**line_ptr != '\0' && **line_ptr != '\n') {
        fprintf(stderr, "Error line %d: Extraneous text after operands\n", line_number);
        return FALSE;
    }
    /* WE will get here also if we have zero operands for the command (for commands that expect to zero operands).  */
    return TRUE;
}

