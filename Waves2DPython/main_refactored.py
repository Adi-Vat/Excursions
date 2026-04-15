import pygame
import math

C = 500
TIME_STEP = 0.0005
DISTANCE_STEP = 1
FIELD_PER_PIXEL = 0.075
SCREEN_WIDTH, SCREEN_HEIGHT = (800, 600)


class WaveField:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.field = [[0]*height for _ in range(width)]
        self.oldField = [[0]*height for _ in range(width)]
        self.newField = [[0]*height for _ in range(width)]

    def computeFieldEquation(self, i, k, elapsed_time):
        amplitude = self.field[i][k]
        oldAmplitude = self.oldField[i][k]

        leftIndex = i - 1
        rightIndex = i + 1
        upIndex = k - 1
        downIndex = k + 1

        if leftIndex < 0:
            leftAmplitude = 0
        else:
            leftAmplitude = self.field[leftIndex][k]

        if rightIndex > self.width - 1:
            rightAmplitude = 0
        else:
            rightAmplitude = self.field[rightIndex][k]

        if upIndex < 0:
            upAmplitude = 0
        else:
            upAmplitude = self.field[i][upIndex]

        if downIndex > self.height - 1:
            downAmplitude = 0
        else:
            downAmplitude = self.field[i][downIndex]

        sourceAmount = 0
        if i == self.width // 2 and k == self.height // 2:
            sourceAmount = math.sin(elapsed_time * 10)

        laplacian = (
            rightAmplitude + leftAmplitude +
            upAmplitude + downAmplitude -
            4 * amplitude
        ) / (DISTANCE_STEP * DISTANCE_STEP)

        newAmplitude = (
            2 * amplitude
            - oldAmplitude
            + C * C * TIME_STEP * TIME_STEP * laplacian
            + sourceAmount
        )

        if i == 15 and (k < (self.height // 2) - 2 or k > (self.height // 2) + 2):
            newAmplitude = 0

        newAmplitude *= 0.995
        return newAmplitude

    def calculateNextField(self, elapsed_time):
        self.oldField = [row[:] for row in self.field]
        self.field = [row[:] for row in self.newField]

        for i in range(self.width):
            for k in range(self.height):
                self.newField[i][k] = self.computeFieldEquation(i, k, elapsed_time)


class Renderer:
    def __init__(self, screen):
        self.screen = screen

    def drawField(self, wave_field):
        for x in range(wave_field.width):
            for y in range(wave_field.height):
                amplitude = abs(wave_field.field[x][y])

                if amplitude > 0.01:
                    screenX = x / FIELD_PER_PIXEL
                    screenY = y / FIELD_PER_PIXEL

                    radius = min(3, int(amplitude * 3))
                    pygame.draw.circle(
                        self.screen,
                        (255, 255, 255),
                        (int(screenX), int(screenY)),
                        radius
                    )


class App:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("2D waves")
        self.clock = pygame.time.Clock()

        fieldWidth = int(SCREEN_WIDTH * FIELD_PER_PIXEL)
        fieldHeight = int(SCREEN_HEIGHT * FIELD_PER_PIXEL)

        self.wave = WaveField(fieldWidth, fieldHeight)
        self.renderer = Renderer(self.screen)

        self.start_time = pygame.time.get_ticks() / 1000.0
        self.running = True

    def run(self):
        while self.running:
            self.handleEvents()

            elapsed_time = pygame.time.get_ticks() / 1000.0 - self.start_time

            self.screen.fill((0, 0, 0))
            self.wave.calculateNextField(elapsed_time)
            self.renderer.drawField(self.wave)

            pygame.display.flip()
            self.clock.tick(60)

        pygame.quit()

    def handleEvents(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    self.resetField()

    def resetField(self):
        self.wave.field = [[0]*self.wave.height for _ in range(self.wave.width)]
        self.wave.oldField = [[0]*self.wave.height for _ in range(self.wave.width)]
        self.wave.newField = [[0]*self.wave.height for _ in range(self.wave.width)]


if __name__ == "__main__":
    app = App()
    app.run()
