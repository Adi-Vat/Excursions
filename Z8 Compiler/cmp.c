#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "shared.h"

#define WORD_SIZE 16
#define instructionLength 29
#define instructionSize 5
#define baseRegisterAddress 0x00A4
#define LABEL_SIZE 32
#define MAX_LABELS 10
#define REGISTER_NUM 4
#define REGISTER_NAME_SIZE 2

char instructions[instructionLength][instructionSize] = {
    "NOP",
    "LDD",
    "-",
    "LDR",
    "STR",
    "-",
    "ADC",
    "-",
    "SUB",
    "-",
    "INC",
    "DEC",
    "ROL",
    "ROR",
    "SLL",
    "SLA",
    "SRL",
    "SRA",
    "PSH",
    "POP",
    "JMP",
    "CMP",
    "-",
    "JZ",
    "JNZ",
    "JN",
    "SB",
    "CB",
    "OUT"
};

char instructionArgs[instructionLength] = {
    0, //NOP
    2, //LDD <REG> <- <REG/MEM>
    2, //LDD_I immediate <REG> <- <VAL>
    2, //LDR <REG> <- <REG/MEM>
    2, //STR <MEM> <- <REG>
    2, //STR_I immediate <MEM> <- <VAL>
    1, //ADC <REG>
    1, //ADC_I immediate <VAL>
    1, //SUB <REG>
    1, //SUB_I immediate <VAL>
    1, //INC <REG>
    1, //DEC <REG>
    1, //ROL <REG>
    1, //ROR <REG>
    1, //SLL <REG>
    1, //SLA <REG>
    1, //SRL <REG>
    1, //SRA <REG>
    1, //PSH <REG>
    1, //POP <REG>
    1, //JMP <VAL>
    2, //CMP <REG> with <REG>
    2, //CMP_I immediate <REG> with <VAL>
    1, //JZ <VAL>
    1, //JNZ <VAL>
    1, //JN <VAL>
    2, //SB <REG> at bit <VAL>
    2, // CB <REG> at bit <VAL>
    1, // OUT print value at <REG/MEM>
};

char registers[REGISTER_NUM][REGISTER_NAME_SIZE] = {
    "a", //A4
    "x", //A5
    "y", //A6
    "z" //A7
};

char labels[MAX_LABELS][LABEL_SIZE];
int labelAddresses[MAX_LABELS];

char** splitSentence(char sentence[], int *length);

int parseOperand(char operand[], int* value, int lineNumber);

