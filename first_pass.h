//
// Created by Neta Cohen on 02/05/2026.
//

#ifndef ASSEMBLER_2026A_FIRST_PASS_H
#define ASSEMBLER_2026A_FIRST_PASS_H

#include <stdio.h>
#include "src/globals/constant.h"

/*
 * This function gets the -am file and the state for the assembler
 * update ic and dc and
 * */
boolean first_pass(FILE *am_file, AssemblerState *state);

#endif
