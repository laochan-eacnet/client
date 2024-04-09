dependencies = {
	basePath = "./deps"
}

function dependencies.load()
	dir = path.join(dependencies.basePath, "premake/*.lua")
	deps = os.matchfiles(dir)

	for i, dep in pairs(deps) do
		dep = dep:gsub(".lua", "")
		require(dep)
	end
end

function dependencies.imports()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.import()
		end
	end
end

function dependencies.projects()
	for i, proj in pairs(dependencies) do
		if type(i) == 'number' then
			proj.project()
		end
	end
end

newoption {
	trigger = "copy-to",
	description = "Optional, copy the EXE to a custom folder after build, define the path here if wanted.",
	value = "PATH"
}

newoption {
	trigger = "dev-build",
	description = "Enable development builds of the client."
}

dependencies.load()

workspace "laochan-client"
startproject "client"
location "./build"
objdir "%{wks.location}/obj"
targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

configurations {"Debug", "Release"}

language "C++"
cppdialect "C++20"

architecture "x86_64"
platforms "x64"

systemversion "latest"
symbols "On"
staticruntime "On"
editandcontinue "Off"
warnings "Extra"
characterset "ASCII"

if _OPTIONS["dev-build"] then
	defines {"DEV_BUILD"}
end

if os.getenv("CI") then
	defines {"CI"}
end

flags {"NoIncrementalLink", "NoMinimalRebuild", "MultiProcessorCompile", "No64BitChecks"}

filter "platforms:x64"
	defines {"_WINDOWS", "WIN32"}
filter {}

filter "configurations:Release"
	optimize "Size"
	buildoptions {"/GL"}
	linkoptions {"/IGNORE:4702", "/LTCG"}
	defines {"NDEBUG"}
	flags {"FatalCompileWarnings"}
filter {}

filter "configurations:Debug"
	optimize "Debug"
	defines {"DEBUG", "_DEBUG"}
filter {}

project "common"
kind "StaticLib"
language "C++"

files {"./src/common/**.hpp", "./src/common/**.cpp"}

includedirs {"./src/common", "%{prj.location}/src"}

resincludedirs {"$(ProjectDir)src"}

dependencies.imports()

project "client"
kind "SharedLib"
language "C++"

targetname "dinput8"

pchheader "std_include.hpp"
pchsource "src/client/std_include.cpp"

linkoptions {"/IGNORE:4254", "/DYNAMICBASE:NO", "/SAFESEH:NO", "/LARGEADDRESSAWARE", "/LAST:.main", "/PDBCompress"}

files {"./src/client/**.rc", "./src/client/**.hpp", "./src/client/**.cpp", "./src/client/resources/**.*"}

includedirs {"./src/client", "./src/common", "%{prj.location}/src"}

resincludedirs {"$(ProjectDir)src"}

links {"common"}

if _OPTIONS["copy-to"] then
	postbuildcommands {"copy /y \"$(TargetPath)\" \"" .. _OPTIONS["copy-to"] .. "\""}
end

dependencies.imports()

group "Dependencies"
dependencies.projects()
