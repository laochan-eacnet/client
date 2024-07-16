ifs_layeredfs = {
	source = path.join(dependencies.basePath, "ifs_layeredfs"),
}

function ifs_layeredfs.import()
	links { "ifs_layeredfs" }
end

function ifs_layeredfs.includes()
	includedirs {
		path.join(ifs_layeredfs.source, "src"),
	}
end

function ifs_layeredfs.project()
	project "ifs_layeredfs"
		language "C"

		ifs_layeredfs.includes()
		defines {"VER_STRING=\"laochan static linked\"", "strcasecmp=_stricmp"}

		files {
			path.join(ifs_layeredfs.source, "src/*.h"),
			path.join(ifs_layeredfs.source, "src/*.cpp"),

			path.join(ifs_layeredfs.source, "src/3rd_party/*.cpp"),
			path.join(ifs_layeredfs.source, "src/3rd_party/libsquish/**.cpp"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, ifs_layeredfs)
