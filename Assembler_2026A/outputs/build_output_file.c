/* This file builds the output files (ob, ent and ext) and prints the words. */

#include <stdio.h>
#include <stdlib.h>
#include "build_output_file.h"
#include "../src/globals/helpers.h"

/* This function print the machine words (form code image and data image) to the file.
 * the print is according to the project instruction -
 * 4 digit for address (%04d), 3 digit for machine word value (Hexadecimal- %03X).
 * 0xFFF- is 12 bits in hexadecimal and its makes sure we don't take more than 12 bits for the machine word. */
static void print_machine_word(FILE *file, int address, machine_word word) {
    fprintf(file, "%04d %03X %c\n", address, word.value & 0xFFF, word.are);
}


/* This function creates three output files- ob, ent and ext files. */
void build_output_files(char *original_name, AssemblerState *state, extern_ptr extern_head) {
    char *ob_name, *entry_name, *extern_name;
    FILE *ob_file, *ent_file, *ext_file;
    int index;
    symbol_ptr current_symbol;
    extern_ptr current_extern;
    boolean is_current_entry = FALSE;

    /*
     *  Creating ob.file.
     *  The first line in the file describes the number of data words and machine words.
     *  we use the function build_output_files in order to print the machine words form code_image and data image.
     *
    */
    ob_name = create_file_name(original_name, ".ob");
    ob_file = fopen(ob_name, "w");
    if (ob_file) {
        fprintf(ob_file, "  %d %d\n", state->ic - IC_START, state->dc);

        for (index = NUMBER_ZERO; index < state->ic - IC_START; index++) {
            print_machine_word(ob_file, IC_START + index, state->code_image[index]);
        }

        for (index = NUMBER_ZERO; index < state->dc; index++) {
            print_machine_word(ob_file, state->ic + index, state->data_image[index]);
        }

        fclose(ob_file);
    }
    else{
        fprintf(stdout, "Error: Cannot create the output file %s\n", ob_name);
    }

    free(ob_name);


    /*
     *  Creating the entry file (if there is at least one symbol with entry type).
     *  we print all the symbols with entry type from symbol list -
     *  their names and their addresses (we print 4 digit for their addresses).
    */
    current_symbol = state->symbol_head;
    while (current_symbol) {
        if (current_symbol->is_entry) {
            is_current_entry = TRUE;
            break;
        }
        current_symbol = current_symbol->next;
    }

    if (is_current_entry) {
        entry_name = create_file_name(original_name, ".ent");
        ent_file = fopen(entry_name, "w");
        if (ent_file) {
            current_symbol = state->symbol_head;
            while (current_symbol) {
                if (current_symbol->is_entry) {
                    fprintf(ent_file, "%s %04d\n", current_symbol->label_name, current_symbol->label_address);
                }
                current_symbol = current_symbol->next;
            }
            fclose(ent_file);
        }
        else{
            fprintf(stdout, "Error: Cannot create the output file %s\n", entry_name);
        }
        free(entry_name);
    }


    /*
     *  Creating the extern file (if there is at least one symbol with extern type).
     *  we print all the symbols with extern type from extern list
     *  (this list include all the symbols with extern type and all the addresses the user uses them) -
     *  their names and their addresses (we print 4 digit for their addresses).
    */
    if (extern_head != NULL) {
        extern_name = create_file_name(original_name, ".ext");
        ext_file = fopen(extern_name, "w");
        if (ext_file) {
            current_extern = extern_head;
            while (current_extern) {
                fprintf(ext_file, "%s %04d\n", current_extern->name, current_extern->address);
                current_extern = current_extern->next;
            }
            fclose(ext_file);
        }
        else{
            fprintf(stdout, "Error: Cannot create the output file %s\n", extern_name);
        }
        free(extern_name);
    }
}
