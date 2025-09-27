#ifndef VERTEX_DATA_H
#define VERTEX_DATA_H

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct VertexData
{
    VertexData() = default;
    ~VertexData() = default;
    
    std::vector<glm::vec3>  vertices;
    std::vector<GLuint>     indices;
    std::vector<glm::vec4>  colours;
    std::vector<glm::vec2>  texCoords;
    std::vector<glm::vec2>  lightMapUV;
    std::vector<glm::vec3>  normals;
    std::vector<glm::vec3>  tangents;
    std::vector<glm::vec3>  binormals;
};


#endif