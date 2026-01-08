#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "shared.h"


int indexOf(char* array, char* element, int arrayLength, int elementSize)
{
    int rowIndex = 0;

    for(int i = 0; i <= ((arrayLength - 1) * elementSize); i += elementSize){
        char str[elementSize];

        strcpy(str, &array[i]);

        if(strcmp(str, element) == 0){
            return rowIndex;
            printf("\n");
        }

        rowIndex++;
    }

    return -1;
}

int hexToInt(char hex[], int length){
    int value = 0;
    
    for(int i = 0; i < length; i++){
        char c = hex[i];
        int v;
        if(c >= '0' && c <= '9')
            v = c - '0';
        else if(c >= 'A' && c <= 'F')
            v = c - 'A' + 10;
        else if(c >= 'a' && c <= 'f')
            v = c - 'a' + 10;
        else
            return -1; // Invalid hex character

        value = (value << 4) | v; // Shift left by 4 bits and add new value
    }

    return value;
}