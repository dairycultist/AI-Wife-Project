app: main.c mesh.c
	@gcc -o app main.c  -lGLEW -framework OpenGL $(shell sdl2-config --libs) $(shell sdl2-config --cflags)

# server_app next

run: app
	@./app

clean:
	rm app