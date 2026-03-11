/* pre-assembler header file */

#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include "constant.h"

/* We pass the function a pointer to the file, the original name (no extension),
 * and a pointer to struct from "AssemblerState" type.
 * The purpose of this function is to read the original file context (.as), 
 * and find macros, in order to create a new output file (.am), in the
 * output file, the macros will be switched and written as commands.
 */
boolean pre_assemble(FILE *source_file, char *original_name, AssemblerState *context);

#endif
