#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.h"

#define DATA_MEM_SIZE 256

int data_mem[DATA_MEM_SIZE];
int regs[REGISTERS_COUNT];

typedef struct{
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
} Seven_Seg;

Seven_Seg seven_seg_displays[6];

typedef struct{
    bool zero;
    bool carry;
    bool overflow;
    bool negative;
} Flags;

typedef struct{
    int opcode;
    int dest;
    int source;
} Instruction;

typedef enum{
    ALU_ADD,
    ALU_SUB, 
    ALU_INC, 
    ALU_DEC,
    ALU_AND, 
    ALU_OR, 
    ALU_XOR,
    ALU_CMP, 
    ALU_SB, 
    ALU_CB 
} ALU_Ops;

int alu(ALU_Ops alu_op, int in_a, int in_b, Flags* new_flags);

int main(int argc, char *argv[]){
    // check if number of arguments given exceeds number of arguments expected
    if(argc != 2){
        printf("Error: Correct usage is '%s <hex file>'\n", argv[0]);
        return 1;
    }

    // open hex file
    FILE *hex_file_ptr;
    hex_file_ptr = fopen(argv[1], "r");
    
    if(hex_file_ptr == NULL){
        printf("Error: '%s' does not exist", argv[1]);
        return 1;
    }

    // initialize memory
    for(int i = 0; i < DATA_MEM_SIZE; i++) data_mem[i] = 0;
    for(int i = 0; i < REGISTERS_COUNT; i++) regs[i] = 0;

    // setup flags
    Flags flags;
    flags.carry = false;
    flags.negative = false;
    flags.overflow = false;
    flags.zero = false;

    char hex_line[MACHINE_CODE_INSTR_LEN + 1] = {0};
    int program_length = 0;

    for(char c = getc(hex_file_ptr); c != EOF; c = getc(hex_file_ptr))
        if(c == '\n') program_length++;
    

    Instruction prog_mem[program_length];

    int line_num = 0;

    rewind(hex_file_ptr);

    while(fgets(hex_line, MACHINE_CODE_INSTR_LEN + 1, hex_file_ptr)){
        if(empty_line(hex_line)) continue;
        // --- FETCH ---
        char opcode[OPCODE_LEN + 1] = {0};
        char operand_a[OPERAND_LEN + 1] = {0};
        char operand_b[OPERAND_LEN + 1] = {0};
        // split up instruction
        memcpy(opcode, hex_line, OPCODE_LEN);
        opcode[OPCODE_LEN] = '\0';

        memcpy(operand_a, hex_line + OPCODE_LEN, OPERAND_LEN);
        operand_a[OPERAND_LEN] = '\0';
    
        memcpy(operand_b, hex_line + OPCODE_LEN + OPERAND_LEN, OPERAND_LEN);
        operand_a[OPERAND_LEN] = '\0';
        
        long int opcode_num = 0;
        char * end_ptr;
        // convert string (hex) to number (! no error checking lmao)
        opcode_num = strtol(opcode, &end_ptr, 16);
        
        long int dest = 0;
        char *end_ptr_dest;
        dest = strtol(operand_a, &end_ptr_dest, 16);
        
        long int source = 0;
        char *end_ptr_source;
        source = strtol(operand_b, &end_ptr_source, 16);

        prog_mem[line_num].opcode = opcode_num;
        prog_mem[line_num].dest = dest;
        prog_mem[line_num].source = source;

        line_num ++;
    }

    
    bool end_program = false;
    int pc = 0;
    int stack_ptr = STACK_START;

    while(!end_program){
        int opcode_num = prog_mem[pc].opcode;
        int dest = prog_mem[pc].dest;
        int source = prog_mem[pc].source;

        // get enum
        Emulator_Operations operation = (Emulator_Operations)opcode_num;
        int next_pc = pc + 1;
        
        // --- DECODE, EXECUTE, WRITEBACK---
        switch(operation){
            case EMU_LDM:
                // no bounds or range checking... i'm really trusting my assembler
                regs[dest] = data_mem[source];
            break;
            case EMU_LDR:
                regs[dest] = regs[source];
            break;
            case EMU_LDD:
                regs[dest] = source;
            break;
            case EMU_STR:
                data_mem[dest] = regs[source];
            break;
            case EMU_STD:
                data_mem[dest] = source;
            break;
            case EMU_ADR:
                regs[dest] = alu(ALU_ADD, regs[dest], regs[source], &flags);
            break;
            case EMU_ADD:
                regs[dest] = alu(ALU_ADD, regs[dest], source, &flags);
            break;
            case EMU_SBR:
                regs[dest] = alu(ALU_SUB, regs[dest], regs[source], &flags);
            break;
            case EMU_SBD:
                regs[dest] = alu(ALU_SUB, regs[dest], source, &flags);
            break;
            case EMU_INC:
                regs[dest] = alu(ALU_INC, regs[dest], 0, &flags);
            break;
            case EMU_DEC:
                regs[dest] = alu(ALU_DEC, regs[dest], 0, &flags);
            break;
            case EMU_ANR:
                regs[dest] = alu(ALU_AND, regs[dest], regs[source], &flags);
            break;
            case EMU_AND:
                regs[dest] = alu(ALU_AND, regs[dest], source, &flags);
            break;
            case EMU_ORR:
                regs[dest] = alu(ALU_OR, regs[dest], regs[source], &flags);
            break;
            case EMU_ORD:
                regs[dest] = alu(ALU_OR, regs[dest], source, &flags);
            break;
            case EMU_XOR:
                regs[dest] = alu(ALU_XOR, regs[dest], regs[source], &flags);
            break;
            case EMU_XOD:
                regs[dest] = alu(ALU_XOR, regs[dest], source, &flags);
            break;
            case EMU_CPR:
                alu(ALU_CMP, regs[dest], regs[source], &flags);
            break;
            case EMU_CPD:
                alu(ALU_CMP, regs[dest], source, &flags);
            break;
            case EMU_JPR:
                next_pc = regs[dest];
            break;
            case EMU_JPD:
                next_pc = dest;
            break;
            case EMU_JZR:
                if(flags.zero) next_pc = regs[dest];
            break;
            case EMU_JZD:
                if(flags.zero) next_pc = dest;
            break;
            case EMU_JNZR:
                if(!flags.zero) next_pc = regs[dest];
            break;
            case EMU_JNZD:
                if(!flags.zero) next_pc = dest;
            break;
            case EMU_JNR:
                if(flags.negative) next_pc = regs[dest];
            break;
            case EMU_JND:
                if(flags.negative) next_pc = dest;
            break;
            case EMU_PSHR:
                data_mem[stack_ptr] = regs[dest];
                if(stack_ptr > STACK_END) stack_ptr--;
            break;
            case EMU_PSHD:
                data_mem[stack_ptr] = dest;
                if(stack_ptr > STACK_END) stack_ptr--;
            break;
            case EMU_POP:
                if(stack_ptr < STACK_START) stack_ptr ++;
                regs[dest] = data_mem[stack_ptr];
            break;
            case EMU_HALT:
                end_program = true;
            break;
            case EMU_SBIM:
                data_mem[source] = alu(ALU_SB, dest, data_mem[source], &flags);
            break;
            case EMU_SBIR:
                regs[source] = alu(ALU_SB, dest, regs[source], &flags);
            break;
            case EMU_CBIM:
                data_mem[source] = alu(ALU_CB, dest, data_mem[source], &flags);
            break;
            case EMU_CBIR:
                regs[source] = alu(ALU_CB, dest, regs[source], &flags);
            break;
            default:
                printf("ERR: UNEXPECTED OPERATION\n");
            break;
        }
        pc = next_pc;
        if(pc > program_length) end_program = true;
    }

    for(int i = 0; i < REGISTERS_COUNT; i++) printf("R%d: %02X\n", i, regs[i]);
    for(int y = 0; y < 16; y++){
        for(int x = 0; x < 16; x++){
            int address = y * 16 + x;
            char stack_ptr_char = ' ';
            if(address == stack_ptr) stack_ptr_char = '>'; 
            printf("%c|%02X:%02X|", stack_ptr_char, address, data_mem[address]);
        }
        printf("\n");
    }
    
    printf("Overflow: %d\n", flags.overflow);
    printf("Carry: %d\n", flags.carry);
    printf("Negative: %d\n", flags.negative);
    printf("Zero: %d\n", flags.zero);
    printf("Stack Pointer: %02X", stack_ptr);
    printf("\n");

    // check bank
    int bit_mask = 1;
    if(data_mem[0] & bit_mask == 1){
        // bank 1 being displayed form
        
    }

    for(int i = 0; i < 6; i++){
        char out_char = '-';
        if(seven_seg_displays[i].a == 1) out_char = '=';
        printf(" %c%c ", out_char, out_char);
        if(i%2 == 1) printf(" ");
    }
    printf("\n");
    for(int i = 0; i < 6; i++){
        char out_char_1 = '|';
        char out_char_2 = '|';
        if(seven_seg_displays[i].f == 1) out_char_1 = '#';
        if(seven_seg_displays[i].b == 1) out_char_2 = '#';
        printf("%c  %c", out_char_1, out_char_2);
        if(i%2 == 1) printf(" ");
    }
    printf("\n");
    for(int i = 0; i < 6; i++){
        char out_char = '-';
        if(seven_seg_displays[i].g == 1) out_char = '=';
        printf(" %c%c ", out_char, out_char);
        if(i%2 == 1) printf(" ");
    }
    printf("\n");
    for(int i = 0; i < 6; i++){
        char out_char_1 = '|';
        char out_char_2 = '|';
        if(seven_seg_displays[i].c == 1) out_char_1 = '#';
        if(seven_seg_displays[i].e == 1) out_char_2 = '#';
        printf("%c  %c", out_char_1, out_char_2);
        if(i%2 == 1) printf(" ");
    }
    printf("\n");
    for(int i = 0; i < 6; i++){
        char out_char = '-';
        if(seven_seg_displays[i].d == 1) out_char = '=';
        printf(" %c%c ", out_char, out_char);
        if(i%2 == 1) printf(" ");
    }
    return 0; 
}

int alu(ALU_Ops alu_op, int in_a, int in_b, Flags* new_flags){
    int out = 0;
    switch(alu_op){
        case ALU_ADD:
            out = in_a + in_b;
        break;
        case ALU_SUB:
            out = in_a - in_b;
        break;
        case ALU_INC:
            out = in_a + 1;
        break;
        case ALU_DEC:
            out = in_a - 1;
        break;
        case ALU_CMP:
            out = in_a - in_b;
        break;
        case ALU_AND:
            out = in_a & in_b;
        break;
        case ALU_OR:
            out = in_a | in_b;
        break;
        case ALU_XOR:
            out = in_a ^ in_b;
        break;
        case ALU_SB:
            out = (1 << in_a) | in_b;
        break;
        case ALU_CB:
            out = ~(1 << in_a) & in_b;
        break;
    }

    // negative
    if(out < 0) new_flags->negative = true;
    
    // carry
    if(out > MAX_VALUE/2 - 1 || out < 0) new_flags->carry = true;

    // zero
    if(out == 0) new_flags->zero = true;

    // overflow
    if(out > MAX_VALUE/2 || out < -(MAX_VALUE/2)) new_flags->overflow = true;

    return out;
}