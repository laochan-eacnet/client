webview = {
	source = path.join(dependencies.basePath, "webview"),
}

function webview.import()
	libdirs { path.join(webview.source, "lib") }
	links { "WebView2LoaderStatic" }

	webview.includes()
end

function webview.includes()
	includedirs {
		path.join(webview.source, "include"),
	}
end

function webview.project()
	
end

table.insert(dependencies, webview)
