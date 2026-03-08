target("UI")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "Platform", "RenderCore", "RHI", "Windowing", "imgui", "stb", "nanosvg")

