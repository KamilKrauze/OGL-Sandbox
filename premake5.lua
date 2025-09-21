proj_common_includes = {
    "%{wks.location}/common/",
}

proj_common_srcs = {
    "%{wks.location}/common/**.c",
    "%{wks.location}/common/**.cpp",
    "%{wks.location}/common/**.h",
    "%{wks.location}/common/**.hpp",

    "%{wks.location}/shaders/**.vert",
    "%{wks.location}/shaders/**.frag",
    "%{wks.location}/shaders/**.tess",
    "%{wks.location}/shaders/**.geom",
    "%{wks.location}/shaders/**.comp",

}

workspace "OGL-Sandbox"
    architecture "x64"
    configurations { "Debug", "Release" }

    targetdir "bin/%{cfg.buildcfg}/%{cfg.system}"

    include "./AtmoScatt/premake5.lua"


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