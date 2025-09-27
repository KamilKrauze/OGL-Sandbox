#include "Loader/MeshLoaders.hpp"

// External libraries
#include <glad/glad.h>
#include <glm/glm.hpp>

// Standard libraries
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

constexpr char VERT[] = "v";	// Vertex position.
constexpr char NORMAL[] = "vn"; // Vertex normals.
constexpr char UVS[] = "vt";	// Vertex texture coordinates.
constexpr char INDEX[] = "f";	// Index data.

// Object indicies delimeter
#define INDEX_DEL '/'

struct VertexKey {
    GLuint pos, uv, norm;
    bool operator<(const VertexKey& other) const {
        return std::tie(pos, uv, norm) < std::tie(other.pos, other.uv, other.norm);
    }
};

namespace MeshLoaders::Static
{
    
    void ImportOBJ(VertexData& vertInfo, const std::string_view filepath)
    {
        std::ifstream file(filepath.data(), std::ios::in);
        if (file.fail()) {
            std::cout << "ERROR: Filepath is invalid, may not exist or may be incorrect at... " << filepath << std::endl;
            return;
        }

        std::vector<glm::vec3> tmp_positions, tmp_normals;
        std::vector<glm::vec2> tmp_uvs;

        std::map<VertexKey, GLuint> uniqueVertexMap;

        GLdouble x, y, z;
        for (std::string line; std::getline(file, line);)
        {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type == VERT) {
                ss >> x >> y >> z;
                tmp_positions.push_back({ x, y, z });
            } 
            else if (type == NORMAL) {
                ss >> x >> y >> z;
                tmp_normals.push_back({ x, y, z });
            } 
            else if (type == UVS) {
                ss >> x >> y;
                tmp_uvs.push_back({ x, y }); // flip Y if needed
            } 
            else if (type == INDEX) {
                std::string token;
                while (ss >> token) {
                    std::stringstream ss_data(token);
                    std::string data;
                    GLuint indices[3] = { 0, 0, 0 };
                    size_t i = 0;

                    while (std::getline(ss_data, data, INDEX_DEL)) {
                        if (!data.empty()) {
                            indices[i] = std::stoi(data) - 1; // OBJ indices start at 1
                        }
                        ++i;
                    }

                    VertexKey key{ indices[0], indices[1], indices[2] };

                    auto it = uniqueVertexMap.find(key);
                    if (it != uniqueVertexMap.end()) {
                        vertInfo.indices.push_back(it->second);
                    } else {
                        GLuint newIndex = static_cast<GLuint>(vertInfo.vertices.size());
                        vertInfo.vertices.push_back(tmp_positions[indices[0]]);
                        if (!tmp_uvs.empty()) vertInfo.texCoords.push_back(tmp_uvs[indices[1]]);
                        if (!tmp_normals.empty()) vertInfo.normals.push_back(tmp_normals[indices[2]]);
                        vertInfo.indices.push_back(newIndex);
                        uniqueVertexMap[key] = newIndex;
                    }
                }
            }
        }

        // Assign default color
        vertInfo.colours.assign(vertInfo.vertices.size(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        // Clear temp data
        tmp_positions.clear();
        tmp_normals.clear();
        tmp_uvs.clear();
    }
}
