#pragma once

#include <string>
#include <vector>
#include <assimp/include/scene.h>
#include <assimp/include/Importer.hpp>
#include <assimp/include/postprocess.h>
#include "shader.h"
#include "mesh.h"
#include "menu.h"
class Model
{
public:
    Model() {

    }
    Model(std::string path) {
        loadModel(path);
    }

    // Call draw function of all meshes in m_meshes
    void Draw(Shader shader) {
        for (unsigned int i = 0; i < m_meshes.size(); i++)
            m_meshes[i].Draw(shader);
    }
    //Directly load a model with a shader and a  path
    //Useful when loading a model during program execution
    Model load3DModel(std::string path, Shader shader, Menu *menu) {
        // Instantiate and load model
        Model ourModel(path);
        // Build model matrix
        glm::mat4 model = glm::mat4(1.0f);

        //Model Transformations
        model = glm::translate(model, menu->translate);
        model = glm::scale(model, glm::vec3(menu->scale));
        model = glm::rotate(model, glm::radians(menu->rotationAngle), glm::vec3(menu->rotate));
        // Send model matrix to vertex shader as it remains constant
        shader.setMat4("model", model);

        //Material Colors
        shader.setVec3("material.ambient", menu->ambientMaterialColor);
        shader.setVec3("material.diffuse", menu->diffuseMaterialColor);
        shader.setVec3("material.specular", menu->specularMaterialColor);
        shader.setFloat("material.shininess", menu->shininess);

        //Scene lighting
        shader.setVec3("light.ambient", menu->ambientLightingColor);
        shader.setVec3("light.diffuse", menu->diffuseLightingColor);
        shader.setVec3("light.specular", menu->specularLightingColor);

        return ourModel;
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