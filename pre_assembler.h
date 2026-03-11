/* pre-assembler header file */

#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include "globals.h"

/*
 *
 * 
 * 
 * 
 * 
 * 
 */
boolean pre_assemble(FILE *source_file, const char *base_file_name, AssemblerContext *context);

#endif
