# av853
import pygame
import math

# av853
# speed of wave propogation
C = 50
# time step used to evaluate
TIME_STEP = 0.005
# distance between adjacent cell
DISTANCE_STEP = 1
# density of field on screen
FIELD_PER_PIXEL = 0.075
SCREEN_WIDTH, SCREEN_HEIGHT = (800, 600)
# therefore, grid dimensions = 60x45 cells
ATTENTUATION = 0.995
STEPS_PER_FRAME = 5

assert C * TIME_STEP / DISTANCE_STEP <= 0.707

class WaveField:
    """
    Represents a 2D wave simulation grid.
    Handles storing the wave state and computing how the wave evolved over time
    """
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.field = [[0]*height for _ in range(width)]
        self.oldField = [[0]*height for _ in range(width)]
        self.newField = [[0]*height for _ in range(width)]
        self.sources = []

    def computeFieldEquation(self, i, k, elapsed_time, field_sources):
        """
        Calculates the new amplitude of a cell in the grid.
        Uses neighbouring cells and the 2D wave equation to calculate wave propagation.

        Parameters:
            i (int): x-coord in grid
            k (int): y-coord in grid
            elapsed_time (float): time since simulation started

        Returns:
            float: new amplitude value
        """
        amplitude = self.field[i][k]
        oldAmplitude = self.oldField[i][k]

        leftIndex = i - 1
        rightIndex = i + 1
        upIndex = k - 1
        downIndex = k + 1

        # Check if neighbouring cell is valid or not
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

        # apply sources
        sourceAmount = 0
        for source in field_sources.sources:
            source_gridX = round(source.position[0] * FIELD_PER_PIXEL)
            source_gridY = round(source.position[1] * FIELD_PER_PIXEL)
            
            if i == source_gridX and k == source_gridY:
                sourceAmount = source.amplitude * math.sin(elapsed_time * 2 * math.pi * source.frequency + source.phase)


        # Apply discrete 2nd order derivative
        laplacian = ( rightAmplitude + leftAmplitude
                     + upAmplitude + downAmplitude 
                     - 4 * amplitude
        ) / (DISTANCE_STEP * DISTANCE_STEP)

        newAmplitude = (
            2 * amplitude
            - oldAmplitude
            + C * C * TIME_STEP * TIME_STEP * laplacian
            + sourceAmount
        )
        

        # apply walls
        if (i == 15 
            and (k < (self.height // 2) - 2
            or k > (self.height // 2) + 2)):
            newAmplitude = 0

        # attenuate
        newAmplitude *= ATTENTUATION
        return newAmplitude

    def calculateNextField(self, elapsed_time, field_sources):
        """
        Updates the entire field by computing the next state for each cell in the grid
        """
        self.oldField = [row[:] for row in self.field]
        self.field = [row[:] for row in self.newField]

        for i in range(self.width):
            for k in range(self.height):
                self.newField[i][k] = self.computeFieldEquation(i, k, elapsed_time, field_sources)


class Renderer:
    """
    Draws the wave onto the screen using pygame.
    """
    def __init__(self, screen):
        self.screen = screen

    def drawField(self, wave_field, sensor_array, source_array):
        """
        Draws the wave field onto the screen.
        Each cell is rendered as a circle whos size is dependant on the wave amplitude at that point.
        
        Parameters:
            wave_field (WaveField): wave field to be drawn
            sensor_array (list[Sensor]) sensors to be drawn
        """
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

        for sensor in sensor_array.sensors:
            pygame.draw.circle(self.screen, (255,0,0), sensor.position, 5)
        
        for source in source_array.sources:
            pygame.draw.circle(self.screen, (0, 0, 255), source.position, 5)

class Sensor:
    """
    Can be placed anywhere on the grid, reads the EM field intensity at that point.
    """
    def __init__(self, position, id):
        # position is stored as screen-pos (pixels)
        self.position = position
        self.radius = 5
        self.id = id
    
    """
    Gets the strength of the EM field under the sensor

    Parameters:
        wave_field (WaveField): wave field to be considered
    
    returns a float of the value of the field
    """
    def read_field(self, wave_field):
        gridX = round(self.position[0] * FIELD_PER_PIXEL)
        gridY = round(self.position[1] * FIELD_PER_PIXEL)
        return(wave_field.field[gridX][gridY])


class SensorArray:
    """
    Contains all the sensors that the user places
    """
    def __init__(self):
        self.sensors = []
        self.recording = False

    def add_sensor(self, sensor):
        self.sensors.append(sensor)

    def remove_sensor(self, mouse_pos):
        for sensor in self.sensors:
            if math.dist(mouse_pos, sensor.position) <= sensor.radius:
                self.sensors.remove(sensor)
        
    """
    Forces all sensors to re-read and writes their outputs to a CSV file

    Parameters:
        wave_field (WaveField): wave field being considered
        output_file (File): file being written to
        elapsed_time (float): time since program start
    """
    def update_sensors(self, wave_field, output_file, elapsed_time):
        if not self.recording: return

        output_file.write(str(round(elapsed_time, 4)) + ',')

        for i in range(len(self.sensors)):
            sensor_value = self.sensors[i].read_field(wave_field)

            end_char = ','
            if i == len(self.sensors) - 1: end_char = '\n'

            output_file.write(str(sensor_value) + end_char)
        
class Source:
    def __init__(self, position, amplitude, frequency, phase):
        self.position = position
        self.amplitude = amplitude
        self.frequency = frequency
        self.phase = phase
        self.radius = 5
    
class Sources:
    def __init__(self):
        self.sources = []
    
    def add_source(self, source):
        self.sources.append(source)
    
    def remove_source(self, mouse_pos):
        for source in self.sources:
            if math.dist(mouse_pos, source.position) <= source.radius:
                self.sensors.remove(source)

class App:
    """
    Main application class,
    Controls the game loop, user input and coordinates simulation and rendering
    """
    def __init__(self):
        # Initialise things that need to be initialised.
        pygame.init()
        self.output_file = open('output.csv', 'w')
        self.sensor_array = SensorArray()
        # unique sensor ids established here
        self.next_sensor_id = 0

        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("2D waves - refactored")
        self.clock = pygame.time.Clock()

        fieldWidth = int(SCREEN_WIDTH * FIELD_PER_PIXEL)
        fieldHeight = int(SCREEN_HEIGHT * FIELD_PER_PIXEL)
        
        self.wave = WaveField(fieldWidth, fieldHeight)
        self.renderer = Renderer(self.screen)

        self.sources = Sources()
        self.sources.add_source(Source(position=(300, 500), amplitude=1, frequency=4, phase=0))
        self.sources.add_source(Source(position=(600, 200), amplitude=2, frequency=2, phase=0))
        self.sources.add_source(Source(position=(100, 300), amplitude=1.5, frequency=6, phase=0))
        self.sources.add_source(Source(position=(500, 300), amplitude=1, frequency=5, phase=0))
        self.elapsed_time = 0
        self.start_time = pygame.time.get_ticks() / 1000.0
        self.running = True
        

    # handles continuous event updates etc
    def run(self):
        while self.running:
            self.handleEvents()
            
            for _ in range(STEPS_PER_FRAME):
                self.elapsed_time += TIME_STEP
                self.wave.calculateNextField(self.elapsed_time, self.sources)
                self.sensor_array.update_sensors(self.wave, self.output_file, self.elapsed_time)
                if self.sensor_array.recording:
                    if self.elapsed_time >= 30:
                        self.running = False
                        break

            self.screen.fill((0, 0, 0))
            self.renderer.drawField(self.wave, self.sensor_array, self.sources)
            pygame.display.flip()
            self.clock.tick(60)

        self.output_file.close()
        pygame.quit()

    def handleEvents(self):
        """
        Handles button presses like reset and quit.
        """
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_r:
                    self.resetField()

                if event.key == pygame.K_g:
                    self.sensor_array.recording = not self.sensor_array.recording

            if event.type == pygame.MOUSEBUTTONDOWN:
                # lmb
                if event.button == 1:
                    x, y = event.pos
                    self.addSensor((x, y))
                # mmb
                if event.button == 2:
                    x, y = event.pos
                    self.sensor_array.remove_sensor((x,y))

    def resetField(self):
        self.wave.field = [[0]*self.wave.height for _ in range(self.wave.width)]
        self.wave.oldField = [[0]*self.wave.height for _ in range(self.wave.width)]
        self.wave.newField = [[0]*self.wave.height for _ in range(self.wave.width)]
        self.start_time = pygame.time.get_ticks() / 1000.0
        self.elapsed_time = 0

    def addSensor(self, position):
        new_sensor = Sensor(position, self.next_sensor_id)
        self.sensor_array.add_sensor(new_sensor)
        self.next_sensor_id += 1

if __name__ == "__main__":
    app = App()
    app.run()
