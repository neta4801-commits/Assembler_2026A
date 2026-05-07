CC = gcc
CFLAGS = -Wall -ansi -pedantic -g

# כל קבצי המקור של הפרויקט
SRCS = src/main.c \
       src/globals/helpers.c \
       src/globals/parser.c \
       src/passes/pre_assembler.c \
       src/passes/first_pass.c \
       src/passes/second_pass.c \
       src/tables/assembler_tables.c \
       src/tables/macro_table.c \
       src/tables/symbol_table.c \
       outputs/build_output_file.c

# פקודת הקימפול היחידה
assembler:
	$(CC) $(CFLAGS) $(SRCS) -o assembler

# ניקוי
clean:
	rm -f assembler