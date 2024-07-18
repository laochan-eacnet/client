#pragma once

class launcher_module : public saucer::module
{
private:
    saucer::native::window* m_window;
    saucer::native::webview* m_webview;

public:
    launcher_module(saucer::smartview_core* smartview);
    void init(saucer::native::window* window, saucer::native::webview* webview) override;
};
