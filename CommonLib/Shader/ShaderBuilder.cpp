#include "ShaderBuilder.hpp"

#include <fstream>
#include <iostream>
#include <vector>
#include <exception>
#include <filesystem>

#include "Logger.hpp"

// Read text in file helper function
static std::string readFile(const char* filePath)
{
	auto size = std::filesystem::file_size(filePath);
	std::string content(size, '\0');
	std::ifstream fileStream(filePath, std::ios::in);
	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}
	fileStream.read(&content[0], size);
	fileStream.close();
	return content;
}

GLuint ShaderBuilder::Load(const char* vertex_path, const char* fragment_path)
{
	GLuint vertShader, fragShader;

	// Read shaders
	std::string vertShaderStr = readFile(vertex_path);
	std::string fragShaderStr = readFile(fragment_path);

	GLint result = GL_FALSE;
	int logLength;

	vertShader = Build(GL_VERTEX_SHADER, vertShaderStr);
	fragShader = Build(GL_FRAGMENT_SHADER, fragShaderStr);

	LOG_INFO("Linking program: [%s <>---<> %s]", vertex_path+'\n', fragment_path+'\n');
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> programError((logLength > 1) ? logLength : 1);
	glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
	if (!&programError[0]) {
		LOG_ERROR(&programError[0]);
		// std::cout << &programError[0] << std::endl;
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}

GLuint ShaderBuilder::Build(GLenum eShaderType, const std::string& shaderText)
{
	GLuint shader = glCreateShader(eShaderType);
	const char* strFileData = shaderText.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		// Output the compile errors

		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char* strShaderType = NULL;
		switch (eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		LOG_ERROR("[Compiler/%s] - %s", strShaderType, strInfoLog);
		
		delete[] strInfoLog;

		throw std::runtime_error("Shader compile exception");
	}

	return shader;
}

GLuint ShaderBuilder::BuildShaderProgram(std::string vertShaderStr, std::string fragShaderStr)
{
	GLuint vertShader, fragShader;
	GLint result = GL_FALSE;

	try
	{
		vertShader = Build(GL_VERTEX_SHADER, vertShaderStr);
		fragShader = Build(GL_FRAGMENT_SHADER, fragShaderStr);
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		throw std::exception("BuildShaderProgram() Build shader failure. Abandoning");
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{

		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		std::cerr << "Linker error: " << strInfoLog << std::endl;

		delete[] strInfoLog;
		throw std::runtime_error("Shader could not be linked.");
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}