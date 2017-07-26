#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

GLint TextureFromFile(const char* path, string directory);

#define MAX(a, b) (((a)>(b))?(a):(b))
#define MIN(a, b) (((a)<(b))?(a):(b))

class Model
{
public:
	aiScene* out_scene;
	/*  Functions   */
	// Constructor, expects a filepath to a 3D model.
	Model(GLchar* path)
	{
		this->loadModel(path);
	}

	// Draws the model, and thus all its meshes
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

	// Simply export obj from aiScene  // ZZA Added
	void exportModel()
	{
		Assimp::Exporter exporter;
		const aiExportFormatDesc* format = exporter.GetExportFormatDescription(3);
		aiReturn ret = exporter.Export(this->out_scene, format->id, "./output1.obj");//
		if (ret != AI_SUCCESS)
			cout << exporter.GetErrorString() << endl;
	}


	void updateNode(double depth, glm::mat4 &m_VM, glm::mat4 &m_PVM, glm::mat3 &m_homography, int index)
	{
		this->updateNode(this->out_scene->mRootNode, depth, m_VM, m_PVM, m_homography, index);
	}

	void updateNode(aiNode* node, double depth, glm::mat4 &m_VM, glm::mat4 &m_PVM, glm::mat3 &m_homography, int index)
	{
		// Release former-allocated resources
		if (out_scene->mMaterials[index])
			delete out_scene->mMaterials[index];

		// Allocate memory for material
		index = 0;
		aiMaterial* mat = new aiMaterial;
		out_scene->mMaterials[index] = mat;
		
		// Set the name of the material:
		string materialName = "diffuseTexture" + std::to_string(index);
		mat->AddProperty(&aiString(materialName.c_str()), AI_MATKEY_NAME);

		// Set the first diffuse texture
		string textureName = "diffuseTexture" + std::to_string(index) + ".jpg";
		mat->AddProperty(&aiString(textureName.c_str()), AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));

