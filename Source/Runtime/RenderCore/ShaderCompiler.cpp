#include <Runtime/RenderCore/ShaderCompiler.hpp>

#include <Runtime/RHI/RHITypes.hpp>

#include <Core/FileSystem/VFS.hpp>
#include <Core/Log/LogMacros.hpp>
#include <string>

#include "ShaderCompilerHelper.hpp"
#include "ShaderReflectingPrinting.hpp"
#include "ShaderUtils.hpp"

namespace CZ {

static const struct {
    SlangCompileTarget format;
    const char* profile;
} kTargets[] = {
    // { SLANG_DXIL, "sm_6_0" },
    { SLANG_SPIRV, "spirv_1_5" },
};
static const int kTargetCount = SLANG_COUNT_OF(kTargets);

ShaderCompiler& ShaderCompiler::Get() {
    static ShaderCompiler instance;
    return instance;
}

ShaderCompiler::ShaderCompiler() {
    if (SLANG_FAILED(slang::createGlobalSession(m_GlobalSession.writeRef()))) {
        CZ_RENDERCORE_LOG(Fatal, "Failed to create Slang global session.");
    }
}

bool ShaderCompiler::Compile(const ShaderCompilerMultiInput& input,
                             std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs) {
    std::filesystem::path sourcePath = VFS::Resolve(input.VirtualPath);

    // ShaderCompilerOutput output;
    std::string parentPath = sourcePath.parent_path().string();
    std::string fileName   = sourcePath.filename().string();

    slang::SessionDesc sessionDesc = {};

    slang::TargetDesc targetDescs[kTargetCount] = {};
    for (int i = 0; i < kTargetCount; i++) {
        targetDescs[i].format  = kTargets[i].format;
        targetDescs[i].profile = m_GlobalSession->findProfile(kTargets[i].profile);
    }

    sessionDesc.targets     = targetDescs;
    sessionDesc.targetCount = kTargetCount;

    const char* searchPaths[]   = { parentPath.c_str() };
    sessionDesc.searchPaths     = searchPaths;
    sessionDesc.searchPathCount = 1;

    std::vector<slang::PreprocessorMacroDesc> macroDescs;
    for (const auto& [name, value] : input.Macros.GetMap()) {
        macroDescs.push_back({ name.c_str(), value.c_str() });
    }
    sessionDesc.preprocessorMacros     = macroDescs.data();
    sessionDesc.preprocessorMacroCount = macroDescs.size();

    slang::CompilerOptionEntry entry;
    entry.name                           = slang::CompilerOptionName::VulkanUseEntryPointName;
    entry.value                          = slang::CompilerOptionValue{ .intValue0 = 1 };
    sessionDesc.compilerOptionEntries    = &entry;
    sessionDesc.compilerOptionEntryCount = 1;

    ComPtr<slang::ISession> session;
    RETURN_ON_FAIL(m_GlobalSession->createSession(sessionDesc, session.writeRef()));

    // ComPtr<slang::IComponentType> shaderProgram;
    // ComPtr<slang::ProgramLayout> programLayout;

    if (!CompileFromSource(session, fileName, outputs)) return false;

    // if (!CompileToSpirvForAllEntryPoints(shaderProgram, programLayout, outputs)) return false;

    return true;
}

bool ShaderCompiler::CompileFromSource(
    slang::ISession* session, const std::string fileName,
    std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs) {

    ComPtr<slang::IBlob> diagnostics;
    SlangResult result = SLANG_OK;

    slang::IModule* slangModule = nullptr;
    {
        slangModule = session->loadModule(fileName.c_str(), diagnostics.writeRef());

        diagnoseIfNeeded(diagnostics.get());
        if (!slangModule) {
            return false;
        }
    }

    std::vector<slang::IComponentType*> componentsToLink;

    // ### Variable decls
    for (auto decl : slangModule->getModuleReflection()->getChildren()) {
        if (auto varDecl = decl->asVariable(); varDecl &&
                                               varDecl->findModifier(slang::Modifier::Const) &&
                                               varDecl->findModifier(slang::Modifier::Static)) {
            // CZ_RENDERCORE_LOG(Trace, "Found static const variable: {}", varDecl->getName());
        }
    }

    // ### Finding Entry Points
    int definedEntryPointCount = slangModule->getDefinedEntryPointCount();
    for (int i = 0; i < definedEntryPointCount; i++) {
        ComPtr<slang::IEntryPoint> entryPoint;
        SLANG_RETURN_ON_FAIL(slangModule->getDefinedEntryPoint(i, entryPoint.writeRef()));

        // CZ_RENDERCORE_LOG(Trace, "Found entry point: {}",
        //                   entryPoint->getFunctionReflection()->getName());

        componentsToLink.push_back(entryPoint.get());
    }

    componentsToLink.push_back(slangModule);

    // ### Composing and Linking
    ComPtr<slang::IComponentType> composed;
    result = session->createCompositeComponentType(componentsToLink.data(), componentsToLink.size(),
                                                   composed.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    RETURN_ON_FAIL(result);

    ComPtr<slang::IComponentType> program;
    result = composed->link(program.writeRef(), diagnostics.writeRef());
    diagnoseIfNeeded(diagnostics);
    SLANG_RETURN_ON_FAIL(result);

    // ### Getting the Program Layout
    std::vector<slang::ProgramLayout*> programLayouts(kTargetCount);
    for (int targetIndex = 0; targetIndex < kTargetCount; ++targetIndex) {

        // ### Getting the Program Layout
        auto programLayout          = program->getLayout(targetIndex, diagnostics.writeRef());
        programLayouts[targetIndex] = programLayout;

        diagnoseIfNeeded(diagnostics);
        if (!programLayout) {
            result = SLANG_FAIL;
            continue;
        }

        // SLANG_RETURN_ON_FAIL(
        //     collectEntryPointMetadata(program, targetIndex, definedEntryPointCount));

        // auto targetFormat = kTargets[targetIndex].format;
        // PrintProgramLayout(programLayout, targetFormat);
    }

    if (!CompileToSpirvForAllEntryPoints(program, programLayouts[0], outputs)) return false;

    return true;
}

bool ShaderCompiler::CompileToSpirvForAllEntryPoints(
    ComPtr<slang::IComponentType>& shaderProgram, slang::ProgramLayout* programLayout,
    std::unordered_map<ShaderStage, ShaderCompilerOutput>& outputs) {
    bool allSuccess = true;

    if (programLayout) {
        unsigned int entryPointCount = programLayout->getEntryPointCount();
        for (unsigned int i = 0; i < entryPointCount; ++i) {
            slang::EntryPointLayout* entryPointLayout = programLayout->getEntryPointByIndex(i);
            const char* epName                        = entryPointLayout->getName();
            SlangStage epStage                        = entryPointLayout->getStage();
            ShaderStage stage = ShaderUtils::GetShaderStageFromSlangStage(epStage);

            ShaderCompilerOutput output;
            ComPtr<slang::IBlob> spirvCode;
            {
                ComPtr<slang::IBlob> diagnosticsBlob;
                SlangResult result = shaderProgram->getEntryPointCode(i, 0, spirvCode.writeRef(),
                                                                      diagnosticsBlob.writeRef());
                diagnoseIfNeeded(diagnosticsBlob);
                RETURN_ON_FAIL(result);
            }

            const uint32_t* spirvData = static_cast<const uint32_t*>(spirvCode->getBufferPointer());
            size_t spirvSize          = spirvCode->getBufferSize() / sizeof(uint32_t);

            output.Stage      = stage;
            output.EntryPoint = epName;
            output.Reflection = ReflectFromProgramLayout(programLayout);
            output.Binary.assign(spirvData, spirvData + spirvSize);

            outputs[stage] = std::move(output);
        }
    }

    return allSuccess;
}

ShaderReflection ShaderCompiler::ReflectFromProgramLayout(slang::ProgramLayout* programLayout) {
    ShaderReflection reflection;

    ShaderStage allStagesMask = ShaderStage::None;
    unsigned int epCount      = programLayout->getEntryPointCount();
    // ---------- Vertex Input Attributes ----------
    for (unsigned int epIdx = 0; epIdx < epCount; ++epIdx) {
        slang::EntryPointLayout* epLayout = programLayout->getEntryPointByIndex(epIdx);

        allStagesMask |= ShaderUtils::GetShaderStageFromSlangStage(epLayout->getStage());

        if (epLayout->getStage() != SLANG_STAGE_VERTEX) continue;

        unsigned int epParamCount = epLayout->getParameterCount();
        for (unsigned int p = 0; p < epParamCount; ++p) {
            slang::VariableLayoutReflection* param = epLayout->getParameterByIndex(p);
            slang::ParameterCategory category      = param->getTypeLayout()->getParameterCategory();

            if (category != slang::ParameterCategory::VertexInput) continue;

            const char* attrName = param->getName();
            if (!attrName) attrName = "unnamed";

            int location = param->getSemanticIndex(); // [[vk::location(location)]]
            slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
            ShaderDataType dataType = SlangTypeToShaderDataType(typeLayout->getType());

            reflection.Attributes.emplace_back(attrName, (uint32)location, dataType);
        }

        std::vector<AttributeInfo> sortedAttribs = reflection.Attributes;
        std::sort(
            sortedAttribs.begin(), sortedAttribs.end(),
            [](const AttributeInfo& a, const AttributeInfo& b) { return a.Location < b.Location; });

        VertexBufferLayout layout;
        for (const auto& attr : sortedAttribs) {
            layout.AddElement(attr.Type, attr.Name, attr.Location);
        }
        reflection.VertexBufferLayout = layout;
    }

    // ---------- Global Resource Bindings ----------
    unsigned int paramCount = programLayout->getParameterCount();
    for (unsigned int i = 0; i < paramCount; ++i) {
        slang::VariableLayoutReflection* varLayout = programLayout->getParameterByIndex(i);
        const char* name                           = varLayout->getName();
        if (!name) continue;

        slang::TypeLayoutReflection* typeLayout = varLayout->getTypeLayout();
        slang::ParameterCategory category       = typeLayout->getParameterCategory();

        // ---- Fill Push Constant ----
        if (category == slang::ParameterCategory::PushConstantBuffer) {
            PushConstantRange range;
            range.StageFlags = allStagesMask;
            range.Offset     = 0;
            range.Size       = static_cast<uint32_t>(typeLayout->getSize());
            reflection.PushConstants.push_back(range);
            continue;
        }

        // ---- Generic Infos ----
        int set = (int)varLayout->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot);
        int binding = (int)varLayout->getOffset(slang::ParameterCategory::DescriptorTableSlot);
        UniformType utype = GetUniformTypeFromSlangCategory(typeLayout);

        // ---- Fill Uniform Specification ----
        UniformSpecification spec;
        spec.Name      = name;
        spec.Set       = set;
        spec.Binding   = binding;
        spec.Type      = utype;
        spec.ArraySize = 1;
        spec.Size      = (uint32_t)typeLayout->getSize();

        reflection.Uniforms.push_back(spec);
        reflection.UniformLocations[name] = binding;

        // --- Fill Resource Bindings ---
        ShaderResourceBinding srb;
        srb.Binding    = binding;
        srb.Type       = utype;
        srb.StageFlags = static_cast<ShaderStage>(allStagesMask); // 假设 ShaderStage 可容纳多个位

        uint32_t descriptorCount = 1;
        if (typeLayout->getKind() == slang::TypeReflection::Kind::Array) {
            descriptorCount = typeLayout->getElementCount();
        }
        srb.DescriptorCount = descriptorCount > 0 ? descriptorCount : 1;

        reflection.ResourceBindings[set].push_back(srb);
    }

    return reflection;
}

void ShaderCompiler::PrintProgramLayout(slang::ProgramLayout* programLayout,
                                        SlangCompileTarget targetFormat) {
    ReflectingPrinting printer;
    printer._programLayout = programLayout;

    printer.printTargetFormat(targetFormat);

    ReflectingPrinting::AccessPath rootOffsets;
    rootOffsets.valid = true;

    printer.printScope(programLayout->getGlobalParamsVarLayout(), rootOffsets);

    int entryPointCount = programLayout->getEntryPointCount();
    for (int i = 0; i < entryPointCount; ++i) {
        printer.printEntryPointLayout(programLayout->getEntryPointByIndex(i), rootOffsets);
    }
}

} // namespace CZ