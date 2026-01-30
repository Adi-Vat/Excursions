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
#define MAX_VALUE 65535
#define COMMENT_CHAR ';'
#define LABEL_CHAR ':'
#define HEX_CHAR '$'
#define BINARY_CHAR '%'

const char DEFAULT_HEX_FILE_NAME[] = "out.hex";
const char VAR_DIRECTIVE[] = "var";

// Check if a string is whitespace 
bool empty_line(const char* line){
    for(int i = 0; line[i]; i++){
        if(!isspace(line[i])) return false;
    }

    return true;
}

typedef struct{
    char name[MAX_LABEL_SIZE];
    uint16_t value;
} Label;

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

        char* line_trim_comment = remove_inline_comment(asm_line);
        char* line_trim = remove_whitespace(line_trim_comment);
        bool is_label = (line_trim[strlen(line_trim)-1] == LABEL_CHAR);

        if(is_label){
            int len = strlen(line_trim);
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

                char var_value_trim[MAX_LINE_SIZE] = {0};
                // Try convert value to decimal
                int base = 10;
                
                switch(var_value[0]){
                    case HEX_CHAR:
                        // Remove the identifier chars
                        strcpy(var_value_trim, var_value + 1);
                        base = 16;    
                    break;
                    case BINARY_CHAR:
                        strcpy(var_value_trim, var_value + 1);
                        base = 2;
                    break;
                    default:
                        strcpy(var_value_trim, var_value);
                    break;
                }

                char * end_ptr;

                long int var_num = strtol(var_value_trim, &end_ptr, base);

                char e[MAX_LINE_SIZE];
                bool is_error = false;
                // Check validity of number
                if(end_ptr == var_name){
                    is_error = true;
                    snprintf(e, sizeof(e), "[Line %d] Error: variable not defined", line_num);
                } else if(*end_ptr != '\0'){
                    is_error = true;
                    snprintf(e, sizeof(e), "[Line %d] Error: invalid number", line_num);
                } else if(var_num > MAX_VALUE/2 || var_num < -(MAX_VALUE/2 + 1)){
                    is_error = true;
                    snprintf(e, sizeof(e), "[Line %d] Error: value exceeds range [%d, %d]", line_num, -(MAX_VALUE/2 + 1), (MAX_VALUE/2));
                }
                
                if(is_error){
                    strcat(error_str, e);
                    continue;
                }

                // add variable to array
                strcpy(variables[vars_count].name, var_name);
                variables[vars_count].value = (uint16_t)var_num;
                vars_count++;
                printf("%s<\n", var_name);
            }
        }

        instr_addr ++;
    }

    /* Second pass
        Split instruction into mnemonic, operandA, operandB
        Decide opcode based on operands (addressing mode)
        Replace labels with their instruction index
        Replace variables with their locations in memory
        Replace variable decleration with LDM <address>, 
    */
    rewind(asm_file_ptr);

    printf(error_str);
    
    //while()
    fclose(asm_file_ptr);
    return 0;
}