		// Is it enough???

		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = this->out_scene->mMeshes[node->mMeshes[i]];
			this->updateMesh(mesh, depth, m_VM, m_PVM, m_homography, index);
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->updateNode(node->mChildren[i], depth, m_VM, m_PVM, m_homography, index);
		}
		//out_scene->mMeshes[0]->mTextureCoords
	}

	void updateMesh(aiMesh* mesh, double depth, glm::mat4 &m_VM, glm::mat4 &m_PVM, glm::mat3 &m_homography, int index)
	{
		// Assign material index
		mesh->mMaterialIndex = index;
		mesh->mNumUVComponents[index] = 2;

		aiVector3D* TexCoords = new aiVector3D[mesh->mNumVertices];
		mesh->mTextureCoords[index] = TexCoords;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; ++i)
		{
			// Initialize all texture coordinate to 0.0f
			mesh->mTextureCoords[index][i].x = 0.0f;
			mesh->mTextureCoords[index][i].y = 0.0f;
		
			// Set a reference to simlify the following code
			const aiVector3D &coord_ = mesh->mVertices[i];
			glm::vec3 coord = glm::vec3(coord_.x, coord_.y, coord_.z);

			// This vertex is too far away from the front
			glm::vec4 transformed_model = m_VM * glm::vec4(coord, 1.0f);
			GLfloat finalcolor = transformed_model.z / this->getMaxBoundingValue(index) / 5 * 4 + 0.2;
			//if (transformed_model.z * this->getMaxBoundingValue(index) < 0) // cull the other half
			//	continue;
			//if (fabs(fabs(finalcolor) - depth) >= 0.1) // cull vertex outside my selected plane
			//	continue;
			if (finalcolor < depth) // cull vertex I can't see?
				continue;
			//if (I can't see these vertex)
			//continue

			// Convert 3D vertex coordinate to model view coordinate, V * M * Vertex
			glm::vec4 new_coord = m_PVM * glm::vec4(coord, 1.0f);

			// Convert to NDC
			new_coord.x /= new_coord.w;
			new_coord.y /= new_coord.w;
			new_coord.z /= new_coord.w;

			// Calculate lvalue of the plane equation Ax+By+Cz=1
			//const double lvalue = coord.x * coefficient.x + coord.y * coefficient.y + coord.z + coefficient.z;

			// This vertex is on the selected plane, compare lvalue with rvalue(1)
			//if (true)//(fabs(lvalue - 1) < 1e-2) // NO NEED TOCHECK 
			//{


			// Calculate texture coordinate using homography matrix
			glm::vec3 screen_coord = glm::vec3(new_coord.x, new_coord.y, 1.0f); // 3*1
			glm::vec3 texture_coord = m_homography * screen_coord; // 3*3 * 3*1
			mesh->mTextureCoords[index][i].x = (1 + texture_coord.x/ texture_coord.z) / 2.0f;
			mesh->mTextureCoords[index][i].y = (1 + texture_coord.y/ texture_coord.z) / 2.0f;

			//}
		}
	}

	GLfloat getMaxBoundingValue(GLint index) {
		if (index < 0 || index > 5)
			return 0.0f;
		return this->max_bounding_value[index];
	}

	/* Always split vertices from the first mesh's first child */
	void splitVertex(glm::mat4 &m_VM, glm::mat4 &m_PVM, GLfloat* depth_image, GLuint width, GLuint height, GLint camera_index) {
		aiScene* scene = this->out_scene;
		unsigned int num_meshes = scene->mNumMeshes;
		aiMesh** old_mMeshes = scene->mMeshes;
		aiMesh** new_mMeshes = new aiMesh*[num_meshes + 1];
		
		for (int i = 0; i < num_meshes; ++i)
			new_mMeshes[i] = old_mMeshes[i];
		delete old_mMeshes;
		scene->mMeshes = new_mMeshes;
		scene->mNumMeshes++;
		new_mMeshes[num_meshes] = new aiMesh;

		
		aiNode* node = this->out_scene->mRootNode->mChildren[0];
		int num_meshes_index = node->mNumMeshes;
		unsigned int * old_mMeshes_index = node->mMeshes;
		unsigned int * new_mMeshes_index = new unsigned int[num_meshes_index + 1];
		
		for (int i = 0; i < num_meshes_index; ++i)
			new_mMeshes_index[i] = old_mMeshes_index[i];
		delete old_mMeshes_index;
		node->mMeshes = new_mMeshes_index;
		node->mNumMeshes++;
		new_mMeshes_index[num_meshes_index] = num_meshes; // original 0 will be ignored in next ignore..(), so it starts from 0 ???
		
		aiMesh* source_mesh = scene->mMeshes[0];
		aiMesh* target_mesh = scene->mMeshes[num_meshes];
		vector<bool> vertex_list;
		vector<aiVector3D> valid_vertex;
		vector<aiFace> valid_face;
		for (int i = 0; i < source_mesh->mNumVertices; i++)
		{
			// Set a reference to simlify the following code
			const aiVector3D &coord_ = source_mesh->mVertices[i];
			glm::vec3 coord = glm::vec3(coord_.x, coord_.y, coord_.z);

			// Generate depth value of this vertex in current camera set and store it in finalcolor
			glm::vec4 transformed_model = m_VM * glm::vec4(coord, 1.0f);
			GLfloat final_color = transformed_model.z / this->getMaxBoundingValue(camera_index) / 5 * 4 + 0.2;

			// Convert 3D vertex coordinate to model view coordinate, P * V * M * Vertex
			glm::vec4 new_coord = m_PVM * glm::vec4(coord, 1.0f);

			// Normalization
			new_coord.x /= new_coord.w;
			new_coord.y /= new_coord.w;

			new_coord.x = (new_coord.x + 1.0f) / 2.0f;
			new_coord.y = (new_coord.y + 1.0f) / 2.0f;

			// Get depth value of the corresponding pixel
			GLint index = (GLuint)(new_coord.x * width) + (GLuint)(new_coord.y * height) * width;
			if (index < 0 || index >= height*width)
			{
				vertex_list.push_back(false);
				continue;
			}
			GLfloat actual_depth = depth_image[index];

			// set this vertex to false if this vertex is behind current view (should be culled)
			//cout << final_color << "   " << actual_depth << endl;
			if (fabs(final_color - actual_depth) > 5e-2)
				vertex_list.push_back(false);
			else
			{
				vertex_list.push_back(true);
				valid_vertex.push_back(source_mesh->mVertices[i]);
			}
		}

		// Copy valid vertices to target mesh
		target_mesh->mVertices = new aiVector3D[valid_vertex.size()];
		target_mesh->mNumVertices = valid_vertex.size();
		for (int i = 0; i < valid_vertex.size(); ++i)
			target_mesh->mVertices[i] = valid_vertex[i];
		
		for (int i = 0; i < source_mesh->mNumFaces; ++i)
		{
			const aiFace &face = source_mesh->mFaces[i];
			bool is_all_true = true;
			for (int j = 0; j < face.mNumIndices; ++j)
				is_all_true = is_all_true && vertex_list[face.mIndices[j]];
			if (is_all_true)
				valid_face.push_back(face);
		}
		
		// Copy valid faces to target mesh
		target_mesh->mFaces = new aiFace[valid_face.size()];
		target_mesh->mNumFaces = valid_face.size();
		for (int i = 0; i < valid_face.size(); ++i)
			target_mesh->mFaces[i] = valid_face[i];

		target_mesh->mName = aiString(string("Mesh0") + to_string(camera_index));
		cout << "[" << camera_index << "]" << valid_vertex.size() << "/" << source_mesh->mNumVertices << " " << valid_face.size() << "/" << source_mesh->mNumFaces << endl;
	}

	void ignoreFirstMeshInRootNode()
	{
		aiNode* node = this->out_scene->mRootNode->mChildren[0];
		int num_meshes_index = node->mNumMeshes;
		unsigned int * old_mMeshes_index = node->mMeshes;
		unsigned int * new_mMeshes_index = new unsigned int[num_meshes_index - 1];

		memcpy_s(new_mMeshes_index, num_meshes_index - 1, old_mMeshes_index + 1, num_meshes_index - 1);

		delete old_mMeshes_index;
		node->mMeshes = new_mMeshes_index;
		node->mNumMeshes--;
	}

	void genTextureCoord(glm::mat4 &m_PVM, glm::mat3 &m_homography, GLuint camera_index)
	{
		// For Arbitary number of materials
		//aiScene* scene = this->out_scene;
		//unsigned int num_materials = scene->mNumMaterials;
		//aiMaterial** old_mMaterials = scene->mMaterials;
		//aiMaterial** new_mMaterials = new aiMaterial*[num_materials + 1];

		//for (int i = 0; i < num_materials; ++i)
		//	new_mMaterials[i] = old_mMaterials[i];
		//delete old_mMaterials;
		//scene->mMaterials = new_mMaterials;
		//scene->mNumMaterials++;
		//new_mMaterials[num_materials] = new aiMaterial;

		// Release former-allocated resources
		if (out_scene->mMaterials[camera_index])
			delete out_scene->mMaterials[camera_index];

		// Allocate memory for material
		aiMaterial* mat = new aiMaterial;
		out_scene->mMaterials[camera_index] = mat;

		// Set the name of the material:
		string materialName = "diffuseTexture" + std::to_string(camera_index);
		mat->AddProperty(&aiString(materialName.c_str()), AI_MATKEY_NAME);

		// Set the first diffuse texture
		string textureName = "diffuseTexture" + std::to_string(camera_index) + ".jpg";
		mat->AddProperty(&aiString(textureName.c_str()), AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));

		// Bind mesh
		aiMesh* mesh = this->out_scene->mMeshes[camera_index];

		// Assign material index
		mesh->mMaterialIndex = camera_index;
		mesh->mNumUVComponents[0] = 2;

		// Allocate memory for TexCoords
		aiVector3D* TexCoords = new aiVector3D[mesh->mNumVertices];
		mesh->mTextureCoords[0] = TexCoords;

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; ++i)
		{
			// Set a reference to simlify the following code
			const aiVector3D &coord_ = mesh->mVertices[i];
			glm::vec3 coord = glm::vec3(coord_.x, coord_.y, coord_.z);

			// Convert 3D vertex coordinate to model view coordinate, P * V * M * Vertex
			glm::vec4 new_coord = m_PVM * glm::vec4(coord, 1.0f);

			// Normalization
			new_coord.x /= new_coord.w;
			new_coord.y /= new_coord.w;
			new_coord.z /= new_coord.w;

			// Calculate texture coordinate using homography matrix
			glm::vec3 screen_coord = glm::vec3(new_coord.x, new_coord.y, 1.0f); // 3*1
			glm::vec3 texture_coord = m_homography * screen_coord; // 3*3 * 3*1
			mesh->mTextureCoords[0][i].x = (1 + texture_coord.x / texture_coord.z) / 2.0f;
			mesh->mTextureCoords[0][i].y = (1 + texture_coord.y / texture_coord.z) / 2.0f;
		}
	}

