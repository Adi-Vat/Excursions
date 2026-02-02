#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 256
#define MAX_ERRORS 128
#define MAX_LABELS 16
#define MAX_NAME_SIZE 32
#define MAX_VARIABLES 16
#define VAR_START_ADDR 0
#define MAX_VALUE 65535
#define OPERATION_COUNT 18
#define ADDR_MODE_COUNT 3
#define MACHINE_CODE_INSTR_LEN 10
#define OPCODE_LEN 2
#define OPERAND_LEN 4
#define NUM_REGISTERS 4
#define COMMENT_CHAR ';'
#define LABEL_CHAR ':'
#define HEX_CHAR '$'
#define BINARY_CHAR '&'
#define MEM_CHAR '@'

extern const char DEFAULT_HEX_FILE_NAME[];
extern const char VAR_DIRECTIVE[];
extern const char PROTECTED_CHARS[];

typedef enum{
    OP_NOP,
    OP_LOAD,
    OP_STORE,
    OP_ADD,
    OP_SUB,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_CMP,
    OP_JMP,
    OP_JZ,
    OP_JNZ,
    OP_JN,
    OP_INC,
    OP_DEC,
    OP_PUSH,
    OP_POP,
    OP_HALT,
    OP_INVALID
} Operation;

typedef enum{
    ADDR_MEM,
    ADDR_REG,
    ADDR_DIR,
    ADDR_INVALID
} Addr_Mode;

typedef enum{
    // No error
    ERR_NONE,
    // Line exceeds character limit
    ERR_CHAR_LIM_EXCEED,
    // Name exceeds character limit
    ERR_NAME_CHAR_LIM_EXCEED,
    // Character is protected
    ERR_PROT_CHAR,
    // Name conflicts with register
    ERR_REG_CONFLICT,
    // Name is whitespace
    ERR_WHITESPACE,
    // Name already exists
    ERR_NAME_EXISTS,
    // Number of labels exceeds maximum
    ERR_MAX_LABELS_EXCEED,
    // Number of variables exceeds maximum
    ERR_MAX_VARS_EXCEED,
    // Variable declared improperly
    ERR_BAD_VAR_DECLERATION,
    // Value empty
    ERR_VAL_EMPTY,
    // Value has a bad prefix
    ERR_BAD_RADIX_PREFIX,
    // Value invalid
    ERR_VAL_INVALID,
    // Value exceeds range
    ERR_VAL_RANGE_EXCEED,
    // Variable undeclared
    ERR_NAME_UNDECLARED,
    // Unexpected operands received
    ERR_UNEXPECTED_OPERANDS,
    // Opcode not found
    ERR_INVALID_OPCODE,
    // Wrong addressing mode given
    ERR_BAD_ADDR_MODE,
    // Protected memory accessed
    ERR_PROT_MEM_ACCESSED,
    // Invalid register accessed
    ERR_INVALID_REGISTER
} Error_Code;

typedef struct{
    char name[MAX_NAME_SIZE];
    uint16_t value;
} Label;

typedef struct{
    char op_name[16];
    Operation op;
    int args;
    Addr_Mode dest_addr_mode;
} Operation_Mapping;

extern uint8_t opcode_map[OPERATION_COUNT][ADDR_MODE_COUNT];

bool is_protected_char(char c);
bool empty_line(const char* line);
int index_of_str(char** array, int array_len, char* item);
int index_of_label(Label array[], int array_len, char* item_key);
int first_char_in_str(char search_char, char* string);
bool is_register_name(const char *name);
char* remove_whitespace(char* str);
char* remove_inline_comment(char* str);
Operation_Mapping find_operation(const char* search_name);
Error_Code str_to_num(const char* str, long int* val_out);
Error_Code unique_name(char* name, int line_num, Label *labels, Label *variables);
Error_Code valid_name(char* name, int line_num, Label* labels, Label* variables);
void add_error(Error_Code error_code, int line_num, char* error_string, size_t size, int* error_pos);
Addr_Mode get_addr_mode(const char* arg);

#endif