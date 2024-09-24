#include "Project.h"

namespace Chozo {

    Project::Project()
    {
    }

    Project::~Project()
    {
    }

    void Project::SetActive(Ref<Project> project)
    {
		s_ActiveProject = project;
        // s_AssetManager = Ref<AssetManager>::Create();
    }
}