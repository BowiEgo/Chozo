target("VulkanRHI")
    set_kind("shared")
    add_rules("chozo_module", "export_header")
    add_deps("Core", "Platform", "RHI", "VulkanSDK_Interface", "vma")

    