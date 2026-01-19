import pygame
import math
import time

C = 500
TIME_STEP = 0.0005
DISTANCE_STEP = 1
FIELD_PER_PIXEL = 0.075
SCREEN_WIDTH, SCREEN_HEIGHT = (800, 600)
fieldWidth = int(SCREEN_WIDTH * FIELD_PER_PIXEL)
fieldHeight = int(SCREEN_HEIGHT * FIELD_PER_PIXEL)
field = [[0]*fieldHeight for _ in range(fieldWidth)]
oldField = [[0]*fieldHeight for _ in range(fieldWidth)]
newField = [[0]*fieldHeight for _ in range(fieldWidth)]
start_time = pygame.time.get_ticks() / 1000.0

def computeFieldEquation(i, k):
    global field, oldField, newField
    amplitude = field[i][k]
    oldAmplitude = oldField[i][k]
    leftIndex = i-1
    rightIndex = i + 1
    upIndex = k-1
    downIndex = k + 1

    if leftIndex < 0: leftIndex = fieldWidth - 1
    leftAmplitude = field[leftIndex][k]
    if rightIndex > fieldWidth - 1: rightIndex = 0
    rightAmplitude = field[rightIndex][k]
    if upIndex < 0: upIndex = fieldHeight - 1
    upAmplitude = field[i][upIndex]
    if downIndex > fieldHeight - 1: downIndex = 0
    downAmplitude = field[i][downIndex]

    '''
    if leftIndex < 0: leftAmplitude = 0
    else: leftAmplitude = field[leftIndex][k]

    if rightIndex > fieldWidth - 1: rightAmplitude = 0
    else: rightAmplitude = field[rightIndex][k]

    if upIndex < 0: upAmplitude = 0
    else: upAmplitude = field[i][upIndex]
    
    if downIndex > fieldHeight - 1: downAmplitude = 0
    else: downAmplitude = field[i][downIndex]
    '''
    sourceAmount = 0
    elapsed_time = pygame.time.get_ticks() / 1000.0 - start_time  # Use elapsed time
    if i == fieldWidth // 2 and k == fieldHeight // 2:
        elapsed_time = pygame.time.get_ticks() / 1000.0 - start_time
        sourceAmount = 1 * math.sin(elapsed_time * 10)
    
    laplacian = (rightAmplitude + leftAmplitude + upAmplitude + downAmplitude - 4*amplitude) / (DISTANCE_STEP * DISTANCE_STEP)
    
    newAmplitude = 2 * amplitude - oldAmplitude + C*C*TIME_STEP*TIME_STEP*laplacian + sourceAmount
    
    if i == 15 and (k < (fieldHeight // 2) - 2 or k > (fieldHeight // 2) + 2): newAmplitude = 0
    
    newAmplitude *= 0.995
    return newAmplitude

def calculateNextField():
    global field, oldField, newField
    oldField = [row[:] for row in field]
    field = [row[:] for row in newField]

    for i in range(fieldWidth):
        for k in range(fieldHeight):
            newAmplitude = computeFieldEquation(i, k)
            newField[i][k] = newAmplitude

def drawField():
    for x in range(fieldWidth):
        for y in range(fieldHeight):
            screenX = x/FIELD_PER_PIXEL
            screenY = y/FIELD_PER_PIXEL
            amplitude = abs(field[x][y])  # Get absolute value
            if amplitude > 0.01:  # Only draw if visible
                radius = min(3, int(amplitude * 3))  # Cap the radius
                pygame.draw.circle(screen, (255, 255, 255), (int(screenX), int(screenY)), radius)

pygame.init()

screen =pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
pygame.display.set_caption("2D waves")
clock = pygame.time.Clock()

running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_r:
                field = oldField = newField = [[0]*fieldHeight for _ in range(fieldWidth)]

    screen.fill((0, 0, 0))
    calculateNextField()
    drawField()

    pygame.display.flip()
    clock.tick(60)

pygame.quit()

