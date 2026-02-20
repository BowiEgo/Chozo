target("Launch")
    set_kind("static")
    add_rules("chozo_module", "export_header")
    add_deps("CopyBinaries", "Engine")