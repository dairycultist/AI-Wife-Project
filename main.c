#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

void log_error(const char *msg) {
	
	if (strlen(SDL_GetError()) == 0) {
		fprintf(stderr, "\n%s: <No error given>\n\n", msg);
	} else {
		fprintf(stderr, "\n%s: %s\n\n", msg, SDL_GetError());
	}
}

#include "layer.c"

int main() {

	printf("Starting Waifu\n");

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		log_error("Could not initialize SDL");
		return 1;
	}

	// init OpenGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// create the window
	SDL_Window *window = SDL_CreateWindow("Waifu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!window) {
        log_error("Could not create window");
		return 1;
    }

	SDL_GLContext context = SDL_GL_CreateContext(window);

	glewExperimental = GL_TRUE;
	glewInit();

	// initialize 3D
	initialize_shader();
	update_screen_size(800, 400);
	
	// let programmer initialize stuff
	Layer *layers[] = {
		create_layer(-0.17, "tex_backerhair.png"),
		create_layer(0.0, "tex_backhair.png"),
		create_layer(0.1, "tex_sidefrills.png"),
		create_layer(0.12, "tex_ears.png"),
		create_layer(0.15, "tex_face.png"),
		create_layer(0.22, "tex_eyeleft.png"),
		create_layer(0.22, "tex_eyeright.png"),
		create_layer(0.25, "tex_nose.png"),
		create_layer(0.21, "tex_frill_left.png"),
		create_layer(0.21, "tex_frill_right.png"),
		create_layer(0.21, "tex_baseofbangs.png"),
		create_layer(0.27, "tex_bangs.png"),
	};

	for (int i = 0; i < sizeof(layers) / sizeof(Layer *); i++) {
		layers[i]->pivot_y = -0.3;
	}

	layers[5]->origin_x = -0.08; // eyes
	layers[5]->origin_y = -0.04;
	layers[5]->origin_yaw = -0.3;

	layers[6]->origin_x = 0.08;
	layers[6]->origin_y = -0.04;
	layers[6]->origin_yaw = 0.3;

	layers[7]->origin_y = -0.13; // nose

	layers[8]->origin_x = -0.25; // frills
	layers[8]->origin_yaw = -0.2;

	layers[9]->origin_x = 0.25;
	layers[9]->origin_yaw = 0.2;

	int screen_w = 800, screen_h = 400;
	float mouse_u = 0.0, mouse_v = 0.0;
	float wobble_pos = 0.0;
	float wobble_vel = 0.01;

	// process events until window is closed
	SDL_Event event;
	int running = TRUE;

	while (running) {

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				
				running = FALSE;

			} else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {

				glViewport(0, 0, event.window.data1, event.window.data2);
				update_screen_size(event.window.data1, event.window.data2);
				screen_w = event.window.data1;
				screen_h = event.window.data2;

			} else if (event.type == SDL_MOUSEMOTION) {

				float prev_mouse_u = mouse_u;

				mouse_u = event.motion.x * 2.0 / screen_w - 1.0;
				mouse_v = event.motion.y * 2.0 / screen_h - 1.0;

				wobble_pos += (prev_mouse_u - mouse_u) * 0.15;
			}
		}

		// draw layers
		for (int i = 0; i < sizeof(layers) / sizeof(Layer *); i++) {

			layers[i]->pivot_pitch = mouse_v * 0.6;
			layers[i]->pivot_yaw = mouse_u * 0.6;

			if (i == 8 || i == 9 || i == 11) {
				layers[i]->origin_roll = wobble_pos;
			}

			draw_layer(layers[i]);
		}
		
		wobble_pos += wobble_vel;
		wobble_vel -= wobble_pos * 0.01;
		wobble_vel *= 0.96;

		// swap buffers
		SDL_GL_SwapWindow(window);
		SDL_Delay(1000 / 60);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	// free everything
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(context);
	SDL_Quit();

	return 0;
}