#ifdef EMU_OVERLAY

#include "overlay/steam_overlay_stats.h"
#include "overlay/steam_overlay_translations.h"
#include <iomanip>
#include <sstream>

Steam_Overlay_Stats::Steam_Overlay_Stats(class Settings* settings) : settings(settings)
{
    show_fps = settings->overlay_always_show_fps;
    show_frametime = settings->overlay_always_show_frametime;
    show_playtime = settings->overlay_always_show_playtime;
}

bool Steam_Overlay_Stats::show_any_stats() const
{
    return show_fps || show_frametime || show_playtime;
}

void Steam_Overlay_Stats::update_frametime(const std::chrono::high_resolution_clock::time_point &now)
{
    running_frametime_ms += static_cast<unsigned>(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_timepoint).count());
    last_frame_timepoint = now;

    if (last_frametime_idx >= (settings->overlay_fps_avg_window - 1)) {
        last_frametime_idx = 0;
        active_frametime_ms = static_cast<float>(running_frametime_ms) / settings->overlay_fps_avg_window;
        active_fps = running_frametime_ms > 0 ? (1000 * settings->overlay_fps_avg_window) / running_frametime_ms : 999;
        running_frametime_ms = 0;
    } else {
        ++last_frametime_idx;
    }
}

void Steam_Overlay_Stats::update_playtime(const std::chrono::high_resolution_clock::time_point &now)
{
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_playtime).count() < 1) return;
    last_playtime = now;

    const auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(now - initial_time).count();
    active_playtime_sec = static_cast<unsigned>(total_sec % 60);
    active_playtime_min = static_cast<unsigned>((total_sec / 60) % 60);
    active_playtime_hr = static_cast<unsigned>((total_sec / 3600) % 24);
}

void Steam_Overlay_Stats::render_stats(int current_language)
{
    auto now = std::chrono::high_resolution_clock::now();
    if (show_fps || show_frametime) update_frametime(now);
    if (show_playtime) update_playtime(now);

    ImGui::PushFont(font);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, settings->overlay_appearance.notification_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(settings->overlay_appearance.stats_background_r, settings->overlay_appearance.stats_background_g, settings->overlay_appearance.stats_background_b, settings->overlay_appearance.stats_background_a));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(settings->overlay_appearance.stats_text_r, settings->overlay_appearance.stats_text_g, settings->overlay_appearance.stats_text_b, settings->overlay_appearance.stats_text_a));
    
    std::stringstream ss;
    if (show_fps) ss << translationFpsDisplay[current_language] << active_fps;
    if (show_frametime) {
        if (ss.tellp() > 0) ss << " | ";
        ss << translationFrametimeDisplay[current_language] << std::fixed << std::setprecision(1) << active_frametime_ms << translationFrametimeUnitDisplay[current_language];
    }
    if (show_playtime) {
        if (ss.tellp() > 0) ss << " | ";
        ss << translationPlaytimeDisplay[current_language] << std::setfill('0') << std::setw(2) << active_playtime_hr << ":" << std::setw(2) << active_playtime_min << ":" << std::setw(2) << active_playtime_sec;
    }
    
    const auto text = ss.str();
    const auto size = ImGui::CalcTextSize(text.c_str());
    const float padding = ImGui::GetStyle().WindowPadding.x + ImGui::GetStyle().WindowPadding.y;
    const ImVec2 box_size(size.x + padding, size.y + padding);

    ImGui::SetNextWindowSize(box_size);
    ImGui::SetNextWindowPos({ ImGui::GetIO().DisplaySize.x * settings->overlay_stats_pos_x - box_size.x * settings->overlay_stats_pos_x, ImGui::GetIO().DisplaySize.y * settings->overlay_stats_pos_y - box_size.y * settings->overlay_stats_pos_y });

    if (ImGui::Begin("stats_overlay", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::TextUnformatted(text.c_str());
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::PopFont();
}

#endif
