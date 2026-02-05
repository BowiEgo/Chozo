target("Engine")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Platform")
    add_deps("Core")
    add_deps("RenderCore")

    add_packages("glfw")

