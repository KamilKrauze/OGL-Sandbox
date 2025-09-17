workspace "OGL-Sandbox"
    architecture "x64"
    configurations { "Debug", "Release" }

    targetdir "bin/{cfg.buildcfg}/%{cfg.system}"

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

project "AtmosphereScattering-Raster"
    location "AtmoScatt"
    kind "ConsoleApp"
    language "C++"
    
    targetname "%{prj.name}_%{cfg.architecture}"

    files { 
        "./%{prj.location}/**.h",
        "./%{prj.location}/**.hpp",
        "./%{prj.location}/**.c",
        "./%{prj.location}/**.cpp",

        "./thirdparty/glad/glad.c",
    }

    includedirs
    {
        "./thirdparty/",
        "./thirdparty/KHR/",
        "./thirdparty/glm/",
        "./thirdparty/glfw/include/",
    }

    libdirs
    {
        "thirdparty/glfw/build/src/%{cfg.buildcfg}",
    }
    links
    {
        "glfw3.lib",
    }