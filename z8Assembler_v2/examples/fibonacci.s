LD R0, 5    ; Loop counter
LD R1, 1    ; Previous Fibonacci number
LD R2, 1    ; Current Fibonacci number
LD R3, 0    ; Temporary storage
.var final_output 0 ; final value

loop:
    LD R3, R2   ; Save current value
    ADD R2, R1  ; Calculate next Fibonacci: R2 = R1 + R2
    LD R1, R3   ; Shift: previous = old current value

    DEC R0  ; Decrement counter
    JNZ loop ; Continue if counter != 0

STR @final_output, R2

; Result: final_output = 13 (0x0D)
