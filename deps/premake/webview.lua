webview = {
	source = path.join(dependencies.basePath, "webview"),
}

function webview.import()
	links { "webview" }
	webview.includes()
end

function webview.includes()
	includedirs {
		path.join(webview.source, "src"),
		-- path.join(dependencies.basePath, "WebView2Loader/build/native/include"),
		-- path.join(dependencies.basePath, "WebView2Loader/build/native/x64"),

	}
end

function webview.project()
	project "webview"
		language "C++"

		webview.includes()

		files {
			path.join(webview.source, "src/**.cc"),
			path.join(webview.source, "src/**.h"),
			-- path.join(dependencies.basePath, "WebView2Loader/build/native/include/**.h"),
		-- path.join(dependencies.basePath, "WebView2Loader/build/native/x64/**.dll"),
		-- path.join(dependencies.basePath, "WebView2Loader/build/native/x64/**.lib"),


		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, webview)
