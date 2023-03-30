Math Visual Tests Application
===============================
Provides a test bed for various visual math functions, ray casting, and
Nearest Point (2D and 3D) tests against shapes 	
* Disc2
* AABB2
* Capsule2
* OBB2
* LineSegment2
* Infinite line (LineSegment2)
* Sphere
* Cylinder
* AABB3

Nearest Point (2D) shows nearest point to a test point, with lines drawn from the point
to the nearest points.

Ray cast vs. Discs (2D) tests against line segments of various lengths

Ray cast vs. Discs (2D) tests against discs of various diameters

Ray cast vs. AABB2(2D) tests against axis-aligned boxes of various dimensions

Ray cast vs. OBB2(2D) tests against oriented bounding boxes of various dimensions

Billiards (2D) creates walls, bumpers and balls with varying elasticity (which is actually 
the coefficient of restitution).  THe test simulates velocity damping after collision of
the moving balls with other objects.

Pachenko (2D) creates walls, bumpers and balls with varying elasticity (which is actually 
the coefficient of restitution). THe test simulates velocity damping after collision of
the moving balls with other objects and the effect of downward gravitational acceleration.
The bottom wall can optionally be toggled off, and the top wall is removed, so that an 
infinite fall will occur through the space.

Test Shapes (3D) creates textured and wire frame versions of spheres, boxes and cylinders.
A moving camera allows viewing the objects from different vantage points.  Ray casts,
overlapping and nearest point tests on the various shapes.

Easing, Curves and Splines demonstrates methods of curve generation using parametric and
Bezier methods.

General Commands In All Test Modes:
===================================
ESC : exit to attract screen
F6 : next mode
F7 : previous mode
F8 : regenerate test data
T slows clock to 10% rate when held

Commands by Test Mode:
===============================
Nearest Point (2D)

WASD moves in traditional directions when held continuously
Left joystick also moves the dot
ESC ends program

Ray cast vs. Line Segment (2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)

Ray cast vs. Discs (2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)

Ray cast vs. AABB2 (2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)

Ray cast vs. OBB2 (2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)

Billiards (2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)
Space fires a ball with velocity in the ray direction and proportional to the ray length.

Pachenko(2D)

WASD moves moves the ray start in directions when held continuously
IJKL moves moves the ray end in directions when held continuously
Arrow keys move the entire ray in directions when held continuously
Left mouse buttion snaps the ray start to the click position (and moves it when held continuously)
Right mouse buttion snaps the ray end to the click position (and moves it when held continuously)
Space creates a ball with velocity in the ray direction and proportional to the ray length.
Holding N creates a ball each frame with velocity in the ray direction and proportional to the ray length.
F3/F4 decreases/increases the physics time step

Test Shapes (3D)

WS moves the camera along the X axis.
AD moves the camera along the Y axis.
QE moves the camera along the Z axis.
Mouse L/R movement pans in the XY plane (yaw)
Mouse U/D movement controls pitch
H snaps the camera to the origin position
F4 toggles the 6 DOF camera for debugging
SPACE locks the ray casts and nearest points to view remotely
Lmouse grabs and ungrabs selected object to move with the camera

Easing, Curves and Splines (2D)

WE cycles through the various easing functions graphed in the upper left pane
MN increases and decreases the number of segments for generating curves and splines
C shows a tinted background of the viewports
T slows the progression of the dots along the curves
F8 randomizes the curves displayed