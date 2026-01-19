#include "overlay.hpp"
#include "game/esp.hpp"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
HDC hdc_overlay = NULL;
HBITMAP hbm_overlay = NULL;
HGDIOBJ hbm_old = NULL;
HWND game_window;
int g_width = 0;
int g_height = 0;
LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void check_window()
{
    if (!overlay::overlay_window) return;

    if (!IsWindow(overlay::overlay_window)) {
        LOG("!!! Window handle has become invalid. Overlay needs to be re-initialized.");
        overlay::overlay_window = NULL; 
        return;
    }

    if (!IsWindowVisible(overlay::overlay_window))
    {
        LOG("Window was hidden. Forcing SW_SHOW...");
        ShowWindow(overlay::overlay_window, SW_SHOW);
    }

    if (GetForegroundWindow() == overlay::target_window)
    {
        HWND current_top = GetWindow(GetDesktopWindow(), GW_CHILD);
        if (overlay::overlay_window != current_top) {
            LOG("Window lost topmost status. Resetting HWND_TOPMOST...");
            SetWindowPos(overlay::overlay_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
    }

    LONG_PTR style = GetWindowLongPtrA(overlay::overlay_window, GWL_EXSTYLE);
    if (!(style & WS_EX_LAYERED) || !(style & WS_EX_TRANSPARENT))
    {
        LOG("Window styles were tampered with. Restoring styles...");
        SetWindowLongPtrA(overlay::overlay_window, GWL_EXSTYLE, style | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
}

namespace menu {
    bool is_open = false;
    int selected_index = 0;

    DWORD last_key_press_time = 0;
    const int key_delay = 150;


    enum ItemType {
        TYPE_BOOL,
        TYPE_COLOR
    };

    struct MenuItem {
        std::string label;
        ItemType type;
        void* value;
    };

    std::vector<MenuItem> items;

    void init_items() {
        if (!items.empty()) return;

        // 1. ESP Box (ON/OFF)
        items.push_back({ "ESP Box",        TYPE_BOOL,  &settings::esp::box });
        items.push_back({ "ESP Bot Box",        TYPE_BOOL,  &settings::esp::bot_box });

        items.push_back({ "Show Bots",        TYPE_BOOL,  &settings::esp::show_bots });

        // 2. ESP Box Color (COLOR)
        items.push_back({ " > Box Color",   TYPE_COLOR, &settings::esp::box_color_idx });
        items.push_back({ " > Bot Box Color",   TYPE_COLOR, &settings::esp::bot_box_color_idx });

        // 3. ESP Snaplines
        items.push_back({ "ESP Lines",      TYPE_BOOL,  &settings::esp::snaplines });
        items.push_back({ " > Line Color",  TYPE_COLOR, &settings::esp::snaplines_color_idx });

 
        items.push_back({ "ESP Name",       TYPE_BOOL,  &settings::esp::name });
        items.push_back({ "ESP Bot Nane",        TYPE_BOOL,  &settings::esp::bot_name });

        items.push_back({ "ESP Health",     TYPE_BOOL,  &settings::esp::health });
        items.push_back({ "Distance",       TYPE_BOOL,  &settings::esp::distance });

        items.push_back({ "Loot ESP",       TYPE_BOOL,  &settings::esp::lootesp });

        items.push_back({ " > Loot Color",  TYPE_COLOR, &settings::esp::loot_color_idx });
        items.push_back({ "Loot ESP Box",       TYPE_BOOL,  &settings::esp::lootbox });
        items.push_back({ "Loot ESP Name",       TYPE_BOOL,  &settings::esp::lootname });
        items.push_back({ "Loot ESP Distance",       TYPE_BOOL,  &settings::esp::lootdistance });



    }

    void handle_input() {
        DWORD current_time = GetTickCount64();

        // Toggle Menu F8
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            if (current_time - last_key_press_time > 300) {
                is_open = !is_open;
                last_key_press_time = current_time;
            }
        }

        if (!is_open) return;

        if (current_time - last_key_press_time > key_delay) {

            // NAVIGATION HAUT/BAS
            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                selected_index--;
                if (selected_index < 0) selected_index = static_cast<int>(items.size()) - 1;
                last_key_press_time = current_time;
            }
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                selected_index++;
                if (selected_index >= items.size()) selected_index = 0;
                last_key_press_time = current_time;
            }

            // MODIFICATION VALEUR (GAUCHE / DROITE / ENTRÉE)
            bool left = (GetAsyncKeyState(VK_LEFT) & 0x8000);
            bool right = (GetAsyncKeyState(VK_RIGHT) & 0x8000);
            bool enter = (GetAsyncKeyState(VK_RETURN) & 0x8000);

            if (left || right || enter) {
                if (!items.empty()) {
                    MenuItem& item = items[selected_index];

                 
                    if (item.type == TYPE_BOOL) {
                        bool* val = static_cast<bool*>(item.value);
                        *val = !(*val);
                    }
                   
                    else if (item.type == TYPE_COLOR) {
                        int* val = static_cast<int*>(item.value);
                        int max_colors = static_cast<int>(settings::colors::list.size());

                        if (right || enter) {
                            *val = (*val + 1) % max_colors; 
                        }
                        else if (left) {
                            *val = (*val - 1); 
                            if (*val < 0) *val = max_colors - 1; 
                        }
                    }
                }
                last_key_press_time = current_time;
            }
        }
    }

    void render() {
        if (!is_open) return;
        init_items();

 
        int x = 50;
        int y = 300;
        int w = 250; 
        int h = 40 + (static_cast<int>(items.size()) * 25);

    
        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 20));
        RECT bgRect = { x, y, x + w, y + h };
        FillRect(hdc_overlay, &bgRect, bgBrush);
        DeleteObject(bgBrush);

     
        drawing::draw_rect(x, y, w, h, 2, RGB(0, 255, 0));
       
        drawing::draw_text("[- MENU: ARROWS -]", x + 25, y + 10, RGB(0, 255, 0));

        int current_y = y + 40;

        for (int i = 0; i < items.size(); i++) {
            bool is_selected = (i == selected_index);
            COLORREF text_color = is_selected ? RGB(255, 255, 0) : RGB(200, 200, 200);
            std::string prefix = is_selected ? "> " : "  ";

          
            drawing::draw_text(prefix + items[i].label, x + 10, current_y, text_color);

            
            std::string status_text;
            COLORREF status_color;

            if (items[i].type == TYPE_BOOL) {
                bool val = *static_cast<bool*>(items[i].value);
                status_text = val ? "[ON]" : "[OFF]";
                status_color = val ? RGB(0, 255, 0) : RGB(255, 0, 0);
            }
            else if (items[i].type == TYPE_COLOR) {
                int val = *static_cast<int*>(items[i].value);

              
                if (val < 0 || val >= settings::colors::names.size()) val = 0;

          
                status_text = "[" + settings::colors::names[val] + "]";

            
                status_color = settings::colors::list[val];
            }

     
            drawing::draw_text(status_text, x + w - 80, current_y, status_color);

            current_y += 25;
        }
    }
}
void overlay::render_loop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    menu::init_items();
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!overlay::overlay_window) {
            LOG("Overlay window is invalid, attempting to re-setup...");
            if (overlay::setup()) {
                LOG("Re-setup successful!");
            }
            else {
                LOG("Re-setup failed, will try again in 1s...");
                Sleep(1000);
                continue; 
            }
        }

        check_window();

        menu::handle_input(); 
        // Clear the overlay buffer
        BitBlt(hdc_overlay, 0, 0, settings::system::screen_x, settings::system::screen_y, NULL, 0, 0, BLACKNESS);
        overlay::temp = hdc_overlay;

        std::string Player_count = "Real Player count :";
        Player_count.append(std::to_string(settings::system::real_player_count));
        drawing::draw_text("Overlay Test: OK", 50, 50, RGB(0, 255, 0)); // Vert
        drawing::draw_text(Player_count, 50, 70, RGB(0, 255, 0)); // Vert
      
        //// Dessine un rectangle au centre de l'écran (en supposant 1920x1080)
        //drawing::draw_rect(910, 490, 100, 100, 2, RGB(255, 0, 0)); // Rouge, 2px d'épaisseur

        //// Dessine une ligne diagonale
        //drawing::draw_line(0, 0, 300, 300, 1, RGB(0, 0, 255)); // Bleu

        // --- All drawing happens here ---
        esp::render(); // Call your main drawing function
        // --------------------------------
        menu::render(); // <--- AJOUT ICI
        // Redraw the overlay window
        RECT rect;
        GetClientRect(overlay_window, &rect);
        HDC hdc_screen = GetDC(overlay_window);
        BitBlt(hdc_screen, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdc_overlay, 0, 0, SRCCOPY);
        ReleaseDC(overlay_window, hdc_screen);

        Sleep(1); // Reduce CPU usage
    }
}

