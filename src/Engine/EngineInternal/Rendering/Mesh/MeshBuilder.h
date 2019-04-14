#pragma once


#include <Rendering/Primitives/Quad.h>
#include <Rendering/Primitives/Cube.h>
#include <Rendering/Primitives/Surface.h>

#include <Rendering/Camera/Projection.h>
#include <Rendering/Shading/ShaderPool.h>
#include <Rendering/Primitives/Line.h>
#include <Rendering/Primitives/LinePrototype.h>

#include "SurfacePrototype.h"

class MeshBuilder {
    public:
        static std::shared_ptr<Mesh> of(const std::shared_ptr<MeshPrototype> & proto, const Projection & projection) {

            std::shared_ptr<Mesh> mesh;

            if (proto->path == nullptr) {
                 switch (proto->meshType) {
                    case LINE:
                        mesh = std::make_shared<Line>();
                        break;
                    case QUAD:
                        mesh = std::make_shared<Quad>();
                        break;
                    case CUBE:
                        mesh = std::make_shared<Cube>();
                        break;
                    case SURFACE:
                        mesh = std::make_shared<Surface>(300, 300);
                        break;
                    case RESOURCE:
                        mesh = std::make_shared<Mesh>(proto->path);
                        break;
                    case NONE:
                        mesh = std::make_shared<Mesh>(proto->path);
                        break;
                }


                if (proto->meshType == LINE) {
                    auto lineProto = std::dynamic_pointer_cast<LinePrototype>(proto);
                    auto lineMesh = std::dynamic_pointer_cast<Line>(mesh);
                    lineMesh->setCoords(lineProto->start, lineProto->end);
                }
            }
            else {
                mesh = std::make_shared<Mesh>(proto->path);
            }

            if (!mesh.get()) {
                std::cerr << "Mesh of type: " << proto->meshType << ", " << proto->path << " is null" << std::endl;
            }

            mesh->shader = ShaderPool::Instance().getShader(proto->shaderType);
            mesh->isInstanced = proto->instanced;
            mesh->disableNormals = proto->disableNormals;
            mesh->shaderInit = [proto](ShaderPtrRef s) {
                s->setVec4("color", proto->color);
                s->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
                s->setVec3("lightPos", glm::vec3(2.2f, 3.0f, 2.0f));
            };
            mesh->projection = projection;

            if (proto->texture != nullptr) {
                mesh->loadTexture(proto->texture);
            }


            return mesh;
        }
};