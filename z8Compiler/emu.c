#include <stdio.h>
#include <string.h>
#include "shared.h"
#include <stdbool.h>

#define DATA_MEMORY_SIZE 256
#define PROGRAM_MEMORY_SIZE 256
#define ACCUMULATOR 164

enum OPCODES{
    NOP,
    LDD,
    LDD_I,
    LDR,
    STR,
    STR_I,
    ADC,
    ADC_I,
    SUB,
    SUB_I,
    INC,
    DEC,
    ROL,
    ROR,
    SLL,
    SLA,
    SRL,
    SRA,
    PSH,
    POP,
    JMP,
    CMP,
    CMP_I,
    JZ,
    JNZ,
    JN,
    SB,
    CB,
    OUT
};

int data_memory[DATA_MEMORY_SIZE];
char program_memory[PROGRAM_MEMORY_SIZE][11];
int programCounter = 0;
bool zeroFlag;
bool negativeFlag;

int breakDownInstruction(char instruction[], enum OPCODES* opcode, int* operandA, int* operandB);
int executeInstruction(enum OPCODES opcodeSymbol, int operandADecimal, int operandBDecimal);

int main(){
    char fileToExecuteName[256];
    scanf("%255s", fileToExecuteName);
    
    FILE *codeToExecutePtr;
    codeToExecutePtr = fopen(fileToExecuteName, "r");

    if(codeToExecutePtr == NULL){
        printf("File '%s' not found", fileToExecuteName);
        return 1;
    }

    char line[12];

    int programLength = 0;

    while(fgets(line,  sizeof(line), codeToExecutePtr)){
        // Read each line of machine code
        // Save it into program memory
        strcpy(program_memory[programLength], line);
        programLength++;
    }

    bool endOfProgram = false;
    
    while(!endOfProgram){
        enum OPCODES opcode;
        int operandA;
        int operandB;
        int validInstruction = breakDownInstruction(program_memory[programCounter], &opcode, &operandA, &operandB);

        if(validInstruction == 1){
            endOfProgram = true;
            return 1;
        }

        //printf("%d: %d, %d, %d\n", programCounter, opcode, operandA, operandB);
        
        executeInstruction(opcode, operandA, operandB);
    
        if(programCounter >= programLength) endOfProgram = true;
    }

    printf("\nData Memory\n------\n");
    for(int i = 0; i < (int)(DATA_MEMORY_SIZE / 16); i++)
    {
        for(int k = 0; k < 16; k++){
            int index = i * 16 + k;
            printf("%02X|%d| ", index, data_memory[index]);
        }
        printf("\n");
    }

    if(codeToExecutePtr) fclose(codeToExecutePtr);
    return 0;
}

int breakDownInstruction(char instruction[], enum OPCODES* opcodeSymbol, int* operandAInt, int* operandBInt){
    char operandA[5];
    char operandB[5];
    char opcode[3];

    strncpy(opcode, instruction, 2);
    strncpy(operandA, &instruction[2], 4);
    strncpy(operandB, &instruction[6], 4);

    // Add null terminators
    operandA[4] = '\0';
    operandB[4] = '\0';
    opcode[2] = '\0';
    
    *opcodeSymbol  = hexToInt(opcode, strlen(opcode));
    *operandAInt = hexToInt(operandA, strlen(operandA));
    *operandBInt = hexToInt(operandB, strlen(operandB));

    if(*opcodeSymbol == -1 || *operandAInt == -1 || *operandBInt == -1) return 1;

    return 0;
}

void setFlags(int value){
    zeroFlag = false;
    negativeFlag = false;

    if(value == 0) zeroFlag = true;
    else if(value < 0) negativeFlag = true;

    //printf("Zero: %d, Negative: %d\n", zeroFlag, negativeFlag);
}

int executeInstruction(enum OPCODES opcodeSymbol, int operandADecimal, int operandBDecimal){
    int exitStatus = 0;
    bool incrementPC = true;
    int newPC = programCounter;
    
    switch(opcodeSymbol){
        case NOP:
            //printf("No operation\n");
            exitStatus = 0;
            break;
        case LDD:
            //printf("Load value\n");
            data_memory[operandADecimal] = data_memory[operandBDecimal];
            exitStatus = 0;
            break;
        case LDD_I:
            //printf("Load immediate value\n");
            data_memory[operandADecimal] = operandBDecimal;
            exitStatus = 0;
            break;
        case ADC:
            data_memory[ACCUMULATOR] += data_memory[operandADecimal];

            setFlags(data_memory[ACCUMULATOR]);

            exitStatus = 0;
            break;
        case ADC_I:
            data_memory[ACCUMULATOR] += operandADecimal;

            setFlags(data_memory[ACCUMULATOR]);

            exitStatus = 0;
            break;
        case INC:
            data_memory[operandADecimal] += 1;

            setFlags(data_memory[operandADecimal]);

            exitStatus = 0;
            break;
        case DEC:
            data_memory[operandADecimal] -= 1;
            setFlags(data_memory[operandADecimal]);
            exitStatus = 0;
            break;
        case JNZ:
            if(!zeroFlag){
                incrementPC = false;
                newPC = operandADecimal;
            }
            exitStatus = 0;
            break;
        case OUT:
            printf("%d\n", data_memory[operandADecimal]);
            break;
        default:
            exitStatus = 0;
            break;
    }

    if(incrementPC) newPC = programCounter + 1;

    programCounter = newPC;

    return exitStatus;
}