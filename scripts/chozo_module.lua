rule("chozo_module")
    on_load(function (target)
        local dir = target:scriptdir():gsub("\\", "/")

        -- 1. Setup Include Directories
        -- Include the root of Public/Private
        target:add("includedirs", dir .. "/Public", {public = true})
        target:add("includedirs", dir .. "/Private")
        
        -- Automatically add all subdirectories under Public to includedirs
        for _, sub in ipairs(os.dirs(path.join(dir, "Public/**"))) do
            target:add("includedirs", sub, {public = true})
        end
        for _, sub in ipairs(os.dirs(path.join(dir, "Private/**"))) do
            target:add("includedirs", sub)
        end

        -- 2. Add source files with PCH support
        local function add_files_if_exists(target, subdir, pattern, is_header)
            local search_path = path.join(dir, subdir, pattern)
            local matched_files = os.files(search_path)
            
            if #matched_files > 0 then
                if is_header then
                    target:add("headerfiles", search_path)
                else
                    target:add("files", search_path)
                end
            end
        end

        -- Traverse all subdirectories recursively using ** pattern
        local subdirs = {"Public", "Private"}
        for _, subdir in ipairs(subdirs) do
            add_files_if_exists(target, subdir, "**.h", true)
            add_files_if_exists(target, subdir, "**.cpp", false)
        end

        -- 3. Setup PCH and Export Macros
        local pch_name = target:name() .. "PCH.h"
        local pch_path = path.join(dir, "Public", pch_name)
        if os.isfile(pch_path) then
            target:set("pcxxheader", pch_path)
        end

        local export_macro = target:name():upper() .. "_EXPORTS"
        print("Configuring module: " .. target:name() .. " with macro: " .. export_macro)

        if target:kind() == "shared" then
            target:add("defines", export_macro)
            
            if not target:is_plat("windows") then
                -- Hide all symbols by default on Linux/Mac to match Windows behavior
                target:add("cxflags", "-fvisibility=hidden")
            end
        end

        -- Ensure Public headers are propagated to dependent targets
        local dir = target:scriptdir():gsub("\\", "/")
        target:add("includedirs", dir .. "/Public", {public = true})
    end)