#ifndef SHADERBUILDER_HPP
#define SHADERBUILDER_HPP

#include <string>

#include <glad/glad.h>

namespace ShaderBuilder {

	/* Load vertex and fragment shader and return the compiled program */
	GLuint Load(const char* vertex_path, const char* fragment_path);

	/* Build shaders from strings containing shader source code */
	GLuint Build(GLenum eShaderType, const std::string& shaderText);

	/**
	 * Builds a compute shader
	 * @param comp_shader_fp Shader program filepath.
	 * @return Shader program reference.
	 */
	GLuint BuildCompute(const char *comp_shader_fp);

	/* Load vertex and fragment shader and return the compiled program */
	GLuint BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr);
};

#endif // !SHADERBUILDER_HPP


