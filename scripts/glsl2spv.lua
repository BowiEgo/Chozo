rule("glsl2spv")
set_extensions(".vert", ".frag", ".comp", ".geom", ".tesc", ".tese", ".glsl")

on_build_file(function(target, sourcefile, opt)
    -- English comment: Locate glslc from Vulkan SDK
    import("lib.detect.find_tool")
    local glslc = assert(find_tool("glslc"), "glslc not found! Please install Vulkan SDK.")

    -- English comment: Set output directory to build/shaders/
    local shaderdir = path.join(target:targetdir(), "shaders")
    if not os.exists(shaderdir) then
        os.mkdir(shaderdir)
    end

    -- English comment: Construct output filename (e.g., shader.vert -> shader.vert.spv)
    local outputfile = path.join(shaderdir, path.filename(sourcefile) .. ".spv")

    -- English comment: Only recompile if source file is newer than output
    if os.mtime(sourcefile) > os.mtime(outputfile) then
        print("compiling shader: %s", sourcefile)
        os.run("%s %s -o %s", glslc.program, sourcefile, outputfile)
    end
end)
