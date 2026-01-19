#pragma once


#include <game/cache.hpp>
#include "settings.hpp"
#include <algorithm>
#include "memory.hpp"
struct Vector2
{
    float x, y;

    Vector2() : x(0.f), y(0.f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
};
Vector2 get_text_size(const std::string& text)
{
    SIZE size;
    if (overlay::temp && GetTextExtentPoint32A(overlay::temp, text.c_str(), text.length(), &size))
    {
        return Vector2(static_cast<float>(size.cx), static_cast<float>(size.cy));
    }
    return Vector2(0, 0);
}
namespace esp {
    /* void Draw2DBox(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, ImU32 color, float thickness) {
             float width = max.x - min.x;
             float height = max.y - min.y;
             float cornerSize = fminf(20, fminf(width * 0.25f, height * 0.25f));

             drawList->AddLine(ImVec2(min.x, min.y), ImVec2(min.x + cornerSize, min.y), color, thickness);
             drawList->AddLine(ImVec2(min.x, min.y), ImVec2(min.x, min.y + cornerSize), color, thickness);

             drawList->AddLine(ImVec2(max.x - cornerSize, min.y), ImVec2(max.x, min.y), color, thickness);
             drawList->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + cornerSize), color, thickness);

             drawList->AddLine(ImVec2(min.x, max.y - cornerSize), ImVec2(min.x, max.y), color, thickness);
             drawList->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + cornerSize, max.y), color, thickness);

             drawList->AddLine(ImVec2(max.x - cornerSize, max.y), ImVec2(max.x, max.y), color, thickness);
             drawList->AddLine(ImVec2(max.x, max.y - cornerSize), ImVec2(max.x, max.y), color, thickness);
     }*/

    void render() {
        FVector my_location = mem.Read<FVector>(cache::Local::LocalRootComponent + offsets::USceneComponent::RelativeLocation);
        FMinimalViewInfo cam = cache::Local::CameraData::Camera;
        // Player list
        for (const auto& p : cache::ActorList) {
            FVector enemy_location = mem.Read<FVector>(p.actor_rootcomponent + offsets::USceneComponent::RelativeLocation);
            float distance = my_location.Distance(enemy_location);

            if (distance / 100.0f > settings::esp::max_distance)
                continue;
           // if (p.IsFriend)
           //     continue;
            if (p.is_dead)
                continue;
            if (std::isnan(p.health) || p.health <= 0.f || p.health > 101.f)
                continue;
            FVector2D esp_point;
            if (!sdk::W2S(cam, enemy_location, &esp_point, 1.0f)) continue;

            
            int snapcolor_idx = settings::esp::snaplines_color_idx;
            COLORREF snapcolor = settings::colors::list[snapcolor_idx];


            float dist_m = distance / 100.0f;

            float base_height = 100000.0f / distance;
            float dist_scale = std::clamp(1.0f - (dist_m * 0.0025f), 0.60f, 1.0f);
            float box_height = std::clamp(base_height * dist_scale, 20.0f, 250.0f);

            float box_width = box_height * 0.55f; 
           // float box_height = std::clamp(150000.0f / distance, 80.0f, 250.0f);
            //float box_width = box_height * 0.6f;
            Vector2 min = Vector2(esp_point.x - box_width / 2, esp_point.y - box_height / 2);
            Vector2 max = Vector2(esp_point.x + box_width / 2, esp_point.y + box_height / 2);

      
            int y_offset = 0; 

            bool is_bot = p.bot;

            FVector head = sdk::GetbonePos(p.actor_mesh,Bones::Head);

            int texttcolor_idx = settings::esp::box_color_idx;

            COLORREF textcolor = settings::colors::list[texttcolor_idx];
            int bot_boxcolor_idx = settings::esp::bot_box_color_idx;
          
            if (!p.player_name.empty()) {
                is_bot = false;
                Vector2 name_size = get_text_size(p.player_name);
                if (settings::esp::name) {
                    if (settings::esp::bot_name) {
                        drawing::draw_text(p.player_name, esp_point.x - name_size.x / 2, min.y - 20, textcolor);
                    }
                y_offset += 15;
                }
            }
            else {
                if (settings::esp::show_bots == false)
                    continue;
                Vector2 name_size = get_text_size("BOT");
                if (settings::esp::name) {
                  
                    textcolor = settings::colors::list[bot_boxcolor_idx];
                drawing::draw_text("BOT", esp_point.x - name_size.x / 2, min.y - 20, textcolor);
                y_offset += 15;
                }
            }
            if (settings::esp::snaplines) {
                if (settings::esp::show_bots) {
                    if (is_bot) {
                        drawing::draw_line(settings::system::screen_x / 2, 0, esp_point.x, esp_point.y, 2, snapcolor);

                    }

                }
                else {
                    drawing::draw_line(settings::system::screen_x / 2, 0, esp_point.x, esp_point.y, 2, snapcolor);

                }
            
                
            }
            if (settings::esp::box) {
                float cornerSize = fminf(20, fminf(box_width * 0.25f, box_height * 0.25f));
                int boxcolor_idx = settings::esp::box_color_idx;
                

                COLORREF boxcolor = settings::colors::list[boxcolor_idx];
                if (is_bot) {
                    boxcolor = settings::colors::list[bot_boxcolor_idx];
                    if (settings::esp::bot_box) {
                        drawing::draw_line(min.x, min.y, min.x + cornerSize, min.y, 2, boxcolor);
                        drawing::draw_line(min.x, min.y, min.x, min.y + cornerSize, 2, boxcolor);
                        drawing::draw_line(max.x - cornerSize, min.y, max.x, min.y, 2, boxcolor);
                        drawing::draw_line(max.x, min.y, max.x, min.y + cornerSize, 2, boxcolor);
                        drawing::draw_line(min.x, max.y - cornerSize, min.x, max.y, 2, boxcolor);
                        drawing::draw_line(min.x, max.y, min.x + cornerSize, max.y, 2, boxcolor);
                        drawing::draw_line(max.x - cornerSize, max.y, max.x, max.y, 2, boxcolor);
                        drawing::draw_line(max.x, max.y - cornerSize, max.x, max.y, 2, boxcolor);
                    }
                }
                else {
                    drawing::draw_line(min.x, min.y, min.x + cornerSize, min.y, 2, boxcolor);
                    drawing::draw_line(min.x, min.y, min.x, min.y + cornerSize, 2, boxcolor);
                    drawing::draw_line(max.x - cornerSize, min.y, max.x, min.y, 2, boxcolor);
                    drawing::draw_line(max.x, min.y, max.x, min.y + cornerSize, 2, boxcolor);
                    drawing::draw_line(min.x, max.y - cornerSize, min.x, max.y, 2, boxcolor);
                    drawing::draw_line(min.x, max.y, min.x + cornerSize, max.y, 2, boxcolor);
                    drawing::draw_line(max.x - cornerSize, max.y, max.x, max.y, 2, boxcolor);
                    drawing::draw_line(max.x, max.y - cornerSize, max.x, max.y, 2, boxcolor);
                }
              

      

            }
            if (settings::esp::skeleton)
            {

                FVector vNeck = sdk::GetbonePos(p.actor_mesh, 16);
                FVector vPelvis = sdk::GetbonePos(p.actor_mesh, Bones::Pelvis);
    
                FVector vFootL = sdk::GetbonePos(p.actor_mesh, Bones::Foot_L);
                FVector vFootR = sdk::GetbonePos(p.actor_mesh, Bones::Foot_R);

         
                FVector vHandL = sdk::GetbonePos(p.actor_mesh, Bones::Hand_L);
                FVector vHandR = sdk::GetbonePos(p.actor_mesh, Bones::Hand_R);

                FVector2D sNeck, sPelvis, sFootL, sFootR, sHandL, sHandR;

         
                if (sdk::W2S(cam, vNeck, &sNeck, 1.f) && sdk::W2S(cam, vPelvis, &sPelvis, 1.f))
                {
                    COLORREF skelColor = RGB(255, 255, 255);

          
                    drawing::draw_line(sNeck.x, sNeck.y, sPelvis.x, sPelvis.y, 1, skelColor);

      
                    if (sdk::W2S(cam, vFootL, &sFootL, 1.f)) drawing::draw_line(sPelvis.x, sPelvis.y, sFootL.x, sFootL.y, 1, skelColor);
                    if (sdk::W2S(cam, vFootR, &sFootR, 1.f)) drawing::draw_line(sPelvis.x, sPelvis.y, sFootR.x, sFootR.y, 1, skelColor);

                    if (sdk::W2S(cam, vHandL, &sHandL, 1.f)) drawing::draw_line(sNeck.x, sNeck.y, sHandL.x, sHandL.y, 1, skelColor);
                    if (sdk::W2S(cam, vHandR, &sHandR, 1.f)) drawing::draw_line(sNeck.x, sNeck.y, sHandR.x, sHandR.y, 1, skelColor);
                }
            }
  

         

            if (settings::esp::distance) {
            std::string distance_text = std::format("{:.0f}m", distance / 100.0f);
            Vector2 distance_size = get_text_size(distance_text);
            drawing::draw_text(distance_text, esp_point.x - distance_size.x / 2, max.y + 5, textcolor);
            }
            if (settings::esp::health) {
            std::string health_text = std::format("HP: {:.0f}", p.health);
            drawing::draw_text(health_text, max.x + 5, min.y, textcolor);
        }
        }

        for (const auto& p : cache::LootList ) {
            if (!settings::esp::lootesp)
                continue;
            FVector enemy_location = mem.Read<FVector>(p.actor_rootcomponent + offsets::USceneComponent::RelativeLocation);
            float distance = my_location.Distance(enemy_location);
            
            if (distance / 100.0f > settings::esp::max_distance)
                continue;


            FVector2D esp_point;
            if (!sdk::W2S(cam, enemy_location, &esp_point, 1.0f)) continue;

           // drawing::draw_line(1920 / 2, 0, esp_point.x, esp_point.y, 2, RGB(0, 255, 0));
            
            float dist_m = distance / 100.0f;

            float base_height = 100000.0f / distance;
            float dist_scale = std::clamp(1.0f - (dist_m * 0.0025f), 0.60f, 1.0f);
            float box_height = std::clamp(base_height * dist_scale, 20.0f, 250.0f);

            float box_width = box_height * 0.55f;
            Vector2 min = Vector2(esp_point.x - box_width / 2, esp_point.y - box_height / 2);
            Vector2 max = Vector2(esp_point.x + box_width / 2, esp_point.y + box_height / 2);
            box_height = box_height / 3;
            // --- Dessin du texte (Nom, Distance, Vie) ---
            int y_offset = 0; // Pour empiler le texte

                          
            int lootcolor_idx = settings::esp::loot_color_idx;
            COLORREF lootcolor = settings::colors::list[lootcolor_idx];
            if (settings::esp::lootname) {
          
                Vector2 name_size = get_text_size(p.actor_name);
                drawing::draw_text(p.actor_name, esp_point.x - name_size.x / 2, min.y - 20, lootcolor, 8, "Consolas");
                y_offset += 15;
            }
        

            
            if(settings::esp::lootbox){
                float cornerSize = fminf(20, fminf(box_width * 0.25f, box_height * 0.25f));

       

                drawing::draw_line(min.x, min.y, min.x + cornerSize, min.y, 2, lootcolor);
                drawing::draw_line(min.x, min.y, min.x, min.y + cornerSize, 2, lootcolor);
                drawing::draw_line(max.x - cornerSize, min.y, max.x, min.y, 2, lootcolor);
                drawing::draw_line(max.x, min.y, max.x, min.y + cornerSize, 2, lootcolor);
                drawing::draw_line(min.x, max.y - cornerSize, min.x, max.y, 2, lootcolor);
                drawing::draw_line(min.x, max.y, min.x + cornerSize, max.y, 2, lootcolor);
                drawing::draw_line(max.x - cornerSize, max.y, max.x, max.y, 2, lootcolor);
                drawing::draw_line(max.x, max.y - cornerSize, max.x, max.y, 2, lootcolor);

            }




            if (settings::esp::lootdistance) {

                int textcolor_idx = settings::esp::name_color_idx;
                COLORREF ccolor = settings::colors::list[textcolor_idx];
                std::string distance_text = std::format("{:.0f}m", distance / 100.0f);
                Vector2 distance_size = get_text_size(distance_text);
                drawing::draw_text(distance_text, esp_point.x - distance_size.x / 2, max.y + 5, lootcolor);
            }

  

      
        }
    }
   

}