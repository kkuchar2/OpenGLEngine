#pragma once

#include "../Mesh.h"
#include "../../Utils/CharacterTextureGenerator.h"

class CharacterMesh : public Mesh {

    public:
        Character ch;

        CharacterMesh(char c, std::map<GLchar, Character> & characters) : Mesh() {

            ch = characters[c];

            textureId = ch.TextureID;

            vertices = {
                0.0f,  1.0f,   0.0,
                0.0f,  0.0f,   0.0,
                1.0f,  0.0f,   0.0,

                0.0f,  1.0f,   0.0,
                1.0f,  0.0f,   0.0,
                1.0f,  1.0f,   0.0
            };

            indices = {
                0, 1, 2, 3, 4, 5
            };

            uvs = {
                    0.0, 1.0,
                    0.0, 0.0,
                    1.0, 0.0,
                    0.0, 1.0,
                    1.0, 0.0,
                    1.0, 1.0
            };
        }
};