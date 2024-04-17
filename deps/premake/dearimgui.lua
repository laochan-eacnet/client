dearimgui = {
	source = path.join(dependencies.basePath, "dearimgui"),
}

function dearimgui.import()
	links { "dearimgui" }
	dearimgui.includes()
end

function dearimgui.includes()
	includedirs {
		dearimgui.source,
		path.join(dearimgui.source, "backends"),
	}
end

function dearimgui.project()
	project "dearimgui"
		language "C++"

		dearimgui.includes()

		files {
			path.join(dearimgui.source, "imgui.cpp"),
			path.join(dearimgui.source, "imgui_draw.cpp"),
			path.join(dearimgui.source, "imgui_widgets.cpp"),
			path.join(dearimgui.source, "imgui_tables.cpp"),
			-- path.join(dearimgui.source, "imgui_demo.cpp"),
			path.join(dearimgui.source, "backends/imgui_impl_win32.cpp"),
			path.join(dearimgui.source, "backends/imgui_impl_dx9.cpp"),

			path.join(dearimgui.source, "imgui.h"),
			path.join(dearimgui.source, "backends/imgui_impl_win32.h"),
			path.join(dearimgui.source, "backends/imgui_impl_dx9.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, dearimgui)
