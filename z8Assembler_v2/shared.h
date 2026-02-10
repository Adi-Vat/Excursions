#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#define WORD_SIZE 8
#define MAX_LINE_SIZE 256
#define MAX_ERRORS 128
#define MAX_LABELS 16
#define MAX_NAME_SIZE 32
#define MAX_VARIABLES 16
#define VAR_START_ADDR 0x01
#define MAX_VALUE 255
#define OPERATION_COUNT 20
#define ADDR_MODE_COUNT 3
#define MACHINE_CODE_INSTR_LEN 10
#define OPCODE_LEN 2
#define OPERAND_LEN 4
#define REGISTERS_COUNT 4
#define BANK_0_START 0xD9
#define BANK_0_SIZE 3
#define BANK_1_START 0xDC
#define BANK_1_SIZE 4
#define COMMENT_CHAR ';'
#define LABEL_CHAR ':'
#define HEX_CHAR '$'
#define BINARY_CHAR '&'
#define MEM_CHAR '@'
#define REG_CHAR 'r'
#define BANK_CHAR 'b'

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
    OP_SB,
    OP_CB,
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
    ERR_INVALID_REGISTER,
    // Invalid output bank accessed
    ERR_INVALID_BANK,
    // Name conflicts with bank name
    ERR_BANK_CONFLICT,
    // Invalid bank address being accessed
    ERR_INVALID_BANK0_ADDR,
    ERR_INVALID_BANK1_ADDR,
    // Unsafe operation applied to bank 1
    ERR_UNSAFE_BANK1_OP,
    // Out of range bit trying to be accessed with SB or CB
    ERR_BIT_OUT_OF_RANGE
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

typedef enum{
    EMU_NOP, // 0x00 NO OPERATION
    EMU_LDM, // 0x01 Load value from memory into register
    EMU_LDR, // 0x02 Load value from register into register
    EMU_LDD, // 0x03 Load value directly into register
    EMU_STR, // 0x04 Store value from register into memory address
    EMU_STD, // 0x05 Store direct value into memory address
    EMU_ADR, // 0x06 Add value from register to another register
    EMU_ADD, // 0x07 Add value to another register
    EMU_SBR, // 0x08 Subtract the source from the dest and store in the dest
    EMU_SBD, // 0x09 Subtract a value from the register and store in the register
    EMU_ANR, // 0x0A AND a register and a register store in the dest
    EMU_AND, // 0x0B AND a register and value store in the dest
    EMU_ORR, // 0x0C OR a reg and reg store in dest
    EMU_ORD, // 0x0D OR a reg and value store in dest
    EMU_XOR, // 0x0E XOR a reg and reg store in dest
    EMU_XOD, // 0x0F XOR a reg and value store in dest
    EMU_CPR, // 0x10 Compare reg and reg
    EMU_CPD, // 0x11 Compare reg and value,
    EMU_JPR, // 0x12 Jump to line stored in reg
    EMU_JPD, // 0x13 Jump directly to line
    EMU_JZR, // 0x14 Jump to line stored in reg if zero flag is set
    EMU_JZD, // 0x15 Jump directly to line if zero flag is set
    EMU_JNZR, // 0x16 Jump to line stored in reg if zero flag is NOT set
    EMU_JNZD, // 0x17 Jump directly to line if zero flag is NOT set
    EMU_JNR, // 0x18 Jump to line stored in reg if negative flag is set
    EMU_JND, // 0x19 Jump directly to line if negative flag is set
    EMU_INC, // 0x1A Increment register
    EMU_DEC, // 0x1B Decrement register
    EMU_PSHR, // 0x1C Push register to stack
    EMU_PSHD, // 0x1D Push value to stack
    EMU_POP, // 0x1E Pop top value from stack into register
    EMU_SBIM, // 0x1F Set bit of word in memory (to 1)
    EMU_SBIR, // 0x20 Set bit of word in register (to 1)
    EMU_CBIM, // 0x21 Clear bit of word in memory (to 0)
    EMU_CBIR, // 0x22 Clear bit of word in register (to 0)
    EMU_HALT = 0xFE // 0xFE Stops program
} Emulator_Operations;

extern uint8_t opcode_map[OPERATION_COUNT][ADDR_MODE_COUNT];

bool is_protected_char(char c);
bool empty_line(const char* line);
int index_of_str(char** array, int array_len, char* item);
int index_of_label(Label array[], int array_len, char* item_key);
int first_char_in_str(char search_char, char* string);
bool is_register_name(const char *name);
bool is_bank_name(const char *name);
char* remove_whitespace(char* str);
char* remove_inline_comment(char* str);
Operation_Mapping find_operation(const char* search_name);
Error_Code str_to_num(const char* str, long int* val_out);
Error_Code unique_name(char* name, Label *labels, Label *variables);
Error_Code valid_name(char* name);
void add_error(Error_Code error_code, int line_num, char* error_string, size_t size, int* error_pos);
Addr_Mode get_addr_mode(const char* arg);

#endif