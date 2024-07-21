#pragma once

class launcher_module : public saucer::module
{
private:
    saucer::native::window* window_;
    saucer::native::webview* webview_;
    saucer::smartview_core* smartview_;

    float scale_;

public:
    launcher_module(saucer::smartview_core* smartview);
    void init(saucer::native::window* window, saucer::native::webview* webview) override;
    void set_dpi_aware_size(int width, int height);
    void send_close();
};
