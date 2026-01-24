target("Core")
    set_kind("shared")
    add_rules("chozo_module")
    add_packages("spdlog", {public = true})