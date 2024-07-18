#pragma once
#include <map>
#include <string>
#include <saucer/webview.hpp>

#include "assets/iidx-BAOXvOlX.jpg.hpp"
#include "assets/index-CiFZe6x-.css.hpp"
#include "assets/index-n-48k65D.js.hpp"
#include "assets/moai-bg-BU2gdkgA.jpg.hpp"
#include "favicon.ico.hpp"
#include "index.html.hpp"

namespace saucer::embedded
{
    inline auto all()
    {
        std::map<std::string, embedded_file> rtn;

        rtn.emplace("assets/iidx-BAOXvOlX.jpg", embedded_file{"image/jpeg", assets_iidx_BAOXvOlX_jpg});
        rtn.emplace("assets/index-CiFZe6x-.css", embedded_file{"text/css", assets_index_CiFZe_x__css});
        rtn.emplace("assets/index-n-48k65D.js", embedded_file{"application/javascript", assets_index_n___k__D_js});
        rtn.emplace("assets/moai-bg-BU2gdkgA.jpg", embedded_file{"image/jpeg", assets_moai_bg_BU_gdkgA_jpg});
        rtn.emplace("favicon.ico", embedded_file{"image/vnd.microsoft.icon", favicon_ico});
        rtn.emplace("index.html", embedded_file{"text/html", index_html});

        return rtn;
    }
} // namespace saucer::embedded
