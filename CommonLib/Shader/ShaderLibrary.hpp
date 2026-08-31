#ifndef SHADERBUILDER_HPP
#define SHADERBUILDER_HPP

#include <string>
#include <vector>

#include <glad/glad.h>

#include "ShaderLibrary.hpp"
#include "Renderer/Texturing/Texture.h"

namespace ShaderLibrary {

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

enum class ShaderType : uint8_t
{
	None = 0,
	Vertex = 1,
	Fragment = 2,
	Geometry = 4,
	Tessellation = 8,
	Compute = 16,
};

class Shader
{
public:
	Shader() = default;
	~Shader() = default;

public:
	void Use() const;

private:
	GLuint programID;
	
public:
	Shader& operator=(const uint32_t& program)
	{
		programID = program;
		return *this;	
	}
};

class ShaderBuilder
{
public:
	ShaderBuilder();
	~ShaderBuilder();

private:
	struct ShaderModule
	{
		uint32_t shaderID = 0;
		ShaderType shaderType = ShaderType::None;
		std::string_view filepath;
	};
	
public:
	const ShaderModule& Load(std::string_view filepath, ShaderType shaderType);
	uint32_t Compile() const;

private:
	static std::string readShaderFile(const char* filePath);
	
private:
	ShaderModule modules[4] = {};
	uint8_t shader_flags = 0x00000;
};

#endif // !SHADERBUILDER_HPP


