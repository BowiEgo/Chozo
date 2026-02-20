target("Editor")
    set_kind("binary")
    add_rules("chozo_module")
    add_deps("Launch", "Core", "Engine", "UI")
