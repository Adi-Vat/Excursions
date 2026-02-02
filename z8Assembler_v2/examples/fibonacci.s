LD R0, 5    ; R0 = 5
LD R1, 1    ; R1 = 1
LD R2, 1    ; R2 = 1
LD R3, 0    ; R3 = 0

loop:
    LD R3, R2   ; R3 = R2
    ADD R2, R1  ; R2 = R1
    LD R1, R3   ; R1 = R3

    DEC R0  ; R0--
    JNZ loop ; if(R0 != 0) goto loop
