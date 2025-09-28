shader_files = {
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

    include "./CommonLib/premake5.lua"
    include "./BasicGfxApp/premake5.lua"
    include "./ComputeShaderApp/premake5.lua"
    include "./BasicTexturing/premake5.lua"
    include "./DSA_DrawnObjects/premake5.lua"
    include "./DeferredRendering/premake5.lua"