#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Shader
{
public:
    Shader(std::string vertex_path, std::string fragment_path, std::string geometry_path) {
        // Compile
        
        unsigned int program = glCreateProgram();
        unsigned int vertex_shader = compile(GL_VERTEX_SHADER, vertex_path);
        unsigned int fragment_shader = compile(GL_FRAGMENT_SHADER, fragment_path);
        unsigned int geometry_shader = compile(GL_GEOMETRY_SHADER, geometry_path);
        // Link
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glAttachShader(program, geometry_shader);
        glLinkProgram(program);
        // Check for errors
        int result;
        char error_log[512];
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetProgramInfoLog(program, 512, nullptr, error_log);
            std::cout << "FAILED TO LINK SHADERS: " << error_log << std::endl;
        }
        // Validate
        glValidateProgram(program);
        m_id = program;
        // Free compiled shaders
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteShader(geometry_shader);        
    }

    // Bind shader program to opengl
    void use(void) {
        
        glUseProgram(m_id);    
    }
    // Uniform setting functions
    void setVec3(const std::string& var_name, glm::vec3 vector) {
        
        int location = glGetUniformLocation(m_id, var_name.c_str());
        glUniform3f(location, vector.x, vector.y, vector.z);
        
    }
    void setMat4(const std::string& var_name, glm::mat4 matrix) {
        
        int location = glGetUniformLocation(m_id, var_name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
        
    }
    void setFloat(const std::string& var_name, float value) {
        int location = glGetUniformLocation(m_id, var_name.c_str());
        glUniform1f(location, value);    
    }

    void reloadValues(Menu* menu) {
        setVec3("material.ambient", menu->ambientMaterialColor);
        setVec3("material.diffuse", menu->diffuseMaterialColor);
        setVec3("material.specular", menu->specularMaterialColor);
        setFloat("material.shininess", menu->shininess);
        setVec3("light.ambient", menu->ambientLightingColor);
        setVec3("light.diffuse", menu->diffuseLightingColor);
        setVec3("light.specular", menu->specularLightingColor);
    }
    unsigned int get_id(void) { return m_id; }

private:
    unsigned int m_id;

    unsigned int compile(unsigned int type, const std::string& path) {
        // Read shader text into c string
        std::ifstream shader_source(path);
        std::stringstream shader_buf;
        shader_buf << shader_source.rdbuf();
        std::string shader_str = shader_buf.str();
        const char* shader_cstr = shader_str.c_str();
       // Compile shader
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &shader_cstr, nullptr);
        glCompileShader(id);
        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            char error_log[512];
            glGetShaderInfoLog(id, 512, nullptr, error_log);
            glDeleteShader(id);
            std::cout << "FAILED TO COMPILE SHADER: " << error_log << std::endl;
            return 0;
        }    
        // Check for errors
       return id;
    }


};
