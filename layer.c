static char *vertex =
"#version 150 core\n"
"uniform float aspect_ratio;\n"
"uniform vec2 origin;\n"
"uniform vec2 pivot;\n"
"uniform vec3 rotation;\n"
"uniform float depth;\n"
"in vec2 position;\n"
"in vec2 UV;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
	"frag_UV = UV;\n"
	// vtuber parts tend to pivot around a certain spot (e.g. head around neck), this matrix math combines the relevant rotations/transformations to do that
	"float roll_sin = sin(rotation.x);\n"
	"float roll_cos = cos(rotation.x);\n"
	"mat3 roll = mat3(roll_cos, roll_sin, 0, -roll_sin, roll_cos, 0, 0, 0, 1.0);\n"

	"float yaw_sin = sin(rotation.y);\n"
	"float yaw_cos = cos(rotation.y);\n"
	"mat3 yaw = mat3(yaw_cos, 0, -yaw_sin, 0, 1.0, 0, yaw_sin, 0, yaw_cos);\n"

	"vec3 rotated = (roll * yaw * vec3(position + origin - pivot, depth)) + vec3(pivot, 0);\n"

    "gl_Position = vec4(rotated.xy, -1.0, 1.0) * vec4(aspect_ratio, 1.0, 1.0, 1.0);\n"
"}";

static char *fragment =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"outColor = texture(tex, frag_UV);\n"
	"if (outColor.a < 0.5) { discard; }\n"
"}";

static GLuint shader_program;

static float aspect_ratio; // for correcting for screen stretch

typedef struct {

	GLuint vertex_array; // "VAO"
	GLuint texture;
	float depth;

	float x, y;
	float pivot_x, pivot_y;
	float roll, yaw, pitch;

} Layer; // a layer is a mesh + a texture + a layer depth + some translation/rotation information (used for rotation; every vertex on the same layer has the same depth, no depth testing needed, just draw painterly)

// TODO allow for layer translation (for lips, eyelids, etc moving up/down) and scaling (same stuff ig)
// translation is applied before rotation (since rotation is around a pivot instead of your origin), unlike other systems

// returns NULL on error
Layer *create_layer(const float depth, const char *tex_path) {

	// read image with SDL
	SDL_Surface *surface = IMG_Load(tex_path);

	if (surface->format->BytesPerPixel != 4) {
		log_error("IMG_Load loaded with the wrong SDL_PixelFormat and now you have to program conversion :(");
		exit(1);
	}

	// create a mesh that fits the size of the image
	float mesh_w = surface->w / (float) 1000;
	float mesh_h = surface->h / (float) 1000;

	float mesh_data[] = {
		-mesh_w, -mesh_h, 0.0, 1.0,
		 mesh_w,  mesh_h, 1.0, 0.0,
		-mesh_w,  mesh_h, 0.0, 0.0,
		-mesh_w, -mesh_h, 0.0, 1.0,
		 mesh_w,  mesh_h, 1.0, 0.0,
		 mesh_w, -mesh_h, 1.0, 1.0,
	};

	// make vertex array
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// make vertex buffer (stored by vertex_array)
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);									// make it the active buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(mesh_data), mesh_data, GL_STATIC_DRAW);	// copy vertex data into the active buffer

	// link active vertex data and shader attributes
	GLint pos_attrib = glGetAttribLocation(shader_program, "position");
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
	glEnableVertexAttribArray(pos_attrib); // requires a VAO to be bound

	GLint uv_attrib = glGetAttribLocation(shader_program, "UV");
	glVertexAttribPointer(uv_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid *) (sizeof(float) * 2));
	glEnableVertexAttribArray(uv_attrib);

	// debind vertex array
	glBindVertexArray(0);

	// create texture object
	GLuint texture;
	glGenTextures(1, &texture);

	// bind texture (to active texture 2D)
	glBindTexture(GL_TEXTURE_2D, texture);

	// wrap repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// filter linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// write to the OpenGL texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	// create final layer object to return
	Layer *layer = malloc(sizeof(Layer));
	layer->vertex_array = vertex_array;
	layer->texture = texture;
	layer->depth = depth;
	layer->x = 0.0;
	layer->y = 0.0;
	layer->pivot_x = 0.0;
	layer->pivot_y = 0.0;
	layer->roll = 0.0;
	layer->yaw = 0.0;
	layer->pitch = 0.0;

	// free the SDL surface
	SDL_FreeSurface(surface);

	return layer;
}

void draw_layer(const Layer *layer) {

	// bind the layer's vertex mesh and texture
	glBindVertexArray(layer->vertex_array);
	glBindTexture(GL_TEXTURE_2D, layer->texture);

	// load shader uniforms
	glUniform1f(glGetUniformLocation(shader_program, "aspect_ratio"), aspect_ratio);
	glUniform2f(glGetUniformLocation(shader_program, "origin"), layer->x, layer->y);
	glUniform3f(glGetUniformLocation(shader_program, "rotation"), layer->roll, layer->yaw, layer->pitch);
	glUniform2f(glGetUniformLocation(shader_program, "pivot"), layer->pivot_x, layer->pivot_y);
	glUniform1f(glGetUniformLocation(shader_program, "depth"), layer->depth);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void initialize_shader() {

	// create shader program
	shader_program = glCreateProgram();

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, (const char *const *) &vertex, NULL);
	glCompileShader(vertex_shader);
	glAttachShader(shader_program, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, (const char *const *) &fragment, NULL);
	glCompileShader(fragment_shader);
	glAttachShader(shader_program, fragment_shader);

	// apply changes to shader program
	glLinkProgram(shader_program);

	// load the shader program (can do this at initialization since we never need to swap shaders)
	glUseProgram(shader_program);
}

void update_screen_size(int w, int h) {

	aspect_ratio = h / (float) w;
}