int main ()
{
    char assemblyFile[50];
    char outputFile[50];
    printf("<ASM file> <output file>\n");
    scanf("%s %s", assemblyFile, outputFile);

    FILE *asmfptr;
    asmfptr = fopen(assemblyFile, "r");

    char errorMessage[265];
    int ret = 0;
    if(asmfptr == NULL){
        snprintf(errorMessage, sizeof(errorMessage), "Failed to open '%s'", assemblyFile);
        perror(errorMessage);
        return 1;
    }  
    
    int labelIndex = 0;
    int nextLabelIndex = 0;
    int lineNumber = 0;
    char line[256];

    while(fgets(line, sizeof(line), asmfptr)){
        char opcode[32];

        int scanResult = sscanf(line, "%s", opcode);
        opcode[strlen(opcode)] = '\0'; // ensure null termination

        if(opcode[0] == ';' || scanResult == -1){
            lineNumber ++;
            continue; // skip comments and empty lines
        }

        // If the line is a label, store it
        if(opcode[0] == '.'){
            int labelIndex = indexOf(labels[0], &opcode[1], MAX_LABELS, LABEL_SIZE);
            if(labelIndex != -1){
                printf("Line %d: Label '%s' is already defined on line %d\n", lineNumber + 1, &opcode[1], labelAddresses[labelIndex] + 1);
                ret = 1;
                goto cleanup;
            }
            strcpy(labels[nextLabelIndex], &opcode[1]);
            //labelAddresses[nextLabelIndex] = lineNumber; 
            nextLabelIndex ++;
        }
        lineNumber ++;
    } // First pass to store labels

    rewind(asmfptr);
    lineNumber = 0;
    char **machineCodeArr = NULL;
    size_t machineCodeLines = 0;
 
    while(fgets(line, sizeof(line), asmfptr)){
        char opcode[32];
        char operandA[32];
        char operandB[32];

        int scanResult = sscanf(line, "%s", opcode);
        opcode[strlen(opcode)] = '\0'; // ensure null termination

        if(opcode[0] == ';' || scanResult == -1){
            lineNumber ++;
            continue; // skip comments and empty lines
        }

        if(opcode[0] == '.'){
            lineNumber ++;
            int labelIndex = indexOf(labels[0], &opcode[1], MAX_LABELS, LABEL_SIZE);
            //printf("%d\n", labelIndex);
            labelAddresses[labelIndex] = (int)machineCodeLines;
            continue;
        }

        // Break down opcode
        int opcodeIndex = indexOf(instructions[0], opcode, instructionLength, instructionSize);
        if(opcodeIndex == -1){
            printf("Line %d: Unknown opcode '%s'", lineNumber + 1, opcode);
            ret = 1; 
            goto cleanup;
        }
        
        int length = 0;

        char **words = splitSentence(line, &length);

        if(length - 1> instructionArgs[opcodeIndex]){
            printf("Line %d: Too many arguments for '%s'\n", lineNumber + 1, opcode);
            ret = 1;
            goto cleanup;
        } else if(length - 1 < instructionArgs[opcodeIndex]){
            printf("Line %d: Not enough arguments for '%s'\n", lineNumber + 1, opcode);
            ret = 1;
            goto cleanup;
        }

        // If it's a two word operation, extract one operand
        if(length == 2){
            strcpy(operandA, words[1]);
            if(operandA[strlen(operandA) - 1] == '\n')
                operandA[strlen(operandA) - 1]  = '\0'; // remove whitespaces
        } else if (length == 3){ // If it's a three word operation, extract two operands
            strcpy(operandA, words[1]);
            operandA[strlen(operandA) - 1]  = '\0'; // remove comma
            strcpy(operandB, words[2]);
            if(operandB[strlen(operandB) - 1] == '\n')
                operandB[strlen(operandB) - 1]  = '\0'; // remove whitespaces
        } else{ // If there's more than two operands, error
            printf("Line %d: Invalid syntax", lineNumber + 1);
            ret = 1;
            goto cleanup;
        }

        // value of operandA
        int value1 = 0x00;
        int operandAType = 0;  
        operandAType = parseOperand(operandA, &value1, lineNumber);

        // value of operandB
        int value2 = 0x00;
        int operandBType = 0;
        if(length == 3) operandBType = parseOperand(operandB, &value2, lineNumber);
        
        if(operandAType == -1 || operandBType == -1) return 1;

        if(operandBType == 1 && opcodeIndex != instructionLength - 1){
            if(strcmp(instructions[opcodeIndex + 1], "-") == 0){
                opcodeIndex += 1;
            }
        }
        
        /*
        printf("#%d: %s (%02X), %s (%04X), %s (%04X)\n", 
            lineNumber + 1, 
            opcode, 
            opcodeIndex, 
            operandA, 
            value1, 
            (length == 3) ? operandB : "N/A", 
            (length == 3) ? value2 : 0x0000
        );
        */
        char machineCodeLine[64];
        char opcodeStr[8];
        char operandAStr[12];
        char operandBStr[12];

        sprintf(opcodeStr, "%02X",opcodeIndex);
        sprintf(operandAStr, "%04X",value1);
        sprintf(operandBStr, "%04X", value2);

        strcpy(machineCodeLine, opcodeStr);
        strcat(machineCodeLine, operandAStr);
        strcat(machineCodeLine, operandBStr);
        strcat(machineCodeLine, "\n");

        char *copy = malloc(strlen(machineCodeLine) + 1);
        if(!copy){
            fprintf(stderr, "Failed to allocate string\n");
            ret = 1;
            goto cleanup;
        }
        strcpy(copy, machineCodeLine);

        char **tmp = realloc(machineCodeArr, (machineCodeLines + 1) * sizeof(char *));

        if(!tmp){
            fprintf(stderr, "Failed to grow string array \n");
            free(copy);
            ret = -1;
            goto cleanup;
        }

        machineCodeArr = tmp;
        machineCodeArr[machineCodeLines] = copy;

        machineCodeLines ++;
        lineNumber ++;
    }

    FILE *binfptr;
    binfptr = fopen(outputFile, "w");
    fclose(binfptr);
    binfptr = fopen(outputFile, "a");

    for(size_t i = 0; i < machineCodeLines; i++){
        fprintf(binfptr, machineCodeArr[i]);
    }

    fclose(binfptr);

    char openNotepad[32];
    sprintf(openNotepad, "notepad %s", outputFile);
    system(openNotepad);

cleanup:
    
    if(asmfptr) fclose(asmfptr);
    for(size_t i = 0; i < machineCodeLines; i++)
        if(machineCodeArr[i]) free(machineCodeArr[i]);
    
    if(machineCodeArr) free(machineCodeArr);
    
    return ret;
}

