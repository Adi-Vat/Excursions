LD R0, 5
LD R1, 1
LD R2, 1
LD R3, 0

loop:
    LD R3, R2
    ADD R2, R1
    LD R1, R3

    DEC R0
    JNZ loop
