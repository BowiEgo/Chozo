target("FileSystem")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "RHI", "stb")
