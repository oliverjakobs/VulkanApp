workspace "Vulkan"
	architecture "x64"
	startproject "VulkanApp"

	configurations
	{
		"Debug",
		"OptimizedDebug",
		"Release"
	}

	flags { "MultiProcessorCompile" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		
	filter "configurations:OptimizedDebug"
		runtime "Debug"
		symbols "On"
		optimize "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"

output_dir = "%{cfg.buildcfg}"

group "Packages"

include "packages/glfw.lua"

group ""

project "VulkanApp"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"
	staticruntime "On"
	
	targetdir ("build/bin/" .. output_dir .. "/%{prj.name}")
	objdir ("build/bin-int/" .. output_dir .. "/%{prj.name}")

	files
	{
		--Config
		"config.ini",
		--Source
		"src/**.h",
		"src/**.c",
		--Resources
		"res/**.ini",
		"res/fonts/**.ttf",
		"res/shaders/**.vert",
		"res/shaders/**.frag",
		"res/textures/**.png"
	}

	links
	{
		"GLFW",
		"$(VULKAN_SDK)/Lib/vulkan-1.lib"
	}

	includedirs
	{
		"src",
		"packages/glfw/include",
		"packages/cglm/include",
		"$(VULKAN_SDK)/Include"
	}

	filter "system:linux"
		links { "dl", "pthread" }
		defines { "OBELISK_PLATFORM_GLFW", "_X11" }

	filter "system:windows"
		systemversion "latest"
		defines { "OBELISK_PLATFORM_GLFW", "WINDOWS", "_CRT_SECURE_NO_WARNINGS" }
