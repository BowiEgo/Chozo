function discover_all_tests(base_path)
    local test_files = {}
    
    local function scan_dir(current_path)
        if not os.isdir(current_path) then return end
        
        if path.filename(current_path) == "Tests" then
            local files = os.files(path.join(current_path, "**.cpp"))
            for _, file in ipairs(files) do
                table.insert(test_files, file)
            end
            print(string.format("  Found tests in: %s (%d files)", current_path, #files))
            return
        end
        
        for _, sub_dir in ipairs(os.dirs(path.join(current_path, "*"))) do
            scan_dir(sub_dir)
        end
    end
    
    scan_dir(base_path)
    return test_files
end

target("Test")
    set_kind("binary")
    add_rules("chozo_module")
    add_deps("Core", "RenderCore", "Asset")

    print("Scanning for test directories...")
        local test_files = discover_all_tests(path.join(os.projectdir(), "Source/Runtime"))

    for _, file in ipairs(test_files) do
            add_files(file)
        end

    add_files("Main.cpp")
    
    add_defines("CZ_BUILD_TESTS")
    set_targetdir("$(buildir)/tests")
    
    -- after_build(function(target)
    --     local test_path = path.join(target:targetdir(), target:basename())
    --     if os.isfile(test_path) then
    --         print("Running tests...")
    --         os.run(test_path)
    --     else
    --         print("Test executable not found: " .. test_path)
    --     end
    -- end)