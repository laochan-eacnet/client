saucer = {
	source = path.join(dependencies.basePath, "saucer"),
}

function saucer.import()
	filter "configurations:Release"
		libdirs { path.join(saucer.source, "lib/release") }
		links { "fmt" }

	filter "configurations:Debug"
		libdirs { path.join(saucer.source, "lib/debug") }
		links { "fmtd" }

	filter {}

	links { "WebView2LoaderStatic", "saucer" }
	saucer.includes()
end

function saucer.includes()
	includedirs {
		path.join(saucer.source, "include"),
	}
end

function saucer.project()
end

table.insert(dependencies, saucer)
