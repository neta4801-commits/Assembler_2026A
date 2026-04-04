/* constant header file */
/* This file includes constants and structs that used in several files  */

#ifndef CONSTANT_H
#define CONSTANT_H

/* constant numbers */
#define NUMBER_ZERO 0
#define NUMBER_ONE 1
#define NUMBER_TWO 2

/* Constant values according to course manual. */
#define MAX_LINE_LENGTH 80
/* MAX_LABEL_LENGTH = Max length for label and also for macro names. */
#define MAX_LABEL_LENGTH 31
#define MEMORY_SIZE 4096
#define IC_START 100
#define NUM_COMMANDS 16


typedef enum {
    FALSE = 0,
    TRUE = 1
} boolean;


/* The function enum gives 'A'=0, 'R'=1, 'E'=2 */
typedef enum {
    ARE_ABSOLUTE = 'A',
    ARE_RELOCATABLE = 'R',
    ARE_EXTERNAL = 'E'
} command_type_are;

/* value - Each machine word is 12 bits so we use short (16 bits) to store the commands machine code.
 * are - Gives us command type- A,R,E (1 bit from the 16 bits) */
typedef struct {
    unsigned short value;
    command_type_are are;
} machine_word;

/* Each command has her own name, opcode and funct. */
typedef struct {
    char *command_name;
    unsigned int opcode;
    unsigned int funct;
} command_info;


 /* represents a single symbol from the assembly source code, stores label name,memory address and type. */
typedef struct symbol_node {
    char label_name[MAX_LABEL_LENGTH + NUMBER_ONE];
    int label_address;
    unsigned int is_code   : NUMBER_ONE;
    unsigned int is_data   : NUMBER_ONE;
    unsigned int is_entry  : NUMBER_ONE;
    unsigned int is_extern : NUMBER_ONE;
    struct symbol_node *next;
} symbol_node;

/* A pointer to the symbol list. */
typedef symbol_node *symbol_ptr;

/* Manages current state of the assembly process,
 * track IC/DC, store machine code and data, manages the symbol table and flags for errors. */
typedef struct {
    int ic;
    int dc;
    int line_number;
    boolean error_found;
    machine_word code_image[MEMORY_SIZE];
    machine_word data_image[MEMORY_SIZE];
    symbol_ptr symbol_head;
} AssemblerState;

#endif