private:
	/*  Model Data  */
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	const aiScene* scene; // ZZA Modified
	GLfloat max_bounding_value[6]; // ZZA Modified

	/*  Functions   */
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string path)
	{
		for (int i = 0; i < 6; i++)
			max_bounding_value[i] = 0.0f;
		// Read file via ASSIMP
		Assimp::Importer importer;
		this->scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		// Check for errors
		if (!this->scene || this->scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !this->scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));
		// Process ASSIMP's root node recursively
		this->processNode(this->scene->mRootNode, this->scene);
		// Copy a new pointer for modification (according to specification)  // ZZA Added
		aiCopyScene(this->scene, &(this->out_scene));
		//cout << this->out_scene->mMaterials << " " << this->out_scene->mNumMaterials << endl;
		//cout << endl;
		this->out_scene->mMaterials = new aiMaterial*[6];
		for (int i = 0; i < 6; ++i)
			this->out_scene->mMaterials[i] = new aiMaterial;
		this->out_scene->mNumMaterials = 6;
	}

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process each mesh located at the current node
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			// The node object only contains indices to index the actual objects in the scene. 
			// The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene, this->max_bounding_value));
		}
		// After we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene, GLfloat* local_max_bounding_value)
	{
		// Data to fill
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;
		//GLfloat local_max_bounding_value[6] = { 0 };

		// Walk through each of the mesh's vertices
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector; // We declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							  // Positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			//std::cout << vector.z << std::endl;

			local_max_bounding_value[1] = MAX(local_max_bounding_value[1], vector.x); // x-axis pos
			local_max_bounding_value[3] = MIN(local_max_bounding_value[3], vector.x); // x-axis neg
			local_max_bounding_value[4] = MAX(local_max_bounding_value[4], vector.y); // y-axis pos
			local_max_bounding_value[5] = MIN(local_max_bounding_value[5], vector.y); // y-axis neg
			local_max_bounding_value[0] = MAX(local_max_bounding_value[0], vector.z); // z-axis pos
			local_max_bounding_value[2] = MIN(local_max_bounding_value[2], vector.z); // z-axis neg

			// Normals
			if (mesh->HasNormals()) // Does the mesh contain normals? // ZZA Added
			{
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			else
				vertex.Normal = glm::vec3(1.0f, 1.0f, 1.0f);

			// Texture Coordinates
			if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
			{
				glm::vec2 vec;
				// A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			vertices.push_back(vertex);
		}
		// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			// Retrieve all indices of the face and store them in the indices vector
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// Process materials
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			// We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// Diffuse: texture_diffuseN
			// Specular: texture_specularN
			// Normal: texture_normalN

			// 1. Diffuse maps
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. Specular maps
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		// Return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; // A texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}
			if (!skip)
			{   // If texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);  // Store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}


};




GLint TextureFromFile(const char* path, string directory)
{
	//Generate texture ID and load texture data 
	string filename = string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}