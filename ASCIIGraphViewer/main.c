#include <stdio.h>
#include <windows.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30
#define MAX_ZOOM 50
#define MIN_ZOOM 0.1f

float f(float x);
void drawGraph();
void drawScreen();
void clearScreen();

int screenTexture[SCREEN_WIDTH][SCREEN_HEIGHT];
float xOffset = 0;
float yOffset = 0;
//float yZoom = 1;
//float xZoom = 1;
float zoom = 1;

float selectedX;
float selectedY;
float derivative;

int main(){
    bool shouldCloseWindow;

    
    while(!shouldCloseWindow){
        clearScreen();
        drawGraph();
        drawScreen();
        char input[128];
        scanf("%s", input);

        if(strcmp(input, "w") == 0){
            yOffset += 1/(float)zoom;
        } else if(strcmp(input, "s") == 0){
            yOffset -= 1/(float)zoom;
        } else if(strcmp(input, "a") == 0){
            xOffset -= 1/(float)zoom;
        } else if(strcmp(input, "d") == 0){
            xOffset += 1/(float)zoom;
        } else if(strcmp(input, "o") == 0){
            xOffset = 0;
            yOffset = 0;
            zoom = 1;
        } else if(strcmp(input, "z") == 0){
            zoom *= 2;
            if(zoom > MAX_ZOOM) zoom = MAX_ZOOM;
        } else if(strcmp(input, "x") == 0){
            zoom /= 2;
            if(zoom < MIN_ZOOM) zoom = MIN_ZOOM;
        } else if(strcmp(input, "c") == 0){
            xOffset = selectedX;
            yOffset = selectedY;
        }

        fflush(stdout);
        //Sleep(100);  // Windows Sleep() in milliseconds
    }
}

void clearScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        printf("\033[2J\033[H");
    #endif // Clear and home
}

float f(float x){
    //float x1 = x/4;
    if (fabs(x) < 0.0001) return 10;
    float out = 10*(sin(x)/x);
    //float out = 1/x1;
    return out;
}

float dBydx(float x){
    float dx = 0.0001f;
    float fLeft = f(x-dx);
    float fRight = f(x+dx);
    float d = (fRight - fLeft)/(2.0f*dx);
    return d;
}

void drawGraph(){
    for(int i = 0; i < SCREEN_WIDTH; i++){
        for(int k = 0; k < SCREEN_HEIGHT; k++){

            //float x = (i - (float)SCREEN_WIDTH/2)/zoom + xOffset;
            //float y = (k - (float)SCREEN_HEIGHT/2)/zoom + yOffset;
            int xAxisColumn = round((-xOffset) * zoom + SCREEN_WIDTH/2);
            int yAxisColumn = round((-yOffset) * zoom + SCREEN_HEIGHT/2);

            int characterIndex = 0;
            if(i == xAxisColumn && k == yAxisColumn - 1) characterIndex = 2;
            else if(k == yAxisColumn - 1) characterIndex = 3;
            else if(i == xAxisColumn) characterIndex = 4;

            screenTexture[i][k] = characterIndex;
        }
    }

    int previousK = 0;

    for(int i = -1; i < SCREEN_WIDTH; i++){
        // convert horizontal screen index to world space
        // i = SCREEN_WIDTH/2 => x = xOffset
        float x = (i - (float)SCREEN_WIDTH/2)/zoom + xOffset;
        float y = f(x);

        if(y > 100) y = 100;
        else if(y < -100) y = -100;

        int k = floor((y - yOffset) * zoom + (float)SCREEN_HEIGHT/2);

        int less = previousK;
        int more = k;
        if(previousK > k){
            less = k;
            more = previousK;
        }

        previousK = k;

        if(i == -1) continue;

        if(less == more) more++;

        for(int j = less; j < more; j++){
            if(j > 0 && j < SCREEN_HEIGHT){
                screenTexture[i][j] = 1;
            }
        }

        if(i == SCREEN_WIDTH/2){
            selectedX = x;
            selectedY = y;
            derivative = dBydx(x);
        }
        

        if(less > 0 && less < SCREEN_HEIGHT){
            if(i == SCREEN_WIDTH/2){
                screenTexture[i][less] = 7;
            }
        }
    }
}

void drawScreen(){
    printf("|ASCII Graph Viewer : y = 10(sin(x)/x) : (%.1f, %.1f) : d/dx ~= %.2f|\n", selectedX, selectedY, derivative);
    for(int y = SCREEN_HEIGHT - 1; y >= 0; y--){
        for(int x = 0; x < SCREEN_WIDTH; x++){
            int characterValue = screenTexture[x][y];

            if(x == 0 || x == SCREEN_WIDTH - 1){
                characterValue = 4;
            }
            else if(y == 0 || y == SCREEN_HEIGHT - 1){
                characterValue = 3;
            }
            
            switch(characterValue){
                case 0:
                    printf(" ");
                    break;
                case 1:
                    printf("*");
                    break;
                case 2:
                    printf("+");
                    break;
                case 3:
                    printf("-");
                    break;
                case 4:
                    printf("|");
                    break;
                case 5:
                    printf("v");
                    break;
                case 6:
                    printf("^");
                    break;
                case 7:
                    printf("@");
                    break;
                default:
                    printf(".");
                    break;
            }
            if(x < SCREEN_WIDTH-1) printf(" ");
        }
        printf("\n");
    }
    printf("|w: up| |s: down| |a: left| |d: right| |o: origin| |c: centre|\n|z: zoom in| |x: zoom out|\n");
}