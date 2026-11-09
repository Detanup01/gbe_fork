#include "overlay/steam_overlay_stats.h"
// translation
#include "overlay/steam_overlay_translations.h"
#include <utility>
#include <cmath>

#if defined(_WIN32)
#include <windows.h>
#endif


// Returns true if the game window is the currently focused foreground window.
// On non-Windows platforms, always returns true (no focus-based pausing).
static bool is_game_focused()
{
#if defined(_WIN32)
    HWND fg = GetForegroundWindow();
    if (!fg) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
#else
    return true;
#endif
}


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

    // Pause accumulation when the game window loses focus (ALT+TAB)
    if (!is_game_focused() && settings->pause_session_when_unfocused) {
        if (!playtime_pause_start.has_value()) {
            playtime_pause_start = now;
        }
        return; // freeze the display values
    }

    // Accumulate paused duration when focus returns
    if (playtime_pause_start.has_value()) {
        total_playtime_paused += now - *playtime_pause_start;
        playtime_pause_start.reset();
    }

    last_playtime = now;

    const auto effective_elapsed = (now - initial_time) - total_playtime_paused;
    const auto time_duration_sec = (unsigned long long)std::chrono::duration_cast<std::chrono::seconds>(
        effective_elapsed
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

    // a compact box with a faint outline like the notifications, kept a little away from the edges of the screen
    const float fs = ImGui::GetFontSize();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, settings->overlay_appearance.notification_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(std::floor(fs * 0.6f + 0.5f), std::floor(fs * 0.5f + 0.5f)));

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
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.14f * settings->overlay_appearance.stats_background_a));

    // the numbers are drawn in the text color, the labels, units and separators around them dimmed
    std::stringstream stats_txt_buff{};
    std::vector<size_t> value_bounds{}; // where each number starts and ends in the text
    if (show_fps) {
        stats_txt_buff << translationFpsDisplay[current_language];
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
        stats_txt_buff << std::left << std::setw(2)
                       << active_fps
                       << std::right << std::setw(0);
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
    }
    if (show_frametime) {
        if (stats_txt_buff.tellp() > 0) {
            stats_txt_buff << " | ";
        }
        stats_txt_buff << translationFrametimeDisplay[current_language];
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
        stats_txt_buff << std::left << std::setw(4) << std::fixed << std::setprecision(1)
                       << active_frametime_ms
                       << std::defaultfloat << std::right << std::setw(0);
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
        stats_txt_buff << translationFrametimeUnitDisplay[current_language];
    }
    if (show_playtime) {
        if (stats_txt_buff.tellp() > 0) {
            stats_txt_buff << " | ";
        }
        const auto org_fill = stats_txt_buff.fill();
        stats_txt_buff << translationPlaytimeDisplay[current_language];
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
        stats_txt_buff << std::setw(2) << std::setfill('0')
                       << active_playtime_hr << ':'
                       << std::setw(2) << std::setfill('0')
                       << active_playtime_min << ':'
                       << std::setw(2) << std::setfill('0')
                       << active_playtime_sec
                       << std::setw(0) << std::setfill(org_fill);
        value_bounds.push_back(static_cast<size_t>(stats_txt_buff.tellp()));
    }
    const auto stats_txt = stats_txt_buff.str();
    value_bounds.push_back(stats_txt.size());

    // set FPS box width/height based on text size, measured piece by piece as it is drawn
    ImVec2 msg_box{};
    size_t piece_begin = 0;
    for (size_t piece_end : value_bounds) {
        const auto piece_box = ImGui::CalcTextSize(stats_txt.c_str() + piece_begin, stats_txt.c_str() + piece_end);
        msg_box.x += piece_box.x;
        msg_box.y = (std::max)(msg_box.y, piece_box.y);
        piece_begin = piece_end;
    }
    auto &global_style = ImGui::GetStyle();
    const auto stats_box = ImVec2(msg_box.x + global_style.WindowPadding.x * 2.0f, msg_box.y + global_style.WindowPadding.y * 2.0f);
    ImGui::SetNextWindowSize(stats_box);

    auto &io = ImGui::GetIO();
    const float margin = std::floor(fs * 0.9f + 0.5f); // the same as the overlay's top bar
    const auto anchor_point_x = stats_box.x * settings->overlay_stats_pos_x;
    const auto anchor_point_y = stats_box.y * settings->overlay_stats_pos_y;
    // a box at the top of the screen moves below the overlay's top bar while it is shown
    const float top_extra = top_inset * (1.0f - settings->overlay_stats_pos_y);
    ImGui::SetNextWindowPos({
        margin + (io.DisplaySize.x - 2.0f * margin) * settings->overlay_stats_pos_x - anchor_point_x,
        margin + top_extra + (io.DisplaySize.y - 2.0f * margin - top_extra) * settings->overlay_stats_pos_y - anchor_point_y
    });

    if (ImGui::Begin("wnd_fps_frametime", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse)) {
        const ImVec4 value_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        const ImVec4 label_color(value_color.x, value_color.y, value_color.z, value_color.w * 0.6f);
        // the pieces between the bounds are in turn a label (with the unit and the separator before it) and a number
        piece_begin = 0;
        for (size_t i = 0; i < value_bounds.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine(0.0f, 0.0f);
            }
            ImGui::PushStyleColor(ImGuiCol_Text, i % 2 == 0 ? label_color : value_color);
            ImGui::TextUnformatted(stats_txt.c_str() + piece_begin, stats_txt.c_str() + value_bounds[i]);
            ImGui::PopStyleColor();
            piece_begin = value_bounds[i];
        }
    }
    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
    ImGui::PopFont();
}
