#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "GpuProgram.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Model {
public:
    Model(std::string const& path) {
        loadModel(path);
    }

    void Draw(const GpuProgram& shader, glm::mat4 transform) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            shader.SetUniform("model", glm::value_ptr(transform), false);
            meshes[i].Draw(shader);
        }
    }

    void Draw(
        const GpuProgram& shader,
        glm::mat4 baseModel,
        glm::mat4 cube001Model,
        glm::mat4 zxModel,
        glm::mat4 cvModel,
        glm::mat4 bnModel
    ) {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);

            switch (i) {
            case 0:
                model = baseModel;
                break;

            case 1:
                model = cube001Model;
                break;

            case 2:
                // Меш, который вращается от Z / X.
                // C / V на него НЕ действует.
                model = zxModel;
                break;

            case 3:
                // Меш перед нужным.
                // Двигается по C / V.
                model = cvModel;
                break;

            case 4:
                // Нужный меш.
                // Двигается по C / V и дополнительно по B / N.
                model = bnModel;
                break;

            default:
                model = glm::mat4(1.0f);
                break;
            }

            shader.SetUniform("model", glm::value_ptr(model), false);
            meshes[i].Draw(shader);
        }
    }

    void Draw(const GpuProgram& shader) {
        glm::mat4 identity = glm::mat4(1.0f);
        Draw(shader, identity);
    }

    size_t getMeshesCount() const {
        return meshes.size();
    }

private:
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string const& path) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_GenSmoothNormals
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }

        directory = path.substr(0, path.find_last_of('/'));

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            vertex.Position = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );

            if (mesh->HasNormals()) {
                vertex.Normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                );
            }
            else {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return Mesh(vertices, indices);
    }
};