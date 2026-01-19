#pragma once
#include <Windows.h>
#include <string>
#include <dwmapi.h>

namespace overlay
{
    void render_loop();
    bool setup();
    void cleanup();
    inline HDC temp;
    inline HWND target_window = NULL;
    inline HWND overlay_window = NULL;
}

namespace drawing
{
    void draw_text(const std::string& text, int x, int y, COLORREF color);
    void draw_text(const std::string& text, int x, int y, COLORREF color, int pointSize, const char* face = "Segoe UI"); // surcharge temporaire

    void draw_line(int x1, int y1, int x2, int y2, int thickness, COLORREF color);
    void draw_rect(int x, int y, int w, int h, int thickness, COLORREF color);
}