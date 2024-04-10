nlohmann_json = {
	source = path.join(dependencies.basePath, "nlohmann_json"),
}

function nlohmann_json.import()
	nlohmann_json.includes()
end

function nlohmann_json.includes()
	includedirs {
		nlohmann_json.source,
	}
end

function nlohmann_json.project()
	
end

table.insert(dependencies, nlohmann_json)
