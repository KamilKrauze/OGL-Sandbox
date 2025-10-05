shader_files = {
    "%{wks.location}/shaders/**.vert",
    "%{wks.location}/shaders/**.frag",
    "%{wks.location}/shaders/**.tess",
    "%{wks.location}/shaders/**.geom",
    "%{wks.location}/shaders/**.comp",
}

imgui_src_files = {
    "%{wks.location}/thirdparty/imgui/*.cpp",
    "%{wks.location}/thirdparty/imgui/backends/*opengl3*.cpp",
    "%{wks.location}/thirdparty/imgui/backends/*glfw*.cpp",
}

imgui_src_include = {
    "%{wks.location}/thirdparty/imgui/*.h",
    "%{wks.location}/thirdparty/imgui/backends/*opengl3*.h",
    "%{wks.location}/thirdparty/imgui/backends/*glfw*.h",
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
    include "./NormalMapping/premake5.lua"