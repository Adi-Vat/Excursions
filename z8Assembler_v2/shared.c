#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.h"

const char DEFAULT_HEX_FILE_NAME[] = "out.hex"; 
const char VAR_DIRECTIVE[] = "var";
const char PROTECTED_CHARS[] = {COMMENT_CHAR, LABEL_CHAR, HEX_CHAR, BINARY_CHAR, MEM_CHAR, '\0'};

Operation_Mapping operations[] = {
    {"nop", OP_NOP, 0, ADDR_INVALID},
    {"ld", OP_LOAD, 2, ADDR_REG},
    {"str", OP_STORE, 2, ADDR_MEM},
    {"add", OP_ADD, 2, ADDR_REG},
    {"sub", OP_SUB, 2, ADDR_REG},
    {"and", OP_AND, 2, ADDR_REG},
    {"or", OP_OR, 2, ADDR_REG},
    {"xor", OP_XOR, 2, ADDR_REG},
    {"cmp", OP_CMP, 2, ADDR_REG},
    {"jmp", OP_JMP, 1, ADDR_DIR},
    {"jz", OP_JZ, 1, ADDR_DIR},
    {"jnz", OP_JNZ, 1, ADDR_DIR},
    {"inc", OP_INC, 1, ADDR_REG},
    {"dec", OP_DEC, 1, ADDR_REG},
    {"psh", OP_PUSH, 1, ADDR_MEM},
    {"pop", OP_POP, 1, ADDR_REG},
    {"sb", OP_SB, 2, ADDR_DIR},
    {"cb", OP_CB, 2, ADDR_DIR},
    {"halt", OP_HALT, 0, ADDR_INVALID}
};

uint8_t opcode_map[OPERATION_COUNT][ADDR_MODE_COUNT] = {
    //          MEM   REG   DIR
    [OP_NOP] = {0x00, 0x00, 0x00},
    [OP_LOAD] = {0x01, 0x02, 0x03},
    [OP_STORE] = {0xFF, 0x04, 0x05},
    [OP_ADD] = {0xFF, 0x06, 0x07},
    [OP_SUB] = {0xFF, 0x08, 0x09},
    [OP_AND] = {0xFF, 0x0A, 0x0B},
    [OP_OR] = {0xFF, 0x0C, 0x0D},
    [OP_XOR] = {0xFF, 0x0E, 0x0F},
    [OP_CMP] = {0xFF, 0x10, 0x11},
    [OP_JMP] = {0xFF, 0x12, 0x13},
    [OP_JZ] = {0xFF, 0x14, 0x15},
    [OP_JNZ] = {0xFF, 0x16, 0x17},
    [OP_JN] = {0xFF, 0x18, 0x19},
    [OP_INC] = {0xFF, 0x1A, 0xFF},
    [OP_DEC] = {0xFF, 0x1B, 0xFF},
    [OP_PUSH] = {0xFF, 0x1C, 0x1D},
    [OP_POP] = {0x1E, 0x1E, 0x1E},
    [OP_SB] = {0x1F, 0x20, 0xFF},
    [OP_CB] = {0x21, 0x22, 0xFF},
    [OP_HALT] = {0xFE, 0xFE, 0xFE}
};

bool is_protected_char(char c){
    for(int i = 0; PROTECTED_CHARS[i]; i++){
        if(c == PROTECTED_CHARS[i]) return true;
    }

    return false;
}

// Check if a string is whitespace 
bool empty_line(const char* line){
    for(int i = 0; line[i]; i++){
        if(!isspace(line[i])) return false;
    }

    return true;
}

/* Returns index of string in string array
    Returns -1 if string does not exist
*/
int index_of_str(char** array, int array_len, char* item){
    if(item[0] == '\0') return -1;

    for(int i = 0; i < array_len; i++){
        if(array[i] != NULL && strcmp(array[i], item) == 0) return i;
    }

    return -1;
}

/* Returns index of label in labels
    Returns -1 if label doesn't exist
*/
int index_of_label(Label array[], int array_len, char* item_key){
    if(item_key[0] == '\0') return -1;

    for(int i = 0; i < array_len; i++){
        if(strcmp(array[i].name, item_key) == 0) return i;
    }

    return -1;
}

int first_char_in_str(char search_char, char* string){
    for(int i = 0; string[i]; i++){
        if(search_char == string[i]) return i;
    }
    return -1;
}

