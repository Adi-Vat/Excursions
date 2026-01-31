#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 128
#define MAX_LABELS 256
#define MAX_LABEL_SIZE 32
#define MAX_VARIABLES 256
#define VAR_START_ADDR 0
#define MAX_VALUE 65535
#define OPERATION_COUNT 18
#define ADDR_MODE_COUNT 3
#define MACHINE_CODE_INSTR_LEN 10
#define OPCODE_LEN 2
#define OPERAND_LEN 4
#define COMMENT_CHAR ';'
#define LABEL_CHAR ':'
#define HEX_CHAR '$'
#define BINARY_CHAR '&'
#define MEM_CHAR '@'

const char DEFAULT_HEX_FILE_NAME[] = "out.hex";
const char VAR_DIRECTIVE[] = "var";

typedef struct{
    char name[MAX_LABEL_SIZE];
    uint16_t value;
} Label;

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
} Addr_mode;

typedef struct{
    char op_name[16];
    Operation op;
    int args;
} Operation_Mapping;

Operation_Mapping operations[] = {
    {"NOP", OP_NOP, 0},
    {"LD", OP_LOAD, 2},
    {"STR", OP_STORE, 2},
    {"AND", OP_AND, 2},
    {"OR", OP_OR, 2},
    {"XOR", OP_XOR, 2},
    {"CMP", OP_CMP, 2},
    {"JMP", OP_JMP, 1},
    {"JZ", OP_JZ, 1},
    {"JNZ", OP_JNZ, 1},
    {"INC", OP_INC, 1},
    {"DEC", OP_DEC, 1},
    {"PSH", OP_PUSH, 1},
    {"POP", OP_POP, 0},
    {"HALT", OP_HALT, 0}
};

uint8_t opcode_map[OPERATION_COUNT][ADDR_MODE_COUNT] = {
    //          MEM   REG   DIR
    [OP_NOP] = {0x00, 0x00, 0x00},
    [OP_LOAD] = {0x01, 0x02, 0x03},
    [OP_STORE] = {0x00, 0x04, 0x05},
    [OP_ADD] = {0x00, 0x06, 0x07},
    [OP_SUB] = {0x00, 0x08, 0x09},
    [OP_AND] = {0x00, 0x0A, 0x0B},
    [OP_OR] = {0x00, 0x0C, 0x0D},
    [OP_XOR] = {0x00, 0x0E, 0x0F},
    [OP_CMP] = {0x00, 0x10, 0x11},
    [OP_JMP] = {0x00, 0x12, 0x13},
    [OP_JZ] = {0x00, 0x14, 0x15},
    [OP_JNZ] = {0x00, 0x16, 0x17},
    [OP_JN] = {0x00, 0x18, 0x19},
    [OP_INC] = {0x00, 0x1A, 0x00},
    [OP_DEC] = {0x00, 0x1B, 0x00},
    [OP_PUSH] = {0x00, 0x1C, 0x1D},
    [OP_POP] = {0x1E, 0x1E, 0x1E},
    [OP_HALT] = {0xFF, 0xFF, 0xFF}
};

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
    if(item == NULL) return -1;

    for(int i = 0; i < array_len; i++){
        if(array[i] != NULL && strcmp(array[i], item) == 0) return i;
    }

    return -1;
}

/* Returns index of label in labels
    Returns -1 if label doesn't exist
*/
int index_of_label(Label array[], int array_len, char* item_key){
    if(item_key == NULL) return -1;

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
    if(!name || name[0] != 'R')
        return false;

    if(!isdigit((unsigned char)name[1]))
        return false;

    for(int i = 1; name[i]; i++){
        if(!isdigit((unsigned char)name[i])) 
            return false;
    }
    
    return true;
}

char *remove_whitespace(char* str){
    if(!str) return '\0';
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
    if(!str) return '\0';
    
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

    Operation_Mapping invalid_struct = {"INVALID", OP_INVALID, 0};
    return invalid_struct;
}

