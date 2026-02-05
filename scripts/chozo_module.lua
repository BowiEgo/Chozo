rule("chozo_module")
    on_load(function (target)
        local dir = target:scriptdir():gsub("\\", "/")
        local current_plat = target:plat()

        local all_platforms = {
            windows = "Windows",
            linux   = "Linux",
            macosx  = "Mac",
            android = "Android",
            iphoneos = "iOS"
        }

        -- Identify which directories should be BLACKLISTED
        local active_plat_name = all_platforms[current_plat]
        local blacklisted_dirs = {}
        for plat, folder in pairs(all_platforms) do
            if plat ~= current_plat then
                blacklisted_dirs[folder] = true
            end
        end

        -- Setup Export Macros
        -- Generate macro like CORE_EXPORTS, RENDER_CORE_EXPORTS
        local prefix = target:name():gsub("([a-z])([A-Z])", "%1_%2"):upper()
        
        if target:kind() == "shared" then
            target:add("defines", prefix .. "_EXPORTS")
        end

        -- Setup PCH
        -- Looks for TargetNamePCH.h in the target root folder
        local pch_name = target:name() .. "PCH.h"
        local pch_path = path.join(dir, pch_name)

        if os.isfile(pch_path) then
            target:set("pcxxheader", pch_path)
            print(string.format("Module [%s]: PCH detected -> %s", target:name(), pch_name))
        end

        -- Recursive function to traverse and add directories/files
        local function process_dir_recursive(current_path, is_public)
            if not os.isdir(current_path) then return end

            local folder_name = path.filename(current_path)
            if blacklisted_dirs[folder_name] then return end

            -- Add current directory to include paths
            -- Only add if it contains headers or is a known source container
            target:add("includedirs", current_path, {public = is_public})

            -- Add files in the CURRENT level (not using ** to maintain control)
            local header_files = os.files(path.join(current_path, "*.h"))
            local source_files = os.files(path.join(current_path, "*.cpp"))
            local mm_files     = os.files(path.join(current_path, "*.mm"))
            
            if #header_files > 0 then target:add("headerfiles", header_files) end
            if #source_files > 0 then target:add("files", source_files) end
            if #mm_files > 0     then target:add("files", mm_files) end

            -- Recurse into subdirectories
            for _, sub_dir in ipairs(os.dirs(path.join(current_path, "*"))) do
                -- Inherit 'is_public' status unless we hit a 'Private' folder
                local next_is_public = is_public
                if path.filename(sub_dir) == "Private" then
                    next_is_public = false
                end
                
                process_dir_recursive(sub_dir, next_is_public)
            end
        end

        target:add("includedirs", dir, {public = true})

        -- Start entry points
        -- We process root-level Public/Private and platform folders
        for _, sub in ipairs(os.dirs(path.join(dir, "*"))) do
            local folder_name = path.filename(sub)

            -- Case A - It's a platform folder
            if all_platforms[current_plat] == folder_name then
                -- Only process if it's the ACTIVE platform
                process_dir_recursive(path.join(sub, "Public"), true)
                process_dir_recursive(path.join(sub, "Private"), false)
            
            -- Case B - It's a general folder (e.g., Public, Private, or Common)
            elseif not blacklisted_dirs[folder_name] then
                if folder_name == "Public" then
                    process_dir_recursive(sub, true)
                elseif folder_name == "Private" then
                    process_dir_recursive(sub, false)
                else
                    -- If it's a generic folder like 'ThirdParty', default to Private
                    process_dir_recursive(sub, false)
                end
            end
        end
    end)
