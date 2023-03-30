SimpleMiner

Available commands:

P, START button = begin game from Attract screen
ESC, BACK button = quit application from Attract screen, or
	pause during game play, or exit game play to attract screen if paused
P = toggle pause during game play
T = update 10x more slowly
Y = update 4x faster and daytime is 50x faster
O = single step one frame and pause
~ = toggle display of DevConsole
F1 = enable developer mode with debug information (location and town locations)
F2 = change camera mode
F3 = change physics mode
F5 = decrease sound volume
F6 = increase sound volume
F8 = reset game
F9 = Test game at splash screen
WASD = move forward backward and side to side
QE = move up and down when flying
SPACE = jump
left mouse = delete block
right mouse = place block
1-9 select block to place

Deep Learning

Using Perlin noise to site towns was not too difficult.  Constructing the towns was the real challenge.
Even though I already had the code to draw trees that cross chunk boundaries, I decided to just have
one building per chunk and not cross boundaries.  This simplified the town layout problem, although
really it would not have been hard to handle buildings crossing chunk boundaries.  What I focused on
instead was determining that buildings should not be placed on slopes and in the water.  I also
devoted significant effort to the process of creating, importing, and customizing building templates.

I tried making a swamp biome and more tree types using the 3D sprites but I was disatisfied with the
results and decided not to do it.  If you look hard, you can actually find some weeping willow trees.