bool is_register_name(const char *name){
    if(!name || name[0] != REG_CHAR)
        return false;

    if(!isdigit((unsigned char)name[1]))
        return false;

    for(int i = 1; name[i]; i++){
        if(!isdigit((unsigned char)name[i])) 
            return false;
    }
    
    return true;
}

bool is_bank_name(const char *name){
    if(!name || name[0] != BANK_CHAR)
        return false;

    if(!isdigit((unsigned char)name[1]))
        return false;
    
    for(int i = 1; name[i]; i++){
        if(!isdigit((unsigned char)name[1]))
            return false;
    }

    return true;
}

char* remove_whitespace(char* str){
    if(!str) return str;
    // >__is__
    //  012345
    int str_length = strlen(str);
    int start_index = 0;
    // remove leading whitespace
    while(isspace((unsigned char)str[start_index]) && start_index < str_length){
        start_index++;
    }
    //__>is__
    //00 0123
    str += start_index;
    int end_index = strlen(str) - 1;
    // remove trailing whitespace
    while(isspace((unsigned char)str[end_index])){
        end_index--;
    }

    str[end_index + 1] = '\0';
    return str;
}

char* remove_inline_comment(char* str){
    if(!str) return str;
    
    int str_length = strlen(str);
    int end_index = 0;
    while(str[end_index] != ';' && end_index < str_length){
        end_index++;
    }

    str[end_index] = '\0';
    return str;
}

Operation_Mapping find_operation(const char* search_name){
    for(int i = 0; i < OPERATION_COUNT; i++){
        if(strcmp(operations[i].op_name, search_name) == 0)
            return operations[i];
    }

    Operation_Mapping invalid_struct = {"INVALID", OP_INVALID, 0, ADDR_INVALID};
    return invalid_struct;
}

// Returns error code
Error_Code str_to_num(const char* str, long int* val_out){
    if(!str) return ERR_VAL_EMPTY;

    // Try convert value to decimal
    int base = 10;
    
    switch(str[0]){
        case HEX_CHAR:
            // Remove the identifier chars
            str ++;
            base = 16;    
        break;
        case BINARY_CHAR:
            str ++;
            base = 2;
        break;
    }

    char * end_ptr;

    *val_out = strtol(str, &end_ptr, base);

    // Check validity of number
    if(end_ptr == str) return ERR_BAD_RADIX_PREFIX;
    else if(*end_ptr != '\0') return ERR_VAL_INVALID;
    else if(*val_out > MAX_VALUE/2 || *val_out < -(MAX_VALUE/2 + 1)) return ERR_VAL_RANGE_EXCEED;

    return ERR_NONE;
}

// Check if a name already exists in labels or variables
Error_Code unique_name(char* name, Label *labels, Label *variables){
    int possible_var_index = index_of_label(variables, MAX_VARIABLES, name);
    int possible_label_index = index_of_label(labels, MAX_LABELS, name);

    if(possible_label_index > -1 || possible_var_index > -1) return ERR_NAME_EXISTS;

    return ERR_NONE;
}

// Reject name if:
// is empty
// already exists (as a label or variable)
// starts with a digit
// too many labels already
// number of characters exceeds limit
// conflicts with a register
Error_Code valid_name(char* name){
    if(isspace(name[0]) || strlen(name) <= 0) return ERR_WHITESPACE;

    if(isdigit(name[0]) || is_protected_char(name[0])) return ERR_PROT_CHAR;

    if(strlen(name) > MAX_NAME_SIZE) return ERR_NAME_CHAR_LIM_EXCEED;

    if(is_register_name(name)) return ERR_REG_CONFLICT;

    if(is_bank_name(name)) return ERR_BANK_CONFLICT;

    return ERR_NONE;
}