// Returns error code
int str_to_num(const char* str, long int* val_out, char* error){
    char var_value_trim[MAX_LINE_SIZE] = {0};
    // Try convert value to decimal
    int base = 10;
    
    switch(str[0]){
        case HEX_CHAR:
            // Remove the identifier chars
            strcpy(var_value_trim, str + 1);
            base = 16;    
        break;
        case BINARY_CHAR:
            strcpy(var_value_trim, str + 1);
            base = 2;
        break;
        default:
            strcpy(var_value_trim, str);
        break;
    }

    char * end_ptr;

    *val_out = strtol(var_value_trim, &end_ptr, base);

    char e[MAX_LINE_SIZE];
    bool is_error = false;

    // Check validity of number
    if(end_ptr == str){
        // Error: variable not defined
        return 1;
    } else if(*end_ptr != '\0'){
        // Error: invalid number
        return 2;
    } else if(*val_out > MAX_VALUE/2 || *val_out < -(MAX_VALUE/2 + 1)){
        // Error: value exceeds range
        return 3;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    char out_hex_file_name[MAX_LINE_SIZE] = {0};
    // check if number of arguments given exceeds number of arguments expected
    if(argc > 4){
        printf("Error: Correct usage is '%s <assembly file> -o <hex output file>'\n", argv[0]);
        return 1;
    } else if(argc < 2){
        printf("Error: Correct usage is '%s <assembly file>'\n", argv[0]);
        return 1;
    }

    if(argc > 2){
        if(strcmp(argv[2], "-o") != 0){
            printf("Error: 3rd argument must be '-o' followed by hex output file name");
            return 1;
        }
        else{
            strcpy(out_hex_file_name, argv[3]);
        }
    }
    else{
        strcpy(out_hex_file_name, DEFAULT_HEX_FILE_NAME);
    }
    
    // open assembly file
    FILE *asm_file_ptr;
    asm_file_ptr = fopen(argv[1], "r");

    if(asm_file_ptr == NULL){
        printf("Error: '%s' does not exist", argv[1]);
        return 1;
    }

    Label labels[MAX_LABELS];
    
    for(int i = 0; i < MAX_LABELS; i++) labels[i].name[0] = '\0'; 

    int labels_count = 0;
    Label variables[MAX_VARIABLES];

    for(int i = 0; i < MAX_VARIABLES; i++) variables[i].name[0] = '\0'; 
    int vars_count = 0;

    uint16_t instr_addr = 0;
    uint16_t line_num = 0;
    
    char error_str[1024] = {0};
    char asm_line[MAX_LINE_SIZE] = {0};
    /* First pass
        If the label doesn't exist already, store it and its instruction index
        Keep track of instruction index by ignoring comments and whitespace
    */
    while(fgets(asm_line, MAX_LINE_SIZE, asm_file_ptr)){
        line_num++;
        
        // skip empty line
        if(empty_line(asm_line)) continue;
        if(strlen(asm_line) > MAX_LINE_SIZE){
            char e[MAX_LINE_SIZE];
            snprintf(e, sizeof(e), "[Line %d] Error: line exceeds %d character limit\n", line_num, MAX_LINE_SIZE);
            strcat(error_str, e);
            continue;
        }
        remove_inline_comment(asm_line);
        char* line_trim = remove_whitespace(asm_line);
        if(strlen(line_trim) < 1) continue;
        bool is_label = (line_trim[strlen(line_trim)-1] == LABEL_CHAR);
       
        if(is_label){
            int len = strlen(line_trim);

            bool has_space = false;
            for(int i = 0; line_trim[i]; i++){
                if(isspace((unsigned char)line_trim[i])){
                    has_space = true;
                    break;
                }
            }

            if(has_space){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: label cannot have whitespace\n", line_num);
                strcat(error_str, e);
                continue;
            }

            if(len < 2){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: invalid label name\n", line_num);
                strcat(error_str, e);
                continue;
            }

            if(len > MAX_LABEL_SIZE){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: label name exceeds %d characters\n", line_num, MAX_LABEL_SIZE);
                strcat(error_str, e);
                continue;
            }

            if(labels_count >= MAX_LABELS){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: maximum %d labels exceeded\n", line_num, MAX_LABEL_SIZE);
                strcat(error_str, e);
                continue;
            }
            
            // remove ':' from the end end the string
            char label[MAX_LABEL_SIZE] = {0};
            memcpy(label, line_trim, len - 1);
            label[len-1] = '\0';

            // Check if label has already been defined
            if(index_of_label(labels, MAX_LABELS, label) != -1){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: the label '%s' is already defined\n", line_num, label);
                strcat(error_str, e);
                continue;
            }

            // Or if label is already being used as a variable
            if(index_of_label(variables, MAX_VARIABLES, label) != -1){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: '%s' is already being used as a variable\n", line_num, label);
                strcat(error_str, e);
                continue;
            }

            if(is_register_name(label)){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: label conflicts with protected register names\n", line_num);
                strcat(error_str, e);
                continue;
            }

            // Store label
            strcpy(labels[labels_count].name, label);
            labels[labels_count].value = instr_addr;
            labels_count ++;

            continue;
        }

        // Directive is being stated
        char directive_type[16] = {0};
        
        bool is_directive = sscanf(line_trim, ".%15[a-zA-Z0-9_]", directive_type) > 0;
        // if the number of items succesfully parsed is > 0, the line trying to be a directive
        if(is_directive){
            // variable is being defined
            if(strcmp(directive_type, VAR_DIRECTIVE) == 0){
                char var_name[MAX_LINE_SIZE] = {0};
                char var_value[MAX_LINE_SIZE] = {0};

                // Too many variables delcared
                if(vars_count >= MAX_VARIABLES){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: maximum %d variables exceeded\n", line_num, MAX_VARIABLES);
                    strcat(error_str, e);
                    continue;
                }

                // parse variable
                if(sscanf(line_trim, ".%*15[a-zA-Z0-9_] %127[a-zA-Z0-9_] %127s", var_name, var_value) != 2){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: incorrect usage of .var <name> <value>\n", line_num);
                    strcat(error_str, e);
                    continue;
                }

                // check if variable is already defined
                if(index_of_label(variables, MAX_VARIABLES, var_name) != -1){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: variable '%s' has already been declared\n", line_num, var_name);
                    strcat(error_str, e);
                    continue;
                }

                // check if variable is alread used as a label
                if(index_of_label(labels, MAX_LABELS, var_name) != -1){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: '%s' is already being used as a label", line_num, var_name);
                    strcat(error_str, e);
                    continue;
                }

                // Check if variable name is valid (no R{num})
                if(is_register_name(var_name)){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: variable name conflicts with protected register names", line_num);
                    strcat(error_str, e);
                    continue;
                }

                long int var_num;
                // convert str to number
                char e[MAX_LINE_SIZE];
                int error_code = str_to_num(var_value, &var_num, e);
                
                switch (error_code)
                {
                    case 1:
                        snprintf(e, sizeof(e), "[Line %d] Error: variable not defined\n", line_num);
                        strcat(error_str, e);
                        continue;
                    break;
                    case 2:
                        snprintf(e, sizeof(e), "[Line %d] Error: invalid number\n", line_num);
                        strcat(error_str, e);
                        continue;
                    break;
                    case 3:
                        snprintf(e, sizeof(e), "[Line %d] Error: Value is out of range [%d, %d]\n", line_num, -(MAX_VALUE/2 + 1), (MAX_VALUE/2));
                        strcat(error_str, e);
                        continue;
                    break;
                }
                // add variable to array
                strcpy(variables[vars_count].name, var_name);
                variables[vars_count].value = (uint16_t)var_num;
                vars_count++;
            }
        }

        instr_addr ++;
    }
    
    /* Second pass
        Split instruction into mnemonic, operandA, operandB
        Decide opcode based on operands (addressing mode)
        Replace labels with their instruction index
        Replace variables with their locations in memory
        Replace variable decleration with STD <address>, <value>
        Disallow any direct loading into this region
    */
    int num_instructions = instr_addr;
    char data_section[vars_count][MACHINE_CODE_INSTR_LEN + 1];
    // allow for HALT instruction and conversion from index to length
    char main_section[num_instructions-vars_count+2][MACHINE_CODE_INSTR_LEN + 1];
    rewind(asm_file_ptr);
    line_num = 0;
    // index of the next available instruction slot
    instr_addr = 0;
    // index of the next available variable slot
    int var_addr = 0;

    while(fgets(asm_line, MAX_LINE_SIZE, asm_file_ptr)){
        line_num++;

        // skip useless lines
        if(empty_line(asm_line)) continue;
        remove_inline_comment(asm_line);
        char* line_trim = remove_whitespace(asm_line);
        // skip comment lines
        if(strlen(line_trim) < 1) continue;
        bool is_label = (line_trim[strlen(line_trim)-1] == LABEL_CHAR);
        if(is_label) continue;

        // replce directives
        char directive_type[16] = {0};
        bool is_directive = sscanf(line_trim, ".%15[a-zA-Z0-9_]", directive_type) > 0;

        char hex_opcode[OPCODE_LEN + 1] = {0};
        char hex_operand_a[OPERAND_LEN + 1] = {0};
        char hex_operand_b[OPERAND_LEN + 1] = {0};

        if(is_directive){
            if(strcmp(directive_type, VAR_DIRECTIVE) == 0){
                char var_name[MAX_LINE_SIZE] = {0};
                // parse variable
                // return if variable is malformed for some reason
                if(sscanf(line_trim, ".%*15[a-zA-Z0-9_] %127[a-zA-Z0-9_] %*127s", var_name) != 1){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: incorrect usage of .var <name> <value>\n", line_num);
                    strcat(error_str, e);
                    continue;
                }
                
                int var_index = index_of_label(variables, vars_count, var_name);
                // return if variable hasn't already been passed
                if(var_index == -1){
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: variable not found in var array\n", line_num);
                    strcat(error_str, e);
                    continue;
                }

                // parse declaration to STD <mem addr> <val>
                snprintf(hex_opcode, sizeof(hex_opcode), "%02X", opcode_map[OP_STORE][ADDR_DIR]);
                snprintf(hex_operand_a, sizeof(hex_operand_a), "%04X", (var_index + VAR_START_ADDR));
                snprintf(hex_operand_b, sizeof(hex_operand_b), "%04X", variables[var_index].value);
                // store in data section
                snprintf(data_section[var_addr], MACHINE_CODE_INSTR_LEN + 1, "%s%s%s", hex_opcode, hex_operand_a, hex_operand_b);

                var_addr ++;
                continue;
            }   
        }
        else{
            // Not a directive, so an instruction. parse it!
            char opcode[32];
            char operand_a[32];
            char operand_b[32];
            char waste[32];

            // Replace commas with nothing
            for(char *p = line_trim; *p; p++)
                if(*p == ',') *p = ' ';

            int num_tokens = sscanf(line_trim, "%32s %32s %32s %32s", opcode, operand_a, operand_b, waste);

            // check if tokens exceeds certain number
            if(num_tokens > 3){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: too many operands, expecting 0 to 2\n", line_num, num_tokens);
                strcat(error_str, e);
                continue;
            }
            
            // now parse the opcode
            // operations can be either 
            // <Opcode> <OperandA> <OperandB>
            // or
            // <Opcode> <OperandA>
            Operation_Mapping op_map = find_operation(opcode);
            if(op_map.op == OP_INVALID){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: Invalid opcode\n", line_num);
                strcat(error_str, e);
                continue;
            }
            // Check number of args is correct
            if(num_tokens - 1 != op_map.args){
                char e[MAX_LINE_SIZE];
                snprintf(e, sizeof(e), "[Line %d] Error: Expecting %d arguments\n", line_num, op_map.args);
                strcat(error_str, e);
                continue;
            }

            // Decide addressing mode
            // Deal with only the last argument
            char last_arg[32];
            if(op_map.args == 1) snprintf(last_arg, sizeof(last_arg), operand_a);
            else if(op_map.args == 2) snprintf(last_arg, sizeof(last_arg), operand_b);
            
            Addr_mode addressing_mode = ADDR_INVALID;
            if(is_register_name(last_arg)) addressing_mode = ADDR_REG;
            else if(last_arg[0] == MEM_CHAR) addressing_mode = ADDR_MEM;
            else addressing_mode = ADDR_DIR;

            char* last_arg_ptr = last_arg;
            
            switch (addressing_mode){
                case ADDR_INVALID:
                    char e[MAX_LINE_SIZE];
                    snprintf(e, sizeof(e), "[Line %d] Error: Invalid addressing mode on last argument\n", line_num, op_map.args);
                    strcat(error_str, e);
                continue;
                case ADDR_MEM:
                    // Remove @ sign
                    last_arg_ptr++;
                break;
                case ADDR_REG:
                    // Remove R
                    last_arg_ptr++;
                break;
            }
            
            long int last_arg_value = 0;
            char e[MAX_LINE_SIZE];
            int error_code = str_to_num(last_arg_ptr, &last_arg_value, e);
            switch (error_code){
                case 1:
                    snprintf(e, sizeof(e), "[Line %d] Error: variable not defined\n", line_num);
                    strcat(error_str, e);
                    continue;
                break;
                case 2:
                    snprintf(e, sizeof(e), "[Line %d] Error: invalid number\n", line_num);
                    strcat(error_str, e);
                    continue;
                break;
                case 3:
                    snprintf(e, sizeof(e), "[Line %d] Error: Value is out of range [%d, %d]\n", line_num, -(MAX_VALUE/2 + 1), (MAX_VALUE/2));
                    strcat(error_str, e);
                    continue;
                break;
            }

            // Check validity of value
            switch (addressing_mode){
                case ADDR_MEM:
                    // Variable memory being accessed
                    if(last_arg_value >= VAR_START_ADDR && last_arg_value < VAR_START_ADDR+MAX_VARIABLES){
                        snprintf(e, sizeof(e), "[Line %d] Error: Cannot directly access variable memory [%d, %d]\n", line_num, VAR_START_ADDR, VAR_START_ADDR+MAX_VARIABLES);
                        strcat(error_str, e);
                        continue;
                    }
                    break;
                break;
                case ADDR_REG:
                    //
                break;
            }

            printf("%d, %d\n", addressing_mode, last_arg_value);
        }

        snprintf(main_section[instr_addr], MACHINE_CODE_INSTR_LEN + 1, "%s%s%s", hex_opcode, hex_operand_a, hex_operand_b);
        instr_addr++;
    }   

    // Add HALT instruction
    snprintf(main_section[instr_addr], MACHINE_CODE_INSTR_LEN + 1, "%02x%04x%04x", OP_HALT, 0, 0);
    printf(error_str);
    
    fclose(asm_file_ptr);
    return 0;
}