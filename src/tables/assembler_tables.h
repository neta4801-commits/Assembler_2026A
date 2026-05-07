/* commands_table header file */

#ifndef ASSEMBLER_TABLE_H
#define ASSEMBLER_TABLE_H

#include "../globals/constant.h"

/* This function is const because those are commands data from the project instructions that we can't change.
 * This function gets a command name and checks if this is a legal command:
 * if it does, returns a pointer to this command that including its information - command name,opcode, funct.
 * Else, returns null. */
const command_info *get_command(char *command_name);

/* This function gets a register name and checks if this is a legal name for register:
 * if it does, returns true. Otherwise, false. */
boolean is_register(char *register_name);

#endif
