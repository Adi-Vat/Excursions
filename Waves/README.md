# Waves
A little 1D wave simulator in C#.
It implements the general wave equation.  

$\frac{\partial^2 u}{\partial^2 t} = c^2 \nabla^2 u$  

[![](WavesThumbnail.png)](https://youtu.be/JeCnEvR4Vik)

I approximate $\frac{d^2 f(x)}{dx^2}$ with $\frac{f(x+2\Delta x) - 2f(x + \Delta x) + f(x)}{\Delta x^2}$  

Each point in the field has a displacement, *u(x, t)*. *x* is an index for a 1D list in this case and sometimes referred to as *i*.  
The LHS of the wave equation is the acceleration of a point at index *i* at time *t*.  
The RHS is the speed of propogation squared times the curvature of the point at *i* at time *t*.  

If a point *p* sticks out more compared to the points around it, its (absolute) curvature will be larger.
The curvature is negative if the point is above its neighbours, and positive if it's below them.  
This provides a **restoring force** to move the point back towards equilibrium with its neighbours.  
(This force is applied as an acceleration as the points are massless.)  

To get $u(x\pm \Delta x$, t) I get the point to the left and right of the current point.  
To get $u(x, t - \Delta t$) I store the previous frame's field data. 

## To Do
- Generalize to 2D
- In 2D, add vectors
- Add relationship between perpendicular vector fields
- Implement curl