bool isthreadcreated = false;
bool overlay::setup()
{
    // --- DÉBUT DE LA LOGIQUE DE HIJACKING NVIDIA ---
    //mem.initialize();
    // 1. Trouver la fenêtre de l'overlay NVIDIA
    LOG("Attempting to find NVIDIA Overlay window...");
    overlay_window = FindWindowA("CEF-OSC-WIDGET", "NVIDIA GeForce Overlay DT");
    if (!overlay_window)
    {
        LOG("!!! NVIDIA Overlay window not found.");
        return false;
    }


    // 2. Modifier les styles de la fenêtre pour la rendre superposable et transparente
    LONG_PTR window_style = GetWindowLongPtrA(overlay_window, GWL_EXSTYLE);
    SetWindowLongPtrA(overlay_window, GWL_EXSTYLE, window_style | WS_EX_LAYERED | WS_EX_TRANSPARENT);

    // 3. Étendre le cadre pour couvrir tout l'écran et supprimer les bordures
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(overlay_window, &margins);

    // 4. Définir la fenêtre comme topmost pour qu'elle reste au-dessus du jeu
    SetWindowPos(overlay_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // --- FIN DE LA LOGIQUE DE HIJACKING ---

    // On récupère la taille de l'écran pour notre canevas GDI
    g_width = GetSystemMetrics(SM_CXSCREEN);
    g_height = GetSystemMetrics(SM_CYSCREEN);

    // La méthode UpdateLayeredWindow que nous utilisons déjà est supérieure à SetLayeredWindowAttributes.
    // Nous n'avons donc pas besoin de SetLayeredWindowAttributes(overlay_window, RGB(0,0,0), 0, LWA_COLORKEY);

    // Configuration de GDI pour le dessin sur la fenêtre NVIDIA détournée
    HDC hdc_screen = GetDC(overlay_window);
    hdc_overlay = CreateCompatibleDC(hdc_screen);
    hbm_overlay = CreateCompatibleBitmap(hdc_screen, g_width, g_height);
    hbm_old = SelectObject(hdc_overlay, hbm_overlay);
    ReleaseDC(overlay_window, hdc_screen);

    ShowWindow(overlay_window, SW_SHOW);
    UpdateWindow(overlay_window);
    SetWindowDisplayAffinity(overlay_window, WDA_EXCLUDEFROMCAPTURE);

    if (isthreadcreated == false) {
        std::thread(cache::cache_loop).detach();
        std::thread(cache::cache_important_loop).detach();
        std::thread(cache::cache_player_loop).detach();
        std::thread(overlay::render_loop).detach();
        isthreadcreated = true;
    }


    return true;
}
void overlay::cleanup()
{
    SelectObject(hdc_overlay, hbm_old);
    DeleteObject(hbm_overlay);
    DeleteDC(hdc_overlay);
    DestroyWindow(overlay_window);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// --- Drawing Implementations ---

void drawing::draw_text(const std::string& text, int x, int y, COLORREF color)
{
    SetBkMode(hdc_overlay, TRANSPARENT);
    SetTextColor(hdc_overlay, color);
    TextOutA(hdc_overlay, x, y, text.c_str(), static_cast<int>(text.length()));
}
void drawing::draw_text(const std::string& text, int x, int y, COLORREF color, int pointSize, const char* face)
{
    if (!hdc_overlay) return;

    // calcule la hauteur en pixels à partir des points (utilise DPI de l'écran)
    HDC screen = GetDC(NULL);
    int dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(NULL, screen);
    int pixelHeight = -MulDiv(pointSize, dpi, 72); // négatif pour hauteur de caractère

    // crée la police temporaire
    HFONT hTempFont = CreateFontA(
        pixelHeight, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, face
    );

    if (!hTempFont)
    {
        // fallback : utilise la draw_text normale
        drawing::draw_text(text, x, y, color);
        return;
    }

    // sélectionne, dessine, restaure et détruit la police temporaire
    HFONT hOld = (HFONT)SelectObject(hdc_overlay, hTempFont);
    SetBkMode(hdc_overlay, TRANSPARENT);
    SetTextColor(hdc_overlay, color);
    TextOutA(hdc_overlay, x, y, text.c_str(), static_cast<int>(text.length()));
    SelectObject(hdc_overlay, hOld);
    DeleteObject(hTempFont);
}
void drawing::draw_line(int x1, int y1, int x2, int y2, int thickness, COLORREF color)
{
    HPEN hpen = CreatePen(PS_SOLID, thickness, color);
    HPEN hpen_old = (HPEN)SelectObject(hdc_overlay, hpen);
    MoveToEx(hdc_overlay, x1, y1, NULL);
    LineTo(hdc_overlay, x2, y2);
    SelectObject(hdc_overlay, hpen_old);
    DeleteObject(hpen);
}

void drawing::draw_rect(int x, int y, int w, int h, int thickness, COLORREF color)
{
    HPEN hpen = CreatePen(PS_SOLID, thickness, color);
    HGDIOBJ hpen_old = SelectObject(hdc_overlay, hpen);
    HGDIOBJ hbrush_old = SelectObject(hdc_overlay, GetStockObject(NULL_BRUSH));

    Rectangle(hdc_overlay, x, y, x + w, y + h);

    SelectObject(hdc_overlay, hpen_old);
    SelectObject(hdc_overlay, hbrush_old);
    DeleteObject(hpen);
}