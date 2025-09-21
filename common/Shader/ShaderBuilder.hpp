#ifndef SHADERBUILDER_HPP
#define SHADERBUILDER_HPP

#include <string>

#include <glad/glad.h>

namespace ShaderBuilder {

	/* Load vertex and fragment shader and return the compiled program */
	GLuint Load(const char* vertex_path, const char* fragment_path);

	/* Build shaders from strings containing shader source code */
	GLuint Build(GLenum eShaderType, const std::string& shaderText);

	/* Load vertex and fragment shader and return the compiled program */
	GLuint BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr);
};

#endif // !SHADERBUILDER_HPP


