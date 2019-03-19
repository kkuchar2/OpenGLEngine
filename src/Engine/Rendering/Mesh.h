#ifndef OPENGL_MESH_H
#define OPENGL_MESH_H

#include <cmath>
#include <memory>
#include <functional>
#include <map>
#include <thread>
#include <limits>

#include "Shading/Shader.h"
#include "../Utils/Logging/Logger.h"
#include "../Utils/OBJ/tiny_obj_loader.h"
#include "../Utils/TextureLoader.h"
#include "../Window/Input/MessageListener.h"

std::ostream& operator<<(std::ostream & out, const glm::vec3 & v)
{
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation =  glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

struct TriangleInfo {
    float angle;
    glm::vec3 normal;
};

struct BoundingBox {
    glm::vec3 size;
    glm::vec3 center;
    glm::vec3 rotation;
};

typedef std::shared_ptr<Shader>& ShaderPtrRef;
typedef std::function<void(ShaderPtrRef)> ShaderFunc;

class Mesh : public MessageListener {
    public:

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::vector<float> uvs;
        std::vector<float> normals;

        GLenum mode = GL_TRIANGLES;

        bool disableNormals = false;
        bool prepared = false;
        bool drawWireframe = false;

        GLuint textureId = 0;

        std::shared_ptr<Shader> shader;

        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint uvbo = 0;
        GLuint nbo = 0;
        GLuint ibo = 0;

        float * pVertexPosBufferData;

        Transform transform;

        ShaderFunc shaderInit = [](std::shared_ptr<Shader> & shader) { };

        explicit Mesh(const char * path) : MessageListener() {
            loadMesh(path);
        }

        Mesh() : MessageListener() {

        }

        void prepare() {
            if (prepared) {
                return;
            }

            if (!disableNormals) {
                calculateNormals();
            }

            std::stringstream ss;

            ss << "Preparing: [ ";
            ss << "Vertices: " << vertices.size() << ", ";
            ss << "Indices: " << indices.size() << ", ";
            ss << "UV's: " << uvs.size() << ", ";
            ss << "Normals: " << normals.size() << " ]";

            Logger::Log(INFO, this, ss.str());

            CreateVertexAttributeObject();
            CreateIndexBuffer();
            CreateVertexBuffer();

            if (!uvs.empty()) {
                CreateUVBuffer();
            }

            if (!normals.empty()) {
                CreateNormalsBuffer();
            }

            UnbindVertexAtrributeObject();

            prepared = true;
        }

        void CreateVertexAttributeObject() {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }

        void CreateIndexBuffer() {
            glGenBuffers(1, &ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        void CreateVertexBuffer() {
            GLbitfield mapFlags =
                    GL_MAP_WRITE_BIT |
                    GL_MAP_PERSISTENT_BIT |
                    GL_MAP_COHERENT_BIT;
            GLbitfield createFlags = mapFlags | GL_DYNAMIC_STORAGE_BIT;

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferStorage(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), nullptr,  createFlags);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

            pVertexPosBufferData = (float *) glMapBufferRange(GL_ARRAY_BUFFER, 0,  vertices.size() * sizeof(float), mapFlags);

            copyVertifcesToBuffer();
        }

        void CreateUVBuffer() {
            glGenBuffers(1, &uvbo);
            glBindBuffer(GL_ARRAY_BUFFER, uvbo);
            glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        void CreateNormalsBuffer() {
            glGenBuffers(1, &nbo);
            glBindBuffer(GL_ARRAY_BUFFER, nbo);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        void UnbindVertexAtrributeObject() {
            glBindVertexArray(0);
        }

        void copyVertifcesToBuffer() {
            for (int i = 0; i < vertices.size(); i+=3) {
                pVertexPosBufferData[i] = vertices[i];
                pVertexPosBufferData[i + 1] = vertices[i + 1];
                pVertexPosBufferData[i + 2] = vertices[i + 2];
            }
        }

        void UpdateVertexBuffer() {
            prepared = false;
            copyVertifcesToBuffer();
            prepared = true;
        }

        virtual void Render() {

            Render(mode, static_cast<int>(indices.size()));
        }

        void Render(GLenum mode, int count) {
            glBindVertexArray(vao);
            glPolygonMode(GL_FRONT_AND_BACK, drawWireframe ? GL_LINE : GL_FILL);
            glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
        }

        void loadTexture(const char * path) {
            textureId = TextureLoader::load(path);
        }

        BoundingBox calculateBoundingBox() {
            float minFloat = std::numeric_limits<float>::min();
            float maxFloat = std::numeric_limits<float>::max();

            float minX = maxFloat;
            float minY = maxFloat;
            float minZ = maxFloat;

            float maxX = minFloat;
            float maxY = minFloat;
            float maxZ = minFloat;

            for (unsigned int vertexId = 0; vertexId < vertices.size() / 3; vertexId++) {
                glm::vec3 coords =  glm::vec3(vertices[vertexId * 3], vertices[vertexId * 3 + 1], vertices[vertexId * 3 + 2]);

                if (coords.x > maxX) {
                    maxX = coords.x;
                }

                if (coords.y > maxY) {
                    maxY = coords.y;
                }

                if (coords.z > maxZ) {
                    maxZ = coords.z;
                }

                if (coords.x < minX) {
                    minX = coords.x;
                }

                if (coords.y < minY) {
                    minY = coords.y;
                }

                if (coords.z < minZ) {
                    minZ = coords.z;
                }
            }

            glm::vec3 size = glm::vec3(std::abs(maxX - minX), std::abs(maxY - minY), std::abs(maxZ - minZ));
            glm::vec3 center = glm::vec3(minX + (maxX - minX) / 2.0f, minY + (maxY - minY) / 2.0f, minZ + (maxZ - minZ) / 2.0f);
            glm::vec3 max = glm::vec3(maxX, maxY, maxZ);
            glm::vec3 min = glm::vec3(minX, minY, minZ);

            BoundingBox bbBox {};
            bbBox.size = transform.scale * size;
            bbBox.center = center * transform.scale + transform.position;
            bbBox.rotation = transform.rotation;
            return bbBox;
        }

        void calculateNormals() {
            if (disableNormals)  return;

            std::map<unsigned int, std::vector<TriangleInfo>> vertexToTriangles;

            std::vector<TriangleInfo> triangles;

            for (unsigned int vertexId = 0; vertexId < vertices.size() / 3; vertexId++) {
                vertexToTriangles.insert(std::pair<int, std::vector<TriangleInfo>>(vertexId, triangles));
            }

            for (unsigned int j = 0; j < indices.size() / 3; j++) {
                unsigned int v1 = indices[j * 3];
                unsigned int v2 = indices[j * 3 + 1];
                unsigned int v3 = indices[j * 3 + 2];

                glm::vec3 v_1 = glm::vec3(vertices[v1 * 3], vertices[v1 * 3 + 1], vertices[v1 * 3 + 2]);
                glm::vec3 v_2 = glm::vec3(vertices[v2 * 3], vertices[v2 * 3 + 1], vertices[v2 * 3 + 2]);
                glm::vec3 v_3 = glm::vec3(vertices[v3 * 3], vertices[v3 * 3 + 1], vertices[v3 * 3 + 2]);

                glm::vec3 a = v_2 - v_1;
                glm::vec3 b = v_3 - v_1;

                TriangleInfo info{};
                info.angle = std::acos(glm::dot(a, b) / (glm::length(a) * glm::length(b)));
                info.normal = glm::normalize(glm::cross(a, b));

                vertexToTriangles.at(v1).emplace_back(info);
                vertexToTriangles.at(v2).emplace_back(info);
                vertexToTriangles.at(v3).emplace_back(info);
            }

            for (auto & pair : vertexToTriangles) {

                std::vector<TriangleInfo> triangleInfos = pair.second;

                glm::vec3 weightedSum = glm::vec3(0.0, 0.0, 0.0);

                for (auto & info : triangleInfos) {
                    weightedSum += info.normal * info.angle;
                }

                weightedSum /= triangleInfos.size();

                normals.insert(normals.end(), { weightedSum.x, weightedSum.y, weightedSum.z });
            }
        }

        void loadMesh(const char * path) {
            tinyobj::attrib_t attrib;

            std::string warning;
            std::string error;

            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;

            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, path);

            if (!error.empty()) {
                std::cerr << error << std::endl;
            }

            if (!warning.empty()) {
                std::cerr << error << std::endl;
            }

            vertices = attrib.vertices;

            for (const auto & shape : shapes) {
                for (const auto &index : shape.mesh.indices) {
                    indices.push_back((unsigned int) index.vertex_index);
                }
            }
        }
};

#endif //OPENGL_MESH_H
