project "minimal"
    kind "StaticLib"
    language "C"

    targetdir ("build/bin/" .. output_dir .. "/%{prj.name}")
    objdir ("build/bin-int/" .. output_dir .. "/%{prj.name}")

    files
    {
        "minimal/src/**.h",
        "minimal/src/**.c"
    }

    links
    {
        "$(VULKAN_SDK)/lib/vulkan-1.lib"
    }

    includedirs
    {
        "minimal/src",
        "$(VULKAN_SDK)/include"
    }

    defines
    {
        "MINIMAL_PLATFORM_WIN32",
        "MINIMAL_VULKAN"
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