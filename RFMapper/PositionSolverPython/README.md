# Position Solver
This program finds the relative position of an RF receiver in a room with 4 transmitters of unique frequencies.  
It uses the recorded power levels from each transmitter and applies a log equation (values experimentally derived) to find the approximate
distance of the transmitter.

![](PositionSolver_Video.gif)

To find our relative position, we must solve the equation  
$(x-x_0)^2 + (y-y_0)^2 = d^2$  
for at least 3 transmitters. To convert this to a linear system of equations we use  
$r = (A^T A)^{-1} A^T b$  
where
```math
r = \begin{bmatrix} x \\ y \end{bmatrix}
```

Once the position is solved relative to some (0,0) the user can left click anywhere on the screen to set the origin.  
Then, a scale must be provided (pixels -> metres) so the user right clicks and drags to define a line of length 10.97m.

## Improvements
- In-app receiver placement and power input
- Automatic power value fetching from RFHeatmap app
- Saving/loading data
- Custom scale (not just 10.97m)
