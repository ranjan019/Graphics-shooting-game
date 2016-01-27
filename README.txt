********** 
ABOUT THE GAME: 
Shoot the cannon ball to destroy the building avoiding the obstacles(green fans). 
********** 
--------------------------------------------- 
Install GLFW, GLAD, GLM. 
Run the game by using these commands in the directory submitted. 

1) 
g++ -o gameexecutable game.cpp glad.c -lGL -lglfw -ldl 
./gameexecutable 

OR 

2)
make gameexecutable
./gameexecutable

--------------------------------------------- 

Keyboard Controls: 

1) Rotate Cannon(up/down) ----- UP/DOWN arrow key 
2) Shoot Ball             ----- SPACEBAR 
#STRENGTH of shoot depends upon duration of pressing of space bar#. 
3) Reload Cannon          ----- r 
4) Zoom OUT		  ----- o 
5) Zoom IN                ----- p 

--------------------------------------------- 

Mouse Controls: 

1) Shoot Ball            ----- LEFT MOUSE BUTTON 
#STRENGTH of shoot depends upon duration of pressing of LEFT MOUSE BUTTON# 
2) Reload Cannon         ----- RIGHT MOUSE BUTTON 
3) Zoom OUT              ----- MOUSE SCROLL UP 
4) Zoom IN               ----- MOUSE SCROLL DOWN 
Use Keyboard up/down arrow keys to change cannon angle.




---------------------------------------------------------------------



----------------------------------------------------------------
DEPENDENCIES
----------------------------------------------------------------
Linux/Windows/ Mac OSX - Dependencies: (Recommended)
 GLFW
 GLAD
 GLM

Linux - Dependencies: (alternative)
 FreeGLUT
 GLEW
 GLM

----------------------------------------------------------------
INSTALLATION
----------------------------------------------------------------
GLFW:
 - Install CMake
 - Obtain & Extract the GLFW source code from
   https://github.com/glfw/glfw/archive/master.zip
 - Compile with below commands
   $ cd glfw-master
   $ mkdir build
   $ cd build
   $ cmake -DBUILD_SHARED_LIBS=ON ..
   $ make && sudo make install

GLAD:
 - Go to http://glad.dav1d.de
 - Language: C/C++
   Specification: OpenGL
   gl: Version 4.5
   gles1: Version 1.0
   gles2: Version 3.2
   Profile: Core
   Select 'Add All' under extensions and click Generate.
 - Download the zip file generated.
 - Copy contents of include/ folder in the downloaded directory 
   to /usr/local/include/
 - src/glad.c should be always compiled along with your OpenGL 
   code

GLM:
 - Download the zip file from 
   https://github.com/g-truc/glm/releases/tag/0.9.7.2
 - Unzip it and copy the folder glm/glm/ to /usr/local/include

Ubuntu users can also install these libraries using apt-get.
It is recommended to use GLFW+GLAD+GLM on all OSes.


FreeGLUT+GLEW+GLM can be used only on linux but not recommended.
Use apt-get to install FreeGLUT, libglew, glm on Linux.

----------------------------------------------------------------
DEMO CONTROLS
----------------------------------------------------------------
Keyboard:
 c - toggle cube rotation
 p - toggle pyramid rotation
 q - quit

Mouse:
 Left click - Change Pyramid rotation direction
 Right click - Change the vector about which Cube rotates
