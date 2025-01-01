
#include "AssimpLoader.h"
#include "TextureImporter.h"

#include "Chozo/Core/Application.h"
#include "Chozo/Renderer/Renderer.h"

#include <assimp/Importer.hpp>

namespace Chozo {

    namespace Utils {

        glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix)
        {
            glm::mat4 result;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
            result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
            result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
            result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
            return result;
        }
    }

    Ref<MeshSource> AssimpLoader::LoadFromFile(const fs::path &filepath)
    {
		CZ_CORE_INFO("AssimpLoader: {0}", filepath.string());
		auto path = filepath.string();

    	uint32_t importFlag =
			aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
			aiProcess_Triangulate |             // Make sure we're triangles
			aiProcess_SortByPType |             // Split meshes by primitive type
			aiProcess_GenNormals |              // Make sure we have legit normals
			aiProcess_GenUVCoords |             // Convert UVs if required
	//		aiProcess_OptimizeGraph |
			aiProcess_OptimizeMeshes |          // Batch draws where possible
			aiProcess_JoinIdenticalVertices |
			aiProcess_LimitBoneWeights |        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
			aiProcess_ValidateDataStructure |   // Validation
			aiProcess_GlobalScale;              // e.g. convert cm to m for fbx import (and other formats where cm is native)

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		// importFlag |= aiProcess_FlipUVs;
		const aiScene* scene = importer.ReadFile(path, importFlag);
        if (!scene /* || !scene->HasMeshes()*/)  // note: scene can legit contain no meshes (e.g. it could contain an armature, an animation, and no skin (mesh)))
		{
			CZ_CORE_ERROR("Failed to load mesh file: {0}", path);
			return nullptr;
		}

		Ref<MeshSource> meshSource = Ref<MeshSource>::Create();
		// Meshes
        if (scene->HasMeshes())
		{
            uint32_t vertexCount = 0;
			uint32_t indexCount = 0;

            meshSource->m_BoundingBox.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
			meshSource->m_BoundingBox.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			meshSource->m_Submeshes.reserve(scene->mNumMeshes);
            for (unsigned m = 0; m < scene->mNumMeshes; m++)
			{
                aiMesh* mesh = scene->mMeshes[m];

				Submesh& submesh = meshSource->m_Submeshes.emplace_back();
				submesh.BaseVertex = vertexCount;
				submesh.BaseIndex = indexCount;
				submesh.MaterialIndex = mesh->mMaterialIndex;
				submesh.VertexCount = mesh->mNumVertices;
				submesh.IndexCount = mesh->mNumFaces * 3;
				submesh.MeshName = mesh->mName.C_Str();

				vertexCount += mesh->mNumVertices;
				indexCount += submesh.IndexCount;

				CZ_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
				CZ_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

                // Vertices
				auto& aabb = submesh.BoundingBox;
				aabb.Min = { FLT_MAX, FLT_MAX, FLT_MAX };
				aabb.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex{};
					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
					aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
					aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
					aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
					aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
					aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
					aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					meshSource->m_Buffer.Vertexs.push_back(vertex);
				}

                // Indices
				for (size_t i = 0; i < mesh->mNumFaces; i++)
				{
					CZ_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.");
					Index index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };
					meshSource->m_Buffer.Indexs.push_back(index);

					meshSource->m_TriangleCache[m].emplace_back(meshSource->m_Buffer.Vertexs[index.V1 + submesh.BaseVertex], meshSource->m_Buffer.Vertexs[index.V2 + submesh.BaseVertex], meshSource->m_Buffer.Vertexs[index.V3 + submesh.BaseVertex]);
				}
            }

            MeshNode& rootNode = meshSource->m_Nodes.emplace_back();
			TraverseNodes(meshSource, scene->mRootNode, 0);

			for (const auto& submesh : meshSource->m_Submeshes)
			{
				AABB transformedSubmeshAABB = submesh.BoundingBox;
				glm::vec3 min = glm::vec3(submesh.Transform * glm::vec4(transformedSubmeshAABB.Min, 1.0f));
				glm::vec3 max = glm::vec3(submesh.Transform * glm::vec4(transformedSubmeshAABB.Max, 1.0f));

				meshSource->m_BoundingBox.Min.x = glm::min(meshSource->m_BoundingBox.Min.x, min.x);
				meshSource->m_BoundingBox.Min.y = glm::min(meshSource->m_BoundingBox.Min.y, min.y);
				meshSource->m_BoundingBox.Min.z = glm::min(meshSource->m_BoundingBox.Min.z, min.z);
				meshSource->m_BoundingBox.Max.x = glm::max(meshSource->m_BoundingBox.Max.x, max.x);
				meshSource->m_BoundingBox.Max.y = glm::max(meshSource->m_BoundingBox.Max.y, max.y);
				meshSource->m_BoundingBox.Max.z = glm::max(meshSource->m_BoundingBox.Max.z, max.z);
			}
        }

		// Materials
		if (scene->HasMaterials())
		{
			meshSource->m_Materials.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{
				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();

				auto mi = Material::Create(Renderer::GetRendererData().m_ShaderLibrary->Get("Geometry"), aiMaterialName.data);

				glm::vec3 baseColorColor(0.8f);
				float emission = 0.0f;
				aiColor3D aiColor, aiEmission;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS)
					baseColorColor = { aiColor.r, aiColor.g, aiColor.b };

				if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmission) == AI_SUCCESS)
					emission = aiEmission.r;

				mi->Set("u_Material.BaseColor", baseColorColor);
				// mi->Set("u_Material.Emission", emission);

				float metallic, roughness;
				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metallic) != aiReturn_SUCCESS)
					metallic = 0.0f;

				if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS)
					roughness = 0.5f;

				mi->Set("u_Material.Metallic", metallic);
				mi->Set("u_Material.Roughness", roughness);

				ApplyTextureByType(mi, aiMaterial, scene, PBRMaterialTextureType::BaseColor, filepath);
				ApplyTextureByType(mi, aiMaterial, scene, PBRMaterialTextureType::MetallicRoughness, filepath);
				ApplyTextureByType(mi, aiMaterial, scene, PBRMaterialTextureType::Normal, filepath);

				Application::GetAssetManager()->AddMemoryOnlyAsset(mi);
				meshSource->m_Materials[i] = mi->Handle;
			}
		}

        return meshSource;
    }

    void AssimpLoader::TraverseNodes(Ref<MeshSource> meshSource, void* assimpNode, uint32_t nodeIndex, const glm::mat4& parentTransform, uint32_t level)
	{
		auto* aNode = (aiNode*)assimpNode;

		MeshNode& node = meshSource->m_Nodes[nodeIndex];
		node.Name = aNode->mName.C_Str();
		node.LocalTransform = Utils::Mat4FromAIMatrix4x4(aNode->mTransformation);

		glm::mat4 transform = parentTransform * node.LocalTransform;
		for (uint32_t i = 0; i < aNode->mNumMeshes; i++)
		{
			uint32_t submeshIndex = aNode->mMeshes[i];
			auto& submesh = meshSource->m_Submeshes[submeshIndex];
			submesh.NodeName = aNode->mName.C_Str();
			submesh.Transform = transform;
			submesh.LocalTransform = node.LocalTransform;

			node.Submeshes.push_back(submeshIndex);
		}

		uint32_t parentNodeIndex = (uint32_t)meshSource->m_Nodes.size() - 1;
		node.Children.resize(aNode->mNumChildren);
		for (uint32_t i = 0; i < aNode->mNumChildren; i++)
		{
			MeshNode& child = meshSource->m_Nodes.emplace_back();
			size_t childIndex = meshSource->m_Nodes.size() - 1;
			child.Parent = parentNodeIndex;
			meshSource->m_Nodes[nodeIndex].Children[i] = childIndex;
			TraverseNodes(meshSource, aNode->mChildren[i], uint32_t(childIndex), transform, level + 1);
		}
	}

    void AssimpLoader::ApplyTextureByType(Ref<Material> target, aiMaterial* aiMaterial, const aiScene* scene, PBRMaterialTextureType propType, fs::path filepath)
    {
		aiString aiTexPath;
		aiTextureType aiTexType;
		std::string propTypeName;

		switch (propType)
		{
		case PBRMaterialTextureType::BaseColor:
			aiTexType = aiTextureType_DIFFUSE;
			propTypeName = "BaseColor";
			break;
		case PBRMaterialTextureType::Metallic:
			aiTexType = aiTextureType_METALNESS;
			propTypeName = "Metallic";
			break;
		case PBRMaterialTextureType::Roughness:
			aiTexType = aiTextureType_SHININESS;
			propTypeName = "Roughness";
			break;
		case PBRMaterialTextureType::Normal:
			aiTexType = aiTextureType_NORMALS;
			propTypeName = "Normal";
			break;
		default:
			break;
		}

		target->Set("u_Material.Enable" + propTypeName + "Map", false);

		bool hasMap = false;
		if (propType == PBRMaterialTextureType::BaseColor) {
			hasMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
			if (!hasMap) {
				hasMap = aiMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &aiTexPath) == AI_SUCCESS;
			}
		}
		else if (propType == PBRMaterialTextureType::Normal)
		{
			hasMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS;
			if (!hasMap) {
				hasMap = aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &aiTexPath) == AI_SUCCESS;
			}
		}
		else
		{
			hasMap = aiMaterial->GetTexture(aiTexType, 0, &aiTexPath) == AI_SUCCESS;
		}

		auto fileName = fs::path(aiTexPath.data).stem().string();

		if (hasMap)
		{
			Ref<Texture2D> texture;
			Texture2DSpecification spec;
			spec.WrapS = ImageParameter::REPEAT;
			spec.WrapT = ImageParameter::REPEAT;
			spec.MinFilter = ImageParameter::LINEAR;
			spec.MagFilter = ImageParameter::LINEAR;
			spec.DebugName = fileName;

			if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
			{
				spec.Format = ImageFormat::RGBA;
				spec.Width = aiTexEmbedded->mWidth;
				spec.Height = aiTexEmbedded->mHeight;
				Buffer imageBuffer = TextureImporter::ToBufferFromMemory(Buffer(aiTexEmbedded->pcData, spec.Width), spec.Format, spec.Width, spec.Height);
				texture = Texture2D::Create(imageBuffer, spec);
			}
			else
			{
				// spec.FlipY = false;
				auto parentPath = filepath.parent_path();
				parentPath /= std::string(aiTexPath.data);
				std::string texturePath = parentPath.string();
				texture = Texture2D::Create(texturePath, spec);
			}

			if (texture)
			{
				target->Set("u_" + propTypeName + "Map", texture);
				target->Set("u_Material.Enable" + propTypeName + "Map", true);
				Application::GetAssetManager()->AddMemoryOnlyAsset(texture);
			}
		}
    }

} // namespace Chozo
