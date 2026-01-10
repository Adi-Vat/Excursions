#include <stdio.h>
#include <string.h>

int main(){
    char transferFunction[256];
    sprintf(transferFunction,  "(s)//(s+4)");
    char* numeratorStr = strtok(transferFunction, "//");
    printf("%s\n", numeratorStr);
    char * denominatorStr = strtok(NULL, transferFunction);
    printf(denominatorStr);
    return 0;
}