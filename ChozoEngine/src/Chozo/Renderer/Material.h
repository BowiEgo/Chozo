#pragma once

#include "czpch.h"
#include "Shader.h"
#include "Texture.h"

namespace Chozo
{

    class Material : public Asset
    {
    public:
        static Ref<Material> Create(const std::string& name = "");
        static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");
        ~Material() override = default;

        static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetAssetType() const override { return GetStaticType(); }

		virtual void CopyProperties(Ref<Material> other) = 0;
        virtual void Set(const std::string& name, const UniformValue& value) = 0;
		virtual void Set(const std::string& name, const Ref<Texture>& texture) = 0;
		virtual void SetTextureHandle(const std::string& name, AssetHandle handle) = 0;
		virtual void Set(const std::string& name, const Ref<Texture2D>& texture) = 0;
		virtual void Set(const std::string& name, const Ref<TextureCube>& texture) = 0;

        virtual std::map<std::string, UniformValue> GetUniforms() = 0;
        virtual Ref<Texture2D> GetTexture(std::string name) = 0;
        virtual std::vector<Ref<Texture>> GetAllTextures() const = 0;
        virtual uint32_t GetLastTextureSlotIndex() const = 0;
        std::vector<std::tuple<std::string, AssetHandle>> GetTextureAssetHandles() const { return m_TextureAssetHandles; }

		virtual std::string GetName() = 0;
		virtual Ref<Shader> GetShader() const = 0;
    protected:
        std::vector<std::tuple<std::string, AssetHandle>> m_TextureAssetHandles;
    };

    class MaterialTable final : public RefCounted
    {
    public:
        explicit MaterialTable(uint32_t materialCount = 0);
        ~MaterialTable() override = default;

        bool HasMaterial(AssetHandle handle);
        uint32_t SetMaterial(AssetHandle handle);
        uint32_t SetMaterial(uint32_t index, AssetHandle handle);
        AssetHandle GetMaterial(const uint32_t index) const { return m_Materials.at(index); }
        inline void RemoveMaterial(uint32_t index);

        std::map<uint32_t, AssetHandle>& GetMaterials() { return m_Materials; }
        const std::map<uint32_t, AssetHandle>& GetMaterials() const { return m_Materials; }

		void SetMaterialCount(const uint32_t materialCount) { m_MaterialCount = materialCount; }
        uint32_t GetMaterialCount() const { return m_MaterialCount; }

		void Clear() { m_Materials.clear(); }
	private:
		std::map<uint32_t, AssetHandle> m_Materials;
		uint32_t m_MaterialCount;
    };
}
