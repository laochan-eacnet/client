simdjson = {
	source = path.join(dependencies.basePath, "simdjson"),
}

function simdjson.import()
	simdjson.includes()
end

function simdjson.includes()
	includedirs {
		simdjson.source,
	}
end

function simdjson.project()
	project "simdjson"
		language "C"

		simdjson.includes()

		files {
			path.join(simdjson.source, "**.h"),
			path.join(simdjson.source, "**.cpp"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, simdjson)
