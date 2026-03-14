/* Main.c, this file will be the main file for our project */

#include <stdio.h>
#include <stdlib.h>
#include "constats.h"
#include "helpers.h"
#include "pre_assembler.h"

/* This function handles some user errors, and will provide output for successful compilation,
 * calling pre_assembler, to check if the am file was generates successfully,
 * initializing assembler context (IC,DC ...). */
int main (int argc, char *argv[]) {
  int i;
  FILE *source_file;
  char *file_name;
  AssemblerStatee current_context;
  
  /* Check if user entered input file names */
  if (argc < NUMBER_TWO) {
    fprintf(stderr, "Error: no input files provided") 
        return EXIT_FAILURE;
    }
    
 /* create a new file name using argv and ".as" extension, open to read */
    for (i = NUMBER_ONE; i < argc; i++) {
        file_name = create_file_name(argv[i], ".as");
        source_file = fopen(file_name, "r");
        
        if (source_file == NULL) {
            fprintf(stderr, "Error: Cannot open file '%s'. Skipping to next.\n", file_name);
            free(file_name);
            continue;
        }

        /* initializing context, as mentioned above */
        current_context.ic = INITIAL_IC; 
        current_context.dc = NUMBER_ZERO;
        current_context.line_number = NUMBER_ONE;
        current_context.error_found = FALSE;
        current_context.symbol_head = NULL;

        printf("Processing macros for file: %s\n", file_name);
        
        /* Spreading the macros */
        if (pre_assemble(source_file, argv[i], &current_context)) {
            printf("Success! created %s.am\n", argv[i]);
        } else {
            printf("Failed: Errors found in %s\n", file_name);
        }

        fclose(source_file);
        free(file_name);
    }

    return EXIT_SUCCESS;
}
