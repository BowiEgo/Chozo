#include "AssetRegistrySerializer.h"

#include <yaml-cpp/yaml.h>

namespace Chozo {

    AssetRegistrySerializer::AssetRegistrySerializer(const AssetRegistry& context)
        : m_AssetRegistry(context)
    {
    }

    void AssetRegistrySerializer::Serialize(const std::string &filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Assets";
        if (!m_AssetRegistry.Empty())
        {
            out << YAML::BeginSeq;
            for (const auto& [handle, metadata] : m_AssetRegistry)
            {
                if (!metadata.IsMemoryAsset)
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "Handle" << YAML::Value << handle;
                    out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath;
                    out << YAML::Key << "FileSize" << YAML::Value << metadata.FileSize;
                    out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.Type);
                    out << YAML::EndMap;
                }
            }
            
            out << YAML::EndSeq;
        }
        else
        {
            out << YAML::Null;
        }

        out << YAML::EndMap;

        fs::path dirPath(filepath);
        if (!(fs::exists(dirPath) && fs::is_directory(dirPath)))
            fs::create_directories(dirPath.parent_path().string());

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void AssetRegistrySerializer::SerializeRuntime(const std::string &filepath)
    {
        // Not implemented
        CZ_CORE_ASSERT(false, "API Not implemented!");
    }

    AssetRegistry AssetRegistrySerializer::Deserialize(const std::string &filepath)
    {
        std::ifstream stream(filepath);
        if (!stream.is_open()) {
            CZ_CORE_ERROR("Failed to open file: %s", filepath.c_str());
            return m_AssetRegistry;
        }
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data;
        try {
            data = YAML::Load(strStream.str());
        } catch (const YAML::ParserException& e) {
            CZ_CORE_ERROR("Failed to parse YAML file: %s", e.what());
            return m_AssetRegistry;
        } catch (const YAML::BadFile& e) {
            CZ_CORE_ERROR("Failed to load YAML file: %s", e.what());
            return m_AssetRegistry;
        } catch (const std::exception& e) {
            CZ_CORE_ERROR("An error occurred while loading the YAML file: %s", e.what());
            return m_AssetRegistry;
        }

        try {
            if (!data["Assets"])
            {
                CZ_CORE_ERROR("YAML file does not contain a 'Assets' node");
                return m_AssetRegistry;
            }
        } catch (const YAML::BadSubscript& e) {
            CZ_CORE_ERROR("Failed to load YAML file: %s", e.what());
            return m_AssetRegistry;
        }

        auto assets = data["Assets"];
        if (assets)
        {
            for (auto asset : assets)
            {
                AssetMetadata metadata;
                metadata.Handle = asset["Handle"].as<uint64_t>();
                metadata.Type = Utils::StringToAssetType(asset["Type"].as<std::string>());
                metadata.FilePath = asset["FilePath"].as<std::string>();
                metadata.FileSize = asset["FileSize"].as<uint64_t>();
                m_AssetRegistry[metadata.Handle] = metadata;
            }
        }

        return m_AssetRegistry;
    }

    AssetRegistry AssetRegistrySerializer::DeserializeRuntime(const std::string &filepath)
    {
        // Not implemented
        CZ_CORE_ASSERT(false, "API Not implemented!");
        return m_AssetRegistry;
    }
}
