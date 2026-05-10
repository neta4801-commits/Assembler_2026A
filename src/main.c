/*
 * This is an assembler project for MAMAN14 2026A in open university,
 * by Neta Cohen 213178023 and Ido Issaki 206592297.
 */


/* This file - main.c, will be the main file for our project */
#include <stdio.h>
#include <stdlib.h>
#include "globals/constant.h"
#include "globals/helpers.h"
#include "tables/symbol_table.h"
#include "passes/pre_assembler.h"
#include "passes/first_pass.h"
#include "passes/second_pass.h"

/* This function handles some user errors, and will provide output for successful compilation,
 * calling pre_assembler, to check if the am file was generates successfully,
 * initializing assembler context (IC,DC ...). */
int main (int argc, char *argv[]) {
    int i;
    FILE *source_file;
    char *file_name;
    AssemblerState current_context;
    extern_ptr extern_head;
    char *am_file_name;
    FILE *am_file;
  
  /* Check if user entered input file names */
    if (argc < NUMBER_TWO) {
        fprintf(stdout, "Error: no input files provided. \n");
        return EXIT_FAILURE;
    }
    
 /* create a new file name using argv and ".as" extension, open to read */
    for (i = NUMBER_ONE; i < argc; i++) {
        file_name = create_file_name(argv[i], ".as");
        source_file = fopen(file_name, "r");
        
        if (source_file == NULL) {
            fprintf(stdout, "Error: Cannot open file '%s'.\n", file_name);
            free(file_name);
            continue;
        }

        /* initializing context, as mentioned above */
        current_context.ic = IC_START; 
        current_context.dc = NUMBER_ZERO;
        current_context.line_number = NUMBER_ONE;
        current_context.error_found = FALSE;
        current_context.symbol_head = NULL;
        extern_head = NULL;

        printf("Starting pre assembler for file: %s\n", file_name);
        
        /* Spreading the macros */
        if (pre_assemble(source_file, argv[i], &current_context)) {
            printf("Success! Created %s.am\n", argv[i]);

            /* First Pass and Second Pass Execution */
            am_file_name = create_file_name(argv[i], ".am");
            am_file = fopen(am_file_name, "r");

            if (am_file != NULL) {
                printf("Starting first pass for file %s\n", am_file_name);
                
                if (first_pass(am_file, &current_context)) {
                    printf("Starting second pass for file %s\n", am_file_name);

                    /* run_second_pass generates the .ob, .ent, and .ext files */
                    if (run_second_pass(am_file, argv[i], &current_context, &extern_head)) {
                        printf("Successfully compiled %s\n", argv[i]);
                    } else {
                        printf("Failed: Errors found during second pass in %s\n", am_file_name);
                    }
                } else {
                    printf("Failed: Errors found during first pass in %s\n", am_file_name);
                }

                fclose(am_file);
            } else {
                fprintf(stdout, "Error: Cannot open file %s for reading.\n", am_file_name);
            }
            free(am_file_name);

        } else {
            printf("Failed: Errors found during pre assembler in %s\n", file_name);
        }

        /* Memory cleanup for symbols and externs between different files */
        if (current_context.symbol_head != NULL) {
            free_symbols(current_context.symbol_head);
            current_context.symbol_head = NULL; /* Reset back to NULL after freeing */
        }
        if (extern_head != NULL) {
            free_extern_list(extern_head);
            extern_head = NULL; /*  Reset back to NULL after freeing */
        }

        fclose(source_file);
        free(file_name);
    }

    return EXIT_SUCCESS;
}
