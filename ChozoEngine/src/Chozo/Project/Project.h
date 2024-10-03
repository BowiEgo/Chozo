#pragma once

#include "Chozo/Asset/AssetManager.h"
#include "Chozo/Asset/EditorAssetManager.h"

namespace Chozo {

    struct ProjectConfig
	{
		std::string Name;

		std::string AssetDirectory;

		// Not serialized
		std::string ProjectFileName;
		std::string ProjectDirectory;
	};

    class Project : public RefCounted
    {
    public:
        Project();
        ~Project();

		const ProjectConfig& GetConfig() const { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		static void SetActive(Ref<Project> project);

        inline static Ref<AssetManager> GetAssetManager() { return s_AssetManager; }
		// inline static Ref<EditorAssetManager> GetEditorAssetManager() { return s_AssetManager.As<EditorAssetManager>(); }

        static fs::path GetAssetDirectory()
		{
			CZ_CORE_ASSERT(s_ActiveProject, "");
			return fs::path(s_ActiveProject->GetConfig().ProjectDirectory) / s_ActiveProject->GetConfig().AssetDirectory;
		}
    private:
		ProjectConfig m_Config;
		inline static Ref<AssetManager> s_AssetManager;

		inline static Ref<Project> s_ActiveProject;
    };
}