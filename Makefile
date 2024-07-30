build: src/main.c src/game.c
	clang -I./include/ src/*.c -o raycasting -lSDL2 -lSDL2_image -lm
