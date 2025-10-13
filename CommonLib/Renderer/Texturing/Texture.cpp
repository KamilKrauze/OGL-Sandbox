#include "Renderer/Texturing/Texture.h"
#include "Logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb/stb_image.h>


// void Texture::CreateTextureUnit(int _binding, const char* fp)
// {
//     if (textureObj > 0)
//     {
//         throw TextureError("Texture has already been created! Call Delete() if you want to use the same object with different tex data!");
//     }
//     stbi_set_flip_vertically_on_load(true);
//
//     int width, height, channels;
//     unsigned char *data = stbi_load(fp, &width, &height, &channels, 3);
//     if (data)
//     {
//         LOG_INFO("Loaded file '%s' {%d, %d} & Channels: %d", fp+'\n', width, height, channels);
//         glCreateTextures(GL_TEXTURE_2D, 1, &textureObj);
//
//         glTextureStorage2D(textureObj, 1, GL_RGBA8, width, height);
//         
//         glTextureSubImage2D(textureObj, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
//         glGenerateTextureMipmap(textureObj);
//         
//         glTextureParameteri(textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
//         glTextureParameteri(textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
//         glTextureParameteri(textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTextureParameteri(textureObj, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
//
//         Bind(_binding);
//         stbi_image_free(data);
//     }
//     else
//     {
//         LOG_ERROR("Failed to load texture image");
//         stbi_image_free(data);
//     }
//     Unbind();
// }
//
void Texture::CreateTextureUnit(const char* fp, TextureSpec spec)
{
    if (textureObj > 0)
    {
        throw TextureError("Texture has already been created! Call Delete() if you want to use the same object with different tex data!");
    }
    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    void* rawData = LoadImageFile(fp, spec.type, width, height, channels);

    uint32_t mipLevels = spec.generateMips ? (int)std::floor(std::log2(pow(std::max(width, height), 0.65f))) + 1 : 1;
    if (rawData)
    {
        LOG_INFO("Loaded file '%s' {%d, %d} & Channels: %d", fp+'\n', width, height, channels);
        glCreateTextures(GL_TEXTURE_2D, 1, &textureObj);

        glTextureStorage2D(textureObj, mipLevels, spec.internalFormat, width, height);
        if (spec.type == GL_FLOAT)
        {
            glTextureSubImage2D(textureObj, 0, 0, 0, width, height, spec.format, spec.type, static_cast<float*>(rawData));
        }
        else
        {
            glTextureSubImage2D(textureObj, 0, 0, 0, width, height, spec.format, spec.type, static_cast<unsigned char*>(rawData));
        }
        
        glTextureParameteri(textureObj, GL_TEXTURE_WRAP_S, spec.wrappingMethod);
        glTextureParameteri(textureObj, GL_TEXTURE_WRAP_T, spec.wrappingMethod);
        
        // Fix filters
        glTextureParameteri(textureObj, GL_TEXTURE_MIN_FILTER,
            spec.generateMips ? spec.minificationFilter : GL_LINEAR);
        glTextureParameteri(textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // always linear or nearest

        if (spec.generateMips)
            glGenerateTextureMipmap(textureObj);

        stbi_image_free(rawData);
    }
    else
    {
        LOG_ERROR("Failed to load texture image");
        stbi_image_free(rawData);
    }
}

void Texture::Bind(int _binding)
{
    // if (binding >= 0)
    // {
    //     LOG_ERROR("This texture unit (binding id: %d) is already bound! Call Unbind() before binding it again!", binding);
    //     throw TextureError("This texture unit is already bound! Call Unbind() before binding it again!");
    // }
    glBindTextureUnit(_binding, textureObj);
    this->binding = _binding;
}

void Texture::Unbind()
{
    if (binding == -1)
    {
        return;
    }
    glBindTextureUnit(binding,0);
    binding = -1;
}

void Texture::Delete()
{
    Unbind();
    glDeleteTextures(1, &textureObj);
    textureObj = 0;
}

void* Texture::LoadImageFile(const char* fp, GLenum datatype, int& width, int& height, int& nmrChannels)
{
    void *data = nullptr;
    switch (datatype)
    {
        case GL_FLOAT:
            data = stbi_loadf(fp, &width, &height, &nmrChannels, 3);
            break;
        case GL_UNSIGNED_BYTE:
            data = stbi_load(fp, &width, &height, &nmrChannels, 3);
            break;
        default:
            return nullptr;
    }
    return data;
}
