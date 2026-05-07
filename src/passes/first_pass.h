/* The heder file for first pass */

#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include <stdio.h>
#include "../globals/constant.h"

/*
 * This function gets the am_file and the assembler's state.
 * it executes the first pass algorithm according to the project instructions:
 * builds us the symbol table, updates the counters ic (for instruction lines) and dc (for datas).
 * adds the code words and the data words to tables.
 * If the first pass is successful, returns true - no errors were found. Otherwise, false and error alerts to the user.
 */
boolean first_pass(FILE *am_file, AssemblerState *state);

#endif
