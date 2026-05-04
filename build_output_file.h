/*The heder file for output file */

#ifndef BUILD_OUTPUT_FILE_H
#define BUILD_OUTPUT_FILE_H

#include "src/globals/constant.h"

/* This function creates three output files- ob, ent and ext files. */
void build_output_files(char *original_name, AssemblerState *state, extern_ptr extern_head);

#endif


