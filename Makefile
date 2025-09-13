app: main.c layer.c
	@gcc -o app main.c  -lGLEW -framework OpenGL -lSDL2_image $(shell sdl2-config --libs) $(shell sdl2-config --cflags)

run: app
	@./app

clean:
	rm app