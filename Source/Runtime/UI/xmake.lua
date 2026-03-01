target("UI")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "RenderCore", "RHI", "Windowing", "imgui", "stb")

    if is_plat("windows") then
        add_syslinks("advapi32", "shell32", "user32", "gdi32")
    end
