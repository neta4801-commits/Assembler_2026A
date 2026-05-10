/* pre-assembler header file */

#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include "../globals/constant.h"

/* We pass the function a pointer to the file, the original name (no extension),
 * and a pointer to struct from "AssemblerState" type.
 * The purpose of this function is to read the original file context (.as), 
 * and find macros, in order to create a new output file (.am), in the
 * output file, the macros will be switched and written as commands.
 * In addition, we are adding labels to our label list,
 * and check if there are conflicts between label names and maco names.
 */
boolean pre_assemble(FILE *source_file, char *original_name, AssemblerState *context);

#endif
