target("Platform")
    set_kind("static")
    
    if is_plat("windows") then
        add_files("Windows/*.cpp")
    elseif is_plat("linux") then
        add_files("Linux/*.cpp")
        elseif is_plat("macosx") then
        add_files("Mac/*.cpp")
        add_files("Mac/*.mm")
    end
    
    add_includedirs(".", {public = true})