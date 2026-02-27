target("Launch")
    set_kind("binary")
    add_rules("chozo_module")
    add_deps("CopyBinaries", "Engine")

    if is_plat("macosx") then
        -- [Note] This tells macOS that your app can handle Retina displays
        add_values("bundle.info.NSHighResolutionCapable", true)
    end