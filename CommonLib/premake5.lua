project "CommonLib"
    location "."
    kind "StaticLib"
    language "C++"
    
    targetname "%{prj.name}"

    files { 
        "./%{wks.location}/%{prj.location}/**.h",
        "./%{wks.location}/%{prj.location}/**.hpp",
        "./%{wks.location}/%{prj.location}/**.c",
        "./%{wks.location}/%{prj.location}/**.cpp",

        "./%{wks.location}/thirdparty/glad/glad.c",
        
        imgui_src_files,
    }

    includedirs
    {
        "./%{wks.location}/%{prj.location}/",
        
        "./%{wks.location}/thirdparty/",
        "./%{wks.location}/thirdparty/KHR/",
        "./%{wks.location}/thirdparty/glm/",
        "./%{wks.location}/thirdparty/glfw/include/",
        "./%{wks.location}/thirdparty/imgui/",
    }

    libdirs
    {
        "./%{wks.location}/thirdparty/glfw/build/src/%{cfg.buildcfg}",
    }
    links
    {
        "glfw3.lib",
    }

    -- Windows system
    filter "system:windows"
        system "windows"
        cppdialect "C++17"
        systemversion "latest"

    -- Linux system
    filter "system:linux"
        system "linux"
        cppdialect "gnu++17"

    -- Build configurations
    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "NDEBUG"
        optimize "On"