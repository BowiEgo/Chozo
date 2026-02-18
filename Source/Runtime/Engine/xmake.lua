target("Engine")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Platform", "Core", "RenderCore", "Windowing", "UI")
