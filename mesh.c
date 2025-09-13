// all 3D objects use the same hardcoded shader for simplicity
static char *vertex =
"#version 150 core\n"
// "uniform mat4 view_matrix;\n" // for correcting for screen stretch
// "uniform mat4 move_matrix;\n" // for the various rotations/transformations associated with looking in different directions and stuff
// "uniform float depth;\n" // each mesh is placed on a different "layer" which rotates based on their layer depth
"in vec2 position;\n"
"in vec2 UV;\n"
"out vec2 frag_UV;\n"
"void main() {\n"
    "gl_Position = vec4(position.xy, -1.0, 1.0);\n" // get final position
    "frag_UV = UV;\n" // pass along UV
"}";

static char *fragment =
"#version 150 core\n"
"uniform sampler2D tex;\n"
"in vec2 frag_UV;\n"
"out vec4 outColor;\n"
"void main() {\n"
	"outColor = texture(tex, frag_UV);\n"
"}";

static GLuint shader_program;

typedef struct {

	GLuint vertex_array; // "VAO"
	uint vertex_count;
	GLuint texture;

} Mesh;

// returns NULL on error
Mesh *create_mesh(const unsigned char *mesh_data, const int mesh_bytecount, const int mesh_vertcount, const unsigned char *tex, const int tex_width, const int tex_height) {

	// make vertex array
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	// make vertex buffer (stored by vertex_array)
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);								// make it the active buffer
	glBufferData(GL_ARRAY_BUFFER, mesh_bytecount, mesh_data, GL_STATIC_DRAW);	// copy vertex data into the active buffer

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

	// write texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex);

	// create final mesh object to return
	Mesh *mesh = malloc(sizeof(Mesh));
	mesh->vertex_array = vertex_array;
	mesh->vertex_count = mesh_vertcount;
	mesh->texture = texture;

	return mesh;
}

void draw_mesh(const Mesh *mesh) {

	// bind the mesh's vertex mesh and texture
	glBindVertexArray(mesh->vertex_array);
	glBindTexture(GL_TEXTURE_2D, mesh->texture);

	// load the shader program and the uniforms we just calculated
	glUseProgram(shader_program);
	// glUniformMatrix4fv(glGetUniformLocation(shader_program, "position_matrix"), 1, GL_FALSE, &position_matrix[0][0]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
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

	// apply changes to shader program (not gonna call "glUseProgram" yet bc not drawing)
	glLinkProgram(shader_program);
}