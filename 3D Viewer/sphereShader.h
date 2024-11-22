#ifndef SPHERE_SHADER_H
#define SPHERE_SHADER_H

const char* vertex_shader = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 Color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"    Color = aColor;\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0)\n;"
"}\0";

const char* fragment_shader = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Color;\n"
"void main()\n"
"{"
"    FragColor = vec4(Color, 1.0);\n"
"}\0";

#endif