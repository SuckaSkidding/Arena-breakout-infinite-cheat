#pragma once
#include <vector>
#include <string>
#include <windows.h> 

namespace settings {
    inline bool running = true;

    namespace colors {
   
        inline std::vector<COLORREF> list = {
            RGB(255, 0, 0),     
            RGB(0, 255, 0),    
            RGB(0, 0, 255),     
            RGB(255, 255, 0),   
            RGB(0, 255, 255),   
            RGB(255, 0, 255),  
            RGB(255, 255, 255), 
            RGB(0, 0, 0)       
        };

        inline std::vector<std::string> names = {
            "RED", "GREEN", "BLUE", "YELLOW", "CYAN", "PURPLE", "WHITE", "BLACK"
        };
    }

    namespace esp {
        inline bool on = true;
        inline float max_distance = 1200.0f;

    
        inline bool box = true;
        inline bool bot_box = true;

        inline bool show_bots = true;

        inline bool skeleton = true;
        inline bool snaplines = true;
        inline bool health = true;
        inline bool name = true;
        inline bool bot_name = true;

        inline bool distance = true;
        inline bool lootesp = true;
        inline bool lootname = true;
        inline bool lootdistance = true;
        inline bool lootbox = false;

 
        inline int box_color_idx = 1;
        inline int bot_box_color_idx = 1;

        inline int snaplines_color_idx = 0;
        inline int skeleton_color_idx = 6;
        inline int name_color_idx = 3;
        inline int loot_color_idx = 0;
    }      


    namespace system {
        inline int screen_x = 2560;
        inline int screen_y = 1440;
        inline int real_player_count = 0;
    }
}