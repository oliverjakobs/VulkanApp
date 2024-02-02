project "ignis"
    kind "StaticLib"
    language "C"
    location "ignis"
    
    targetdir ("build/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("build/bin-int/" .. output_dir .. "/%{prj.name}")

    files
    {
        "ignis/src/**.h",
        "ignis/src/**.c"
    }

    links
    {
        "$(VULKAN_SDK)/lib/vulkan-1.lib"
    }

    includedirs
    {
        "ignis/src",
        "$(VULKAN_SDK)/include"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
        defines { "_CRT_SECURE_NO_WARNINGS" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"