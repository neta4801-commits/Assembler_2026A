/* The header file for second pass */

#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include <stdio.h>
#include "src/globals/constant.h"

/* Executes second pass and, on success, builds output files (.ob/.ent/.ext). */
boolean run_second_pass(FILE *am_file, char *original_name, AssemblerState *state, extern_ptr *extern_head);

/* Executes second pass only: resolves symbols, handles .entry and extern usages. */
boolean second_pass(FILE *am_file, AssemblerState *state, extern_ptr *extern_head);

/* Frees all allocated memory in extern-usage list. */
void free_extern_list(extern_ptr head);

#endif
