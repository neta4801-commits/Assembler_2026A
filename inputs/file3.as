cmp val1, #-6
 mcro string
 lea STR, r3
 inc r3
 mov r4, K
mcroend
 bne %END
 dec K
 jmp %LOOP
 GEN_MC
 sub
 sub
END: stop
STR: .string "abcd"
LIST: .data 6, -9
 .data -100
.entry K
K: .data 31
.extern val
