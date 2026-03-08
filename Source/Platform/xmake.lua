target("Platform")
    set_kind("static")
    add_rules("chozo_module")

    if is_plat("windows") then
        add_syslinks("advapi32", "shell32", "user32", "gdi32", "Ole32")
    end
    
    if is_plat("macosx") then
        add_frameworks("AppKit", "QuartzCore", "QuickLookThumbnailing", "UniformTypeIdentifiers")
    end