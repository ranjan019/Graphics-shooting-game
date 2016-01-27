all: gameexecutable

gameexecutable: game.cpp glad.c
	g++ -o gameexecutable game.cpp glad.c -lGL -lglfw -ldl

clean:
	rm gameexecutable