void add_error(Error_Code error_code, int line_num, char* error_string, size_t size, int* error_pos){
    char error_start[32] = "";
    snprintf(error_start, sizeof(error_start), "[Line %d] Error: ", line_num);
    char error_message[MAX_LINE_SIZE - 32] = "";
    
    switch(error_code){
        case ERR_NONE:
        break;
        case ERR_CHAR_LIM_EXCEED:
            snprintf(error_message, sizeof(error_message), "line exceeds %d char limit", MAX_LINE_SIZE);
        break;
        case ERR_NAME_CHAR_LIM_EXCEED:
            snprintf(error_message, sizeof(error_message), "name exceeds %d char limit", MAX_NAME_SIZE);
        break;
        case ERR_WHITESPACE:
            snprintf(error_message, sizeof(error_message), "name cannot be whitespace");
        break;
        case ERR_PROT_CHAR:
            snprintf(error_message, sizeof(error_message), "name cannot start with digit or %s", PROTECTED_CHARS);
        break;
        case ERR_REG_CONFLICT:
            snprintf(error_message, sizeof(error_message), "name conflicts with register notation %c{num}", REG_CHAR);
        break;
        case ERR_NAME_EXISTS:
            snprintf(error_message, sizeof(error_message), "name already declared as variable or label");
        break;
        case ERR_MAX_LABELS_EXCEED:
            snprintf(error_message, sizeof(error_message), "maximum %d labels exceeded", MAX_LABELS);
        break;
        case ERR_MAX_VARS_EXCEED:
            snprintf(error_message, sizeof(error_message), "maximum %d variables exceeded", MAX_VARIABLES);
        break;
        case ERR_BAD_VAR_DECLERATION:
            snprintf(error_message, sizeof(error_message), "incorrect usage of .var <name> <value>");
        break;
        case ERR_VAL_EMPTY:
            snprintf(error_message, sizeof(error_message), "no value given");
        break;
        case ERR_VAL_INVALID:
            snprintf(error_message, sizeof(error_message), "invalid value given");
        break;
        case ERR_VAL_RANGE_EXCEED:
            snprintf(error_message, sizeof(error_message), "value is out of range [%d, %d]", -(MAX_VALUE/2 + 1), (MAX_VALUE/2));
        break;
        case ERR_NAME_UNDECLARED:
            snprintf(error_message, sizeof(error_message), "trying to access an undeclared variable/label");
        break;
        case ERR_UNEXPECTED_OPERANDS:
            snprintf(error_message, sizeof(error_message), "unexpected number of operands");
        break;
        case ERR_INVALID_OPCODE:
            snprintf(error_message, sizeof(error_message), "unexpected opcode");
        break;
        case ERR_BAD_ADDR_MODE:
            snprintf(error_message, sizeof(error_message), "wrong addressing mode used");
        break;
        case ERR_PROT_MEM_ACCESSED:
            snprintf(error_message, sizeof(error_message), "cannot directly access protected memory 0x%04X - 0x%04X", VAR_START_ADDR, VAR_START_ADDR + MAX_VARIABLES);
        break;
        case ERR_BAD_RADIX_PREFIX:
            snprintf(error_message, sizeof(error_message), "invalid radix prefix used");
        break;
        case ERR_INVALID_REGISTER:
            snprintf(error_message, sizeof(error_message), "attempted to access invalid register, use R0-R%d", REGISTERS_COUNT-1);
        break;
        case ERR_INVALID_BANK:
            snprintf(error_message, sizeof(error_message), "attempted to access an invalid output bank, valid banks are: 0, 1");
        break;
        case ERR_BANK_CONFLICT:
            snprintf(error_message, sizeof(error_message), "name conflicts with output bank notation %c{num}", BANK_CHAR);
        break;
        case ERR_INVALID_BANK0_ADDR:
            snprintf(error_message, sizeof(error_message), "trying to access invalid BANK 0 row, valid rows are %d-%d", 0, BANK_0_SIZE - 1);            
        break;
        case ERR_INVALID_BANK1_ADDR:
            snprintf(error_message, sizeof(error_message), "trying to access invalid BANK 1 row, valid rows are 0-1");            
        break;
        case ERR_UNSAFE_BANK1_OP:
            snprintf(error_message, sizeof(error_message), "BANK 1 can only be operated on with SB (set bit) and CB (clear bit)");
        break;
        case ERR_BIT_OUT_OF_RANGE:
            snprintf(error_message, sizeof(error_message), "bit out of range, must be 0-%d for normal access and 0-11 for BANK 1", (WORD_SIZE - 1));
        break;
    }

    char full_error[MAX_LINE_SIZE] = "";
    snprintf(full_error, sizeof(full_error), "%s%s\n", error_start, error_message);
    *error_pos += snprintf(error_string + *error_pos, size - *error_pos, "%s", full_error); 
}

Addr_Mode get_addr_mode(const char* arg){
    if(is_register_name(arg)) return ADDR_REG;
    else if(arg[0] == MEM_CHAR) return ADDR_MEM;
    else return ADDR_DIR;
}