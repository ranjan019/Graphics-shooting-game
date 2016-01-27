# Graphics-shooting-game
Graphics, opengl, glfw, shooting game, projectile, collisions.


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


