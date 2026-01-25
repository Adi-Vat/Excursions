import numpy as np
import math
import cv2
# AV853
SCREEN_W, SCREEN_H = 800, 600
img = cv2.imread("Foyer_Floor2.png")
# resizing the background image to fit the screen
h, w = img.shape[:2]
scale = min(SCREEN_W / w, SCREEN_H / h)
new_w, new_h = int(w * scale), int(h * scale)
img_resized = cv2.resize(img, (new_w, new_h), interpolation=cv2.INTER_AREA)

# transmitter locations (m)
# transmitter 1 865.5
x1, y1, d1 = 2.81, 0.74, 0
# transmitter 2 868.0
x2, y2, d2 = 8.77, 0.79, 0
# transmitter 3 870.5
x3, y3, d3 = 3.93, 10.97, 0
# transmitter 4 872.5
x4, y4, d4 = 8.77, 10.04, 0

# power levels (dbm)
# point 1
p1, p2, p3, p4 = -58, -61, -48, -54
# point 2
#p1, p2, p3, p4 = -57, -68, -48, -53
# point 4
#p1, p2, p3, p4 = -60, -57, -63, -60

originX = 0
originY = 0

# coefficients of power-distance log graph
m = -17.476
c = -43.004

# calculated position of the reciever
pos = (0,0)

def get_distance_from_rssi(rssi):
    # rssi = m * log(10)(dist) - c 
    # [rearranged to] dist = 10^(rssi-c)/m
    exponent = (rssi - c)/m
    dist = 10**exponent
    return dist

def getPosition(A, b):
    W = np.diag(1 / dists[1:]**2)  # weights for eqns 2–4
    Aw = W @ A # matrix magic lol vvvv
    bw = W @ b # multiplies the original distance matrices with a weighting to reduce anomalous values

    # equivilent to pos = (Aw^T Aw)^-1 Aw^T b
    pos = np.linalg.lstsq(Aw, bw, rcond=None)[0]
    return pos

def applymatrix():
    global pos

    # Subtracting equation 1 from equations 2, 3, 4
    A = np.array([
        [2*(x2-x1), 2*(y2-y1)],
        [2*(x3-x1), 2*(y3-y1)],
        [2*(x4-x1), 2*(y4-y1)]
    ])
    
    b = np.array([
        x2**2 - x1**2 + y2**2 - y1**2 + d1**2 - d2**2,
        x3**2 - x1**2 + y3**2 - y1**2 + d1**2 - d3**2,
        x4**2 - x1**2 + y4**2 - y1**2 + d1**2 - d4**2
    ])
    
    pos = getPosition(A, b)
    print(f"\nPosition: x={pos[0]:.2f}, y={pos[1]:.2f}")

# could be shortened by using arrays instead
# or my own class of "transmitter" that has x,y coordinates and power level
d1 = get_distance_from_rssi(p1)
d2 = get_distance_from_rssi(p2)
d3 = get_distance_from_rssi(p3)
d4 = get_distance_from_rssi(p4)
dists = np.array([d1, d2, d3, d4])
med = np.median(dists)

# fun little vector trick :)
mask = dists < 2.0 * med   # reject absurd ranges

print(d1, " ", d2, " ", d3, " ",d4)

applymatrix()

# set up a bunch of these variables
drawingLine = False
lineStartPoint = (-1,-1)
lineEndPoint = (-1, -1)
mousePos = (0,0)
pixelsToM = 0


# called when the mouse does something
def mouse_callback(event, x, y, flags, param):
    global originX, originY, drawingLine, lineStartPoint, lineEndPoint, mousePos, pixelsToM
    mousePos = (x,y)

    # set the origin position with the LMB
    if event == cv2.EVENT_LBUTTONDOWN:
        x_orig = int(x / scale)
        y_orig = int(y / scale)
        print(f"Scaled: ({x},{y}) → Original: ({x_orig},{y_orig})")
        originX = x
        originY = y
    # start drawing the reference line with the RMB going down
    if event == cv2.EVENT_RBUTTONDOWN:
        drawingLine = True
        lineStartPoint = (x, y)
    # end drawing the reference line with th RMB coming up
    if event == cv2.EVENT_RBUTTONUP:
        drawingLine = False
        lineEndPoint = (x,y)
        # get pixel distance that the line covers
        d = math.dist(lineStartPoint, lineEndPoint)
        # convert to m knowing that the distance is always 10.97m (lazy but whatever)
        pixelsToM = d/10.97

cv2.namedWindow("Position Solver")
cv2.setMouseCallback("Position Solver", mouse_callback)

while True:
    frame = img_resized.copy()
    # draw origin position
    cv2.circle(frame, (originX, originY), 5, (0, 0, 255), -1)
    
    # draw the 10.97m reference line
    cv2.circle(frame, lineStartPoint, 5, (255, 0, 0), -1)
    cv2.circle(frame, lineEndPoint, 5, (255, 0, 0), -1)
    cv2.line(frame, lineStartPoint, lineEndPoint, (125, 0, 125), 2)

    # correct the estimated position with the origin centre
    correctedPosX = int(round((pos[0] * pixelsToM))) + originX
    correctedPosY = originY - int(round((pos[1] * pixelsToM)))

    # draw the estimated position
    cv2.circle(frame, (correctedPosX, correctedPosY), 5, (0, 0, 0), -1)
    
    if drawingLine : lineEndPoint = mousePos
    
    # show final drawn frame
    cv2.imshow("Position Solver", frame)
    
    key = cv2.waitKey(20) & 0xFF
    # break if esc key pressed
    if cv2.waitKey(20) == 27:
        break
    # break if close window button is pressed
    if cv2.getWindowProperty('Position Solver', cv2.WND_PROP_VISIBLE) < 1:
        break

cv2.destroyAllWindows()
