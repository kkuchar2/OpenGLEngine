#pragma once

#include <Rendering/Mesh/Mesh.h>

class Line : public Mesh {
    public:

        explicit Line() : Mesh() {
            vertices = { 0.0f, 0.0f, 0.0, 1.0f, 0.0f, 0.0 };
            indices = { 0, 1 };
        }

        void setCoords(const glm::vec3 & start, const glm::vec3 & end) {
            vertices[0] = start.x;
            vertices[1] = start.y;
            vertices[2] = start.z;
            vertices[3] = end.x;
            vertices[4] = end.y;
            vertices[5] = end.z;
        }
};