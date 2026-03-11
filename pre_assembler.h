/* pre-assembler header file */

#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h>
#include "constant.h"

/* We pass the function a pointer to the file, the base file name (no extension),
 * and a pointer to struct type "AssemberState"
 * The purpose of this function is to read the original file context, (.as), 
 * and find for macros, in order to create a new output file (.am), in that
 * .am file, the macros will be switched and written as commands.
 */
boolean pre_assemble(FILE *source_file, char *base_file_name, AssemblerState *context);

#endif
