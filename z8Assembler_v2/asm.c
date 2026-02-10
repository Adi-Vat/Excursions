#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include "shared.h"

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
            snprintf(out_hex_file_name, sizeof(out_hex_file_name), "hex_out/%s", argv[3]);
        }
    }
    else{
        char in_file_name[MAX_LINE_SIZE] = {0};
        snprintf(in_file_name, sizeof(in_file_name), argv[1]);
        char *dot = strrchr(in_file_name, '.');
        if (dot != NULL) {
            *dot = '\0';
        }
        
        int in_file_ptr;

        int len = strlen(in_file_name);
        for(int i = len; i >= 0; i--){
            if(in_file_name[i] == '\\' || in_file_name[i] == '/'){
                in_file_ptr = i;
                break;
            }   
        }

        snprintf(out_hex_file_name, sizeof(out_hex_file_name), "hex_out/%s.hex", in_file_name + in_file_ptr);
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

    int num_labels = 0;
    Label variables[MAX_VARIABLES];

    for(int i = 0; i < MAX_VARIABLES; i++) variables[i].name[0] = '\0'; 
    int num_vars = 0;

    uint16_t instr_addr = 0;
    uint16_t line_num = 0;
    
    char error_str[MAX_LINE_SIZE * MAX_ERRORS] = {0};
    int error_pos = 0;
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
           add_error(ERR_CHAR_LIM_EXCEED, line_num, error_str, sizeof(error_str), &error_pos);
           continue;
        }

        // convert to lowercase
        for(int i = 0; asm_line[i]; i++)
            asm_line[i] = tolower(asm_line[i]);
        
        remove_inline_comment(asm_line);
        char* line_trim = remove_whitespace(asm_line);
        if(strlen(line_trim) < 1) continue;

        bool is_label = (line_trim[strlen(line_trim)-1] == LABEL_CHAR);
        
        if(is_label){
            // remove :
            int len = strlen(line_trim);
            char label[MAX_NAME_SIZE] = {0};
            memcpy(label, line_trim, len - 1);
            label[len-1] = '\0';
            
            bool valid_label = true;
            
            Error_Code err = valid_name(label, line_num, labels, variables);
            if(err != ERR_NONE){
                add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                valid_label = false;
            }
            
            err = unique_name(label, line_num, labels, variables);
            if(err != ERR_NONE){
                add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                valid_label = false;
            }
            
            if(num_labels >= MAX_LABELS){
                err = ERR_MAX_LABELS_EXCEED;
                add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                valid_label = false;
            }
            
            if(!valid_label) continue;
            
            strcpy(labels[num_labels].name, label);
            labels[num_labels].value = instr_addr;
            num_labels ++;

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

                // parse variable
                if(sscanf(line_trim, ".%*15[a-zA-Z0-9_] %127[a-zA-Z0-9_] %127s", var_name, var_value) != 2 || !var_name){
                    add_error(ERR_BAD_VAR_DECLERATION, line_num, error_str, sizeof(error_str), &error_pos);
                    continue;
                }

                bool valid_var = true;
                
                Error_Code err = valid_name(var_name, line_num, labels, variables);
                if(err != ERR_NONE){
                    add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                    valid_var = false;
                }
                
                err = unique_name(var_name, line_num, labels, variables);
                if(err != ERR_NONE){
                    add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                    valid_var = false;
                }
                
                if(num_labels >= MAX_VARIABLES){
                    err = ERR_MAX_VARS_EXCEED;
                    add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                    valid_var = false;
                }
                
                if(!valid_var) continue;

                long int var_num;
                // convert str to number
                err = str_to_num(var_value, &var_num);
                if(err != ERR_NONE){
                    add_error(err, line_num, error_str, sizeof(error_str), &error_pos);
                    continue;
                }
                
                // add variable to array
                strcpy(variables[num_vars].name, var_name);
                variables[num_vars].value = (uint16_t)var_num;
                num_vars++;
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
    char data_section[num_vars][MACHINE_CODE_INSTR_LEN + 1];
    // allow for HALT instruction and conversion from index to length
    char main_section[num_instructions-num_vars+2][MACHINE_CODE_INSTR_LEN + 1];
    rewind(asm_file_ptr);
    line_num = 0;
    // index of the next available instruction slot
    instr_addr = 0;
    // index of the next available variable slot
    int next_avail_var_addr = 0;

    while(fgets(asm_line, MAX_LINE_SIZE, asm_file_ptr)){
        line_num++;

        // skip useless lines
        if(empty_line(asm_line)) continue;

        // convert to lowercase
        for(int i = 0; asm_line[i]; i++)
            asm_line[i] = tolower(asm_line[i]);

        remove_inline_comment(asm_line);
        char* line_trim = remove_whitespace(asm_line);
        // skip comment lines
        if(strlen(line_trim) < 1) continue;
        bool is_label = (line_trim[strlen(line_trim)-1] == LABEL_CHAR);
        if(is_label) continue;

        // replce directives
        char directive_type[16] = {0};
        bool is_directive = sscanf(line_trim, ".%15[a-zA-Z0-9_]", directive_type) > 0;

        int hex_opcode = 0;
        int hex_operand_a = 0;
        int hex_operand_b = 0;

        if(is_directive){
            if(strcmp(directive_type, VAR_DIRECTIVE) == 0){
                char var_name[MAX_LINE_SIZE] = {0};
                // parse variable
                // return if variable is malformed for some reason
                if(sscanf(line_trim, ".%*15[a-zA-Z0-9_] %127[a-zA-Z0-9_] %*127s", var_name) != 1){
                    add_error(ERR_BAD_VAR_DECLERATION, line_num, error_str, sizeof(error_str), &error_pos);
                    continue;
                }
                
                int var_index = index_of_label(variables, num_vars, var_name);
                // return if variable hasn't already been passed
                if(var_index == -1){
                    add_error(ERR_NAME_UNDECLARED, line_num, error_str, sizeof(error_str), &error_pos);
                    continue;
                }

                // parse declaration to STD <mem addr> <val>
                hex_opcode = opcode_map[OP_STORE][ADDR_DIR];
                hex_operand_a = (var_index + VAR_START_ADDR);
                hex_operand_b = variables[var_index].value;
                snprintf(data_section[next_avail_var_addr], sizeof(data_section[next_avail_var_addr]), "%02X%04X%04X\n", hex_opcode, hex_operand_a, hex_operand_b);
                next_avail_var_addr ++;
                continue;
            }   
        }
        else{
            // Parsing an instruction:
            // Identify the opcode
            // If the opcode exists, continue, if not, return
            // Extract the first operand
            // Find the type it's trying to be (register, memory, direct)
            // Check the operand's destination addressing mode againt this type.
            // If they match, continue, if not, return
            // If the mode is memory, check if the address is a variable.
            // If it is, get its absolute address instead. If not, return
            // Does the opcode have more than one operand?
            // Get the next operand
            // Extract operand type
            // Generate the right opcode from this type
            // convert from variable to address
            char opcode[32];

            // remove commas
            for(char *p = line_trim; *p; p++)
                if(*p == ',') *p = ' ';
            
            // get number of tokens
            int num_tokens = 0;

            if(!isspace(line_trim[0])) num_tokens++;

            for(char *p = line_trim; *p; p++){
                if(*(p+1) && isspace(*p) && !isspace(*(p+1)))
                    num_tokens++;
            }
            
            // extract opcode
            sscanf(line_trim, "%32s", opcode);
            
            Operation_Mapping this_op_map = find_operation(opcode);

            if(this_op_map.op == OP_INVALID){
                add_error(ERR_INVALID_OPCODE, line_num, error_str, sizeof(error_str), &error_pos);
                continue;
            }

            // number of args given against args expected
            if(this_op_map.args != num_tokens - 1){
                add_error(ERR_UNEXPECTED_OPERANDS, line_num, error_str, sizeof(error_str), &error_pos);
                continue;
            }

            if(this_op_map.args == 0) hex_opcode = opcode_map[this_op_map.op][0];

            for(int i = 0; i < this_op_map.args; i++){
                bool is_op_a = i == 0;
                char this_operand[32];
                // extract first argument
                int this_op_ptr = 0;
                // get the first or second arguments
                sscanf(line_trim, is_op_a ? "%*32s %32s" : "%*32s %*32s %32s", this_operand);
                
                Addr_Mode this_op_addr_mode = get_addr_mode(this_operand);
                
                // check addressing mode matches what's expected for operand A
                if(is_op_a && this_op_map.op != OP_PUSH){
                    if(this_op_map.dest_addr_mode != this_op_addr_mode){
                        add_error(ERR_BAD_ADDR_MODE, line_num, error_str, sizeof(error_str), &error_pos);
                        continue;
                    }
                    hex_opcode = opcode_map[this_op_map.op][this_op_map.dest_addr_mode];
                    
                } else{
                    // if it's operand B, decompose the opode based on address mode
                    hex_opcode = opcode_map[this_op_map.op][this_op_addr_mode];
                }

                // check invalid opcode on FINAL pass
                if(i == this_op_map.args - 1 && hex_opcode == 0xFF){
                    add_error(ERR_BAD_ADDR_MODE, line_num, error_str, sizeof(error_str), &error_pos);
                    continue;
                }

                int this_op_val = 0;

                // remove @ or R
                if(this_op_addr_mode == ADDR_MEM || this_op_addr_mode == ADDR_REG) this_op_ptr ++;

                // Do validity checks and find the absolute value of operand A
                switch(this_op_addr_mode){
                    case ADDR_MEM: {
                        int this_var_addr = index_of_label(variables, num_vars, (this_operand + this_op_ptr));
                        // if it's a variable, store its absolute address
                        if(this_var_addr > -1){
                            this_op_val = this_var_addr + VAR_START_ADDR;
                            break;
                        }
                        
                        // If the first char isn't a radix prefix OR a number, 
                        // then they're trying to access a variable that doesn't exist
                        char first_char = (this_operand + this_op_ptr)[0];
                        if(!(is_protected_char(first_char) || isdigit(first_char) || is_bank_name(this_operand + this_op_ptr))){
                            add_error(ERR_NAME_UNDECLARED, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }
                        
                        // Check they're not trying to access an output bank
                        if(is_bank_name(this_operand + this_op_ptr)){
                            char second_char = (this_operand + this_op_ptr + 1)[0];
                            // Bank access is done with b<bank number><row>
                            // Check if a valid bank is being acccessed
                            bool valid_bank = second_char == '0' || second_char == '1';
                            if(!valid_bank){
                                add_error(ERR_BANK_CONFLICT, line_num, error_str, sizeof(error_str), &error_pos);
                                continue;
                            }

                            // now we know it's a bank being accessed, try and convert the bank index to a byte index
                            // third character stores the byte being accessed
                            long int byte_to_access = 0;
                            Error_Code e =  str_to_num(this_operand + this_op_ptr + 2, &byte_to_access);
                            if(e != ERR_NONE){
                                add_error(e, line_num, error_str, sizeof(error_str), &error_pos);
                                continue;
                            }

                            // bank 0 access, easy mapping
                            if(second_char == '0'){
                                if(byte_to_access < 0 || byte_to_access >= BANK_0_SIZE){
                                    // trying to access an invalid byte from the bank
                                    add_error(ERR_INVALID_BANK0_ADDR, line_num, error_str, sizeof(error_str), &error_pos);
                                    continue;
                                }

                                if(this_op_map.op == OP_CB || this_op_map.op == OP_SB){
                                    if(hex_operand_a < 0 || hex_operand_a > WORD_SIZE-1){
                                        add_error(ERR_BIT_OUT_OF_RANGE, line_num, error_str, sizeof(error_str), &error_pos);
                                        continue;
                                    }
                                }
                                
                                this_op_val = BANK_0_START + byte_to_access;
                            } else{
                                // If they're trying to acesss bank 1 with an instruction other than SB or CB;
                                // tell them NO! for now. Because how are you meant to assign an 8 bit number to a 12 bit space?
                                if(this_op_map.op != OP_CB && this_op_map.op != OP_SB || is_op_a){
                                    add_error(ERR_UNSAFE_BANK1_OP, line_num, error_str, sizeof(error_str), &error_pos);
                                    continue;
                                }

                                if(byte_to_access < 0 || byte_to_access >= 2){
                                    // trying to access an invalid row from the bank
                                    add_error(ERR_INVALID_BANK1_ADDR, line_num, error_str, sizeof(error_str), &error_pos);
                                    continue;
                                }

                                // Convert from user facing 2 row access to internal 4 row
                                // get bit trying to be accessed, 0-11 (incl.)
                                if(hex_operand_a < 0 || hex_operand_a > 11){
                                    add_error(ERR_BIT_OUT_OF_RANGE, line_num, error_str, sizeof(error_str), &error_pos);
                                    continue;
                                }
                                // Convert from 2x12 access to 4x6 access
                                int converted_column = hex_operand_a % 6;
                                int converted_row = (byte_to_access << 1) + (hex_operand_a/6);
                                hex_operand_a = converted_column;
                                this_op_val = converted_row + BANK_1_START;
                            }

                            break;
                        }

                        // If it's a memory address, check that it's not a protected address or an invalid address
                        long int str_to_num_out = 0;

                        Error_Code e = str_to_num((this_operand + this_op_ptr), &str_to_num_out);
                        if(e != ERR_NONE){
                            add_error(e, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }

                        // prevent access of protected memory
                        if(str_to_num_out >= VAR_START_ADDR && str_to_num_out < VAR_START_ADDR+MAX_VARIABLES){
                            add_error(ERR_PROT_MEM_ACCESSED, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }
                        
                        this_op_val = (int)str_to_num_out;
                    break;
                    }
                    case ADDR_REG: {
                        // Get index of the register
                        long int register_index = -1;
                        Error_Code e = str_to_num((this_operand + this_op_ptr), &register_index);

                        if(e != ERR_NONE){
                            add_error(e, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }

                        // make sure it's accessable
                        if(register_index < 0 || register_index >= REGISTERS_COUNT){
                            add_error(ERR_INVALID_REGISTER, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }

                        this_op_val = (int)register_index;
                    break;
                    }
                    case ADDR_DIR: {
                        int this_label_addr = index_of_label(labels, num_labels, (this_operand + this_op_ptr));
                        
                        // if it's a label, store its absolute address
                        if(this_label_addr > -1){
                            this_op_val = labels[this_label_addr].value;
                            break;
                        }

                        // If the first char isn't (a radix prefix OR a number), 
                        // then they're trying to access a label that doesn't exist
                        char first_char = (this_operand + this_op_ptr)[0];
                        if(!(is_protected_char(first_char) || isdigit(first_char))){
                            add_error(ERR_NAME_UNDECLARED, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }
                        
                        long int direct_value = 0;
                        Error_Code e = str_to_num((this_operand + this_op_ptr), &direct_value);

                        if(e != ERR_NONE){
                            add_error(e, line_num, error_str, sizeof(error_str), &error_pos);
                            continue;
                        }

                        this_op_val = (int)direct_value;
                    break;
                    }
                }

                if(is_op_a) hex_operand_a = this_op_val;
                else hex_operand_b = this_op_val;
            }

            // Emit to machine code
            snprintf(main_section[instr_addr], sizeof(main_section[instr_addr]), "%02X%04X%04X\n", hex_opcode, hex_operand_a, hex_operand_b);

            instr_addr++;
        }
    }   

    // Add HALT instruction
    snprintf(main_section[instr_addr], MACHINE_CODE_INSTR_LEN + 1, "%02x%04x%04x", opcode_map[OP_HALT][0], 0, 0);
    instr_addr++;

    if(error_str[0]){
        printf(error_str);  
        fclose(asm_file_ptr);
        return 1;
    }

    // Write to hex file
    FILE *hex_file_ptr;

    hex_file_ptr = fopen(out_hex_file_name, "w");
    
    for(int i = 0; i < num_vars; i++) fprintf(hex_file_ptr, "%s\n", data_section[i]);
    for(int i = 0; i < instr_addr; i++)  fprintf(hex_file_ptr, "%s\n", main_section[i]);
    
    fclose(hex_file_ptr);
    return 0;
}