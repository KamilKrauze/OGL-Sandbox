#pragma once

#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3native.h>

enum WrappingMode : uint32_t
{
    Clamp = GL_CLAMP_TO_EDGE,
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
};

enum FilterMode : uint32_t
{
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    Bilinear = GL_LINEAR_MIPMAP_NEAREST,
    Trilinear = GL_LINEAR_MIPMAP_LINEAR,
    NearestMipAndTexel = GL_NEAREST_MIPMAP_NEAREST,
    NearestMipBlendTexel = GL_NEAREST_MIPMAP_LINEAR,
    
};

struct TextureSpec
{
    TextureSpec() = default;
    TextureSpec(WrappingMode wrapMode, FilterMode minFilter,
        FilterMode magFilter, uint32_t internalFmt,
        uint32_t fileFmt, uint32_t file_data_type, bool useMips) :
    wrappingMethod(wrapMode), minificationFilter(minFilter),
    magnificationFilter(magFilter), internalFormat(internalFmt),
    format(fileFmt), type(file_data_type), generateMips(useMips) {}
    
    TextureSpec(const TextureSpec&) = default;
    TextureSpec(TextureSpec&&) = default;
    ~TextureSpec() = default;

public:
    WrappingMode wrappingMethod = Repeat;
    FilterMode minificationFilter = Linear;
    FilterMode magnificationFilter = Linear;
    uint32_t internalFormat = GL_RGBA8;
    uint32_t format = GL_RGB;
    uint32_t type = GL_UNSIGNED_BYTE;
    bool generateMips = true;
};

class Texture
{
public:
    Texture() = default;
    ~Texture() = default;
public:
    // void CreateTextureUnit(int _binding, const char* fp);
    void CreateTextureUnit(const char* fp, TextureSpec spec = {});
    void Bind(int _binding);
    void Unbind();
    void Delete();
private:
    static void* LoadImageFile(const char* fp, GLenum datatype, int& width, int& height, int& nmrChannels);

private:
    GLuint textureObj = 0;
    int binding = -1;
};


class TextureError : public std::runtime_error
{
public:
    explicit TextureError(const char* msg) : std::runtime_error(msg) {}
    ~TextureError() = default;
};