webview2 = {
	source = path.join(dependencies.basePath, "webview2"),
}

function webview2.import()
	libdirs { path.join(webview2.source, "lib") }
	links { "WebView2LoaderStatic" }
	
	webview2.includes()
end

function webview2.includes()
	includedirs {
		path.join(webview2.source, "include"),
	}
end

function webview2.project()
	
end

table.insert(dependencies, webview2)
