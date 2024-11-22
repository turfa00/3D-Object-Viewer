#pragma once

#include <string>
#include <vector>
#include <assimp/include/scene.h>
#include <assimp/include/Importer.hpp>
#include <assimp/include/postprocess.h>
#include "shader.h"
#include "mesh.h"

class Model
{
public:
    Model(std::string path) {
        loadModel(path);
    }

    // Call draw function of all meshes in m_meshes
    void Draw(Shader shader) {
        for (unsigned int i = 0; i < m_meshes.size(); i++)
            m_meshes[i].Draw(shader);
    }

private:
    std::vector<Mesh> m_meshes;

    // Load model and call processNode
    void loadModel(std::string path) {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ASSIMP ERROR: " << import.GetErrorString() << std::endl;
            return;
        }

        processNode(scene->mRootNode, scene);
    }
    // Recursively process each Node by calling processMesh on each node's 
    // meshes and adding them to m_meshes vector
    void processNode(aiNode* node, const aiScene* scene) {
        // Process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene));
        }
        // Do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }
    // Create mesh object with vertex and index daa
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        // Load verticies
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex = {glm::vec3(0, 0, 0), glm::vec3(0, 0, 0)};
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
            vertices.push_back(vertex);
        }
        // Load indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return Mesh(vertices, indices);
    }
};