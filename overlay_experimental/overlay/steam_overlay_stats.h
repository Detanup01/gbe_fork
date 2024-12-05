#ifndef _STEAM_OVERLAY_STATS_H_
#define _STEAM_OVERLAY_STATS_H_

#include <chrono>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "dll/settings.h"
#include "InGameOverlay/ImGui/imgui.h"


class Steam_Overlay_Stats {
private:
    class Settings* settings{};
    
    unsigned last_frametime_idx{};
    std::chrono::high_resolution_clock::time_point last_frame_timepoint =
        std::chrono::high_resolution_clock::now();
    unsigned running_frametime_ms = 0; // used for the ongoing calculation
    float active_frametime_ms = 0; // the final calculated frametime after averaging
    unsigned active_fps = 0; // the final calculated FPS after averaging


    std::chrono::high_resolution_clock::time_point initial_time =
        std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point last_playtime =
        std::chrono::high_resolution_clock::now();
    unsigned active_playtime_hr = 0;
    unsigned active_playtime_min = 0;
    unsigned active_playtime_sec = 0;

    void update_frametime(const std::chrono::high_resolution_clock::time_point &now);
    void update_playtime(const std::chrono::high_resolution_clock::time_point &now);

public:
    ImFont *font = nullptr;
    bool show_fps = false;
    bool show_frametime = false;
    bool show_playtime = false;

    Steam_Overlay_Stats(class Settings* settings);

    bool show_any_stats() const;
    void render_stats(int current_language);
};


#endif // _STEAM_OVERLAY_STATS_H_
