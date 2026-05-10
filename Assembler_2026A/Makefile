# Makefile according to the flags required in the course.
# project files we need to compile.
assembler:
	gcc -Wall -ansi -pedantic \
	src/assembler.c \
	src/passes/pre_assembler.c \
	src/passes/first_pass.c \
	src/passes/second_pass.c \
	src/tables/assembler_tables.c \
	src/tables/macro_table.c \
	src/tables/symbol_table.c \
	src/globals/helpers.c \
	src/globals/parser.c \
	outputs/build_output_file.c \
	-o assembler

# To clean makefile.
clean:
	rm -f assembler
