/* This file has the tables for the commands and the registers. */

#include <string.h>
#include "assembler_tables.h"

/* Constant */
#define LENGTH_FOR_REGISTERS 8

/* Opcodes FOR COMMANDS */
#define OPCODE_MOV 0
#define OPCODE_CMP 1
#define OPCODE_GROUP_2 2 /* for commands - add, sub */
#define OPCODE_LEA 4
#define OPCODE_GROUP_5 5 /* for commands- clr, not, inc, dec */
#define OPCODE_GROUP_9 9 /* for commands- jmp, bne, jsr */
#define OPCODE_RED 12
#define OPCODE_PRN 13
#define OPCODE_RTS 14
#define OPCODE_STOP 15

/* Funct values */
#define FUNCT_ZERO 0
#define FUNCT_ADD 10
#define FUNCT_SUB 11
#define FUNCT_CLR 10
#define FUNCT_NOT 11
#define FUNCT_INC 12
#define FUNCT_DEC 13
#define FUNCT_JMP 10
#define FUNCT_BNE 11
#define FUNCT_JSR 12

/* The number of operand for each command */
#define ZERO_OPERANDS 0
#define ONE_OPERAND 1
#define TWO_OPERANDS 2


/* commands_table- the const table for our commands according to our instructions project.
 * Any command contains information: command name, opcode (in base 10), funct (in base 10), number of operand and
 * types of addressing modes according to instructions project (immediate, direct, relative,  register direct)-
 * for the 'source operand' and the 'destination operand'.
 * TRUE= the addressing mode EXISTS and FALSE= the addressing mode NOT EXISTS (TRUE=1, FALSE=0).
 * The table is static because we don't want other files to have access to it. */
static const command_info commands_table[NUM_COMMANDS] = {
        {"mov", OPCODE_MOV, FUNCT_ZERO, TWO_OPERANDS,  {TRUE, TRUE, FALSE, TRUE} , {FALSE, TRUE, FALSE, TRUE} },
        {"cmp",OPCODE_CMP , FUNCT_ZERO, TWO_OPERANDS, {TRUE, TRUE , FALSE, TRUE} , {TRUE, TRUE, FALSE, TRUE}  },
        {"add", OPCODE_GROUP_2, FUNCT_ADD, TWO_OPERANDS, {TRUE,  TRUE,  FALSE, TRUE} , {FALSE, TRUE,  FALSE, TRUE}  },
        {"sub", OPCODE_GROUP_2 , FUNCT_SUB, TWO_OPERANDS, {TRUE,  TRUE,  FALSE, TRUE }, {FALSE, TRUE,  FALSE, TRUE} },
        {"lea", OPCODE_LEA, FUNCT_ZERO, TWO_OPERANDS, {FALSE, TRUE,  FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"clr", OPCODE_GROUP_5, FUNCT_CLR, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"not", OPCODE_GROUP_5, FUNCT_NOT, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"inc", OPCODE_GROUP_5, FUNCT_INC, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"dec", OPCODE_GROUP_5, FUNCT_DEC, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"jmp", OPCODE_GROUP_9, FUNCT_JMP, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  FALSE} },
        {"bne", OPCODE_GROUP_9, FUNCT_BNE, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  FALSE} },
        {"jsr", OPCODE_GROUP_9, FUNCT_JSR, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  FALSE} },
        {"red", OPCODE_RED, FUNCT_ZERO, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  FALSE, TRUE} },
        {"prn", OPCODE_PRN, FUNCT_ZERO, ONE_OPERAND, {FALSE, FALSE, FALSE, FALSE}, {TRUE,  TRUE,  FALSE, TRUE} },
        {"rts", OPCODE_RTS, FUNCT_ZERO, ZERO_OPERANDS, {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE} },
        {"stop", OPCODE_STOP, FUNCT_ZERO, ZERO_OPERANDS, {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE} }
};


/* registers_table- the const table for our registers according to our instructions project (r0-r7).
 * The table is static because we don't want other files to have access to it. */
static const char *registers_table[LENGTH_FOR_REGISTERS] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};


 /* This function gets a command name and checks if this is a legal command:
 * if it does, returns a pointer to this command that including its information - command name,opcode, funct.
 * Else, returns null.
 * This function returns a pointer to const command data from the project instructions that we can't change. */
const command_info *get_command(char *command_name) {
    int i;
    if (command_name == NULL) {
        return NULL;
    }

    /* We compare the command name we got with each command name from the commands_table,
     * if we find match - we return the address to this command cell. Otherwise, null. */
    for (i = NUMBER_ZERO; i < NUM_COMMANDS; i++) {
        if (strcmp(commands_table[i].command_name, command_name) == NUMBER_ZERO)  {
            return &commands_table[i];
        }
    }
    return NULL;
}


/* This function gets a register name and checks (with the function compare()) if this is a legal name for register:
 * if it does, returns true. Otherwise, false. */
boolean is_register(char *register_name) {
    int i;
    if (register_name == NULL){
        return FALSE;
    }
    for (i = NUMBER_ZERO; i < LENGTH_FOR_REGISTERS; i++) {
        if (strcmp(registers_table[i], register_name) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}
