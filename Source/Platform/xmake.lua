target("Platform")
    set_kind("static")
    add_rules("chozo_module")
    
    if is_plat("macosx") then
        add_frameworks("AppKit", "QuartzCore")
    end