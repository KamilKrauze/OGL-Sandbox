project "AtmosphereScattering-Raster"
    location "."
    kind "ConsoleApp"
    language "C++"
    
    targetname "%{prj.name}_%{cfg.architecture}"

    files { 
        "./%{wks.location}/%{prj.location}/**.h",
        "./%{wks.location}/%{prj.location}/**.hpp",
        "./%{wks.location}/%{prj.location}/**.c",
        "./%{wks.location}/%{prj.location}/**.cpp",

        "./%{wks.location}/thirdparty/glad/glad.c",
        
        proj_common_srcs,
        proj_common_includes

    }

    includedirs
    {
        "./%{wks.location}/%{prj.location}/",
        
        "./%{wks.location}/thirdparty/",
        "./%{wks.location}/thirdparty/KHR/",
        "./%{wks.location}/thirdparty/glm/",
        "./%{wks.location}/thirdparty/glfw/include/",

        proj_common_includes
    }

    libdirs
    {
        "./%{wks.location}/thirdparty/glfw/build/src/%{cfg.buildcfg}",
    }
    links
    {
        "glfw3.lib",
    }