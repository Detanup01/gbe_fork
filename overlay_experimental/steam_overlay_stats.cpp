#include "overlay/steam_overlay_stats.h"
// translation
#include "overlay/steam_overlay_translations.h"
#include <utility>


Steam_Overlay_Stats::Steam_Overlay_Stats(class Settings* settings):
    settings(settings)
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
    running_frametime_ms += static_cast<unsigned>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame_timepoint).count()
    );
    last_frame_timepoint = now;
    if (last_frametime_idx >= (settings->overlay_fps_avg_window - 1)) {
        last_frametime_idx = 0;
        active_frametime_ms = static_cast<float>(running_frametime_ms) / settings->overlay_fps_avg_window;
        if (running_frametime_ms > 0) {
            active_fps = static_cast<unsigned>((1000 * settings->overlay_fps_avg_window) / running_frametime_ms);
        } else { // happens when avg window =1, no idea why!
            active_fps = 999;
        }
        running_frametime_ms = 0;
    } else {
        ++last_frametime_idx;
    }
}

void Steam_Overlay_Stats::update_playtime(const std::chrono::high_resolution_clock::time_point &now)
{
    const auto update_duration_sec = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_playtime
    ).count();
    if (update_duration_sec < 1) return;

    last_playtime = now;

    const auto time_duration_sec = (unsigned long long)std::chrono::duration_cast<std::chrono::seconds>(
        now - initial_time
    ).count();
    active_playtime_sec = static_cast<unsigned>(time_duration_sec % 60);

    const auto time_duration_min = time_duration_sec / 60;
    active_playtime_min = static_cast<unsigned>(time_duration_min % 60);

    const auto time_duration_hr = time_duration_min / 60;
    active_playtime_hr = static_cast<unsigned>(time_duration_hr % 24);
}

void Steam_Overlay_Stats::render_stats(int current_language)
{
    auto now = std::chrono::high_resolution_clock::now();
    if (show_fps || show_frametime) {
        update_frametime(now);
    }
    if (show_playtime) {
        update_playtime(now);
    }

    ImGui::PushFont(font);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, settings->overlay_appearance.notification_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(
        settings->overlay_appearance.stats_background_r,
        settings->overlay_appearance.stats_background_g,
        settings->overlay_appearance.stats_background_b,
        settings->overlay_appearance.stats_background_a
    ));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
        settings->overlay_appearance.stats_text_r,
        settings->overlay_appearance.stats_text_g,
        settings->overlay_appearance.stats_text_b,
        settings->overlay_appearance.stats_text_a
    ));
    
    std::stringstream stats_txt_buff{};
    if (show_fps) {
        stats_txt_buff << translationFpsDisplay[current_language]
                       << std::left << std::setw(2)
                       << active_fps
                       << std::right << std::setw(0);
    }
    if (show_frametime) {
        if (stats_txt_buff.tellp() > 0) {
            stats_txt_buff << " | ";
        }
        stats_txt_buff << translationFrametimeDisplay[current_language]
                       << std::left << std::setw(4) << std::fixed << std::setprecision(1)
                       << active_frametime_ms
                       << std::defaultfloat << std::right << std::setw(0)
                       << translationFrametimeUnitDisplay[current_language];
    }
    if (show_playtime) {
        if (stats_txt_buff.tellp() > 0) {
            stats_txt_buff << " | ";
        }
        const auto org_fill = stats_txt_buff.fill();
        stats_txt_buff << translationPlaytimeDisplay[current_language]
                       << std::setw(2) << std::setfill('0')
                       << active_playtime_hr << ':'
                       << std::setw(2) << std::setfill('0')
                       << active_playtime_min << ':'
                       << std::setw(2) << std::setfill('0')
                       << active_playtime_sec
                       << std::setw(0) << std::setfill(org_fill);
    }
    const auto stats_txt = stats_txt_buff.str();

    // set FPS box width/height based on text size
    const auto msg_box = ImGui::CalcTextSize(
        stats_txt.c_str(),
        stats_txt.c_str() + stats_txt.size()
    );
    auto &global_style = ImGui::GetStyle();
    const float padding_all_sides = global_style.WindowPadding.y + global_style.WindowPadding.x;
    const auto stats_box = ImVec2(msg_box.x + padding_all_sides, msg_box.y + padding_all_sides);
    ImGui::SetNextWindowSize(stats_box);

    auto &io = ImGui::GetIO();
    const auto anchor_point_x = stats_box.x * settings->overlay_stats_pos_x;
    const auto anchor_point_y = stats_box.y * settings->overlay_stats_pos_y;
    ImGui::SetNextWindowPos({
        io.DisplaySize.x * settings->overlay_stats_pos_x - anchor_point_x,
        io.DisplaySize.y * settings->overlay_stats_pos_y - anchor_point_y
    });

    if (ImGui::Begin("wnd_fps_frametime", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::TextWrapped("%s", stats_txt.c_str());
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::PopFont();
}