char **splitSentence(char sentence[], int *length){
    char * ptr = strtok(sentence, " ");
    char *str = strdup(sentence);

    int capacity = 2;
    int size = 0;
    char **result = malloc(capacity * sizeof(char*));

    while(ptr != NULL){
        if(size >= capacity){
            capacity *= 2;
            result = realloc(result, capacity * sizeof(char*));
            if(result == NULL){
                perror("Failed to reallocate memory");
                exit(1);
            }
        }
        
        result[size++] = strdup(ptr);
        ptr = strtok(NULL, " ");
    }

    *length = size;
    free(str);
    return result;
}

// Check if operand is register, memory address or immediate value
// Convert register to memory address
// Returns -1 if an error occured, 0 if a register or memory address, 1 if immediate value, 2 if a label
int parseOperand(char operand[], int* value, int lineNumber){
    int value1 = 0x00;
    int toReturn;

    // If operandA is a label, get its address
    if(operand[0] == '.'){
        // get index of the label
        int labelIndex = indexOf(labels[0], &operand[1], MAX_LABELS, LABEL_SIZE);

        if(labelIndex == -1){
            printf("Line %d: Undefined label '%s'\n", lineNumber + 1, &operand[1]);
            return -1;
        }
        toReturn = 2;
        value1 = labelAddresses[labelIndex];
    } else{ // The operand is not a label, process to find the memory location
    
        // Check if operandA is a register
        int registerIndex = indexOf(registers[0], operand, REGISTER_NUM, REGISTER_NAME_SIZE);
        
        // Register -> Memory address
        // Memory address -> Memory address
        // Immediate value -> Increment opcode if LDD/LDR

        if(registerIndex != -1){ // Check if operand is a register
            // If it is, convert register to memory address
            value1 = registerIndex + baseRegisterAddress;
            //value1 = (registerIndex * WORD_SIZE) + baseRegisterAddress;
            toReturn = 0;
        } else if(operand[0] == '*'){ // Check if operand is a memory address
            value1 = hexToInt(&operand[1], strlen(operand) - 1);

            if(value1 == -1){
                printf("Line %d: Invalid hex value '%s'\n", lineNumber + 1, &operand[1]);
                return -1;
            }
            
            // Check if memory address is in range
            if(value1 < 0 || value1 > 0xFFFF){
                printf("Line %d: Memory address out of range '%s'\n", lineNumber + 1, &operand[1]);
                return -1;
            }

            toReturn = 0;
        } else{
            // Immediate value
            if(operand[0] == '$'){
                value1 = hexToInt(&operand[1], strlen(operand) - 1);
            } else{ 

                char *endptr;
                long val;
                val = strtol(operand, &endptr, 10);

                if(endptr == operand) val = -1;
                if(*endptr != '\0') val = -1;
                
                value1 = (int)val;
            }

            if(value1 == -1){
                printf("Line %d: Invalid value '%s'\n", lineNumber + 1, operand);
                return -1;
            }

            // Check if immediate value is in range
            if(value1 < 0 || value1 > 0xFFFF){
                printf("Line %d: Immediate value out of range '%s'\n", lineNumber + 1, operand);
                return -1;
            }

            toReturn = 1;
        }
    }   

    *value = value1;
    return toReturn;
}