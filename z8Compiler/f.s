.fibonacci
    LDD y, 5
    LDD a, 1
    LDD x, 1
    LDD z, 0
.loop
    LDD z, a
    ADC x
    LDD x, z

    OUT a

    DEC y
    JNZ .loop

