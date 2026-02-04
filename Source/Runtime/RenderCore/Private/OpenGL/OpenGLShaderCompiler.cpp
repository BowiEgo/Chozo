#include "OpenGLShaderCompiler.h"
#include "FileUtils.h"

bool COpenGLShaderCompiler::CompileInternal(const FShaderCompilerInput &input,
                                            FShaderCompilerOutput &output) {
    // std::string source = ChozoUtils::File::ReadTextFile(input.SourcePath);

    // // Manual Macro Injection
    // std::string macroPrefix = "#version 450 core\n";
    // for (const auto &[name, value] : input.Macros) {
    //     macroPrefix += "#define " + name + " " + value + "\n";
    // }

    // // Manual Include Resolving (since OpenGL driver won't do it)
    // std::string processedSource = ResolveIncludes(source, input.SourcePath);

    // output.SourceCode = macroPrefix + processedSource;
    // output.bSucceeded = true;
    return true;
};