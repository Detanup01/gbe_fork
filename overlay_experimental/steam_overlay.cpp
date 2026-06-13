#ifdef EMU_OVERLAY

#include "overlay/steam_overlay.h"

#include <atomic>
#include <thread>
#include <string>
#include <sstream>
#include <cctype>
#include <utility>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cmath>

#include "InGameOverlay/RendererDetector.h"

#include "dll/dll.h"
#include "dll/settings_parser.h"

#include "overlay/steam_overlay_translations.h"
#include "fonts/unifont.hpp"
#include "fonts/FontAwesome.hpp"
#include "fonts/FontAwesomeBrands.hpp"
#include "fonts/RawAwesome6.hpp"
#include "overlay/notification.h"

#undef BLOCK_SIZE
#include "InGameOverlay/ImGui/imgui_internal.h"

#include <curl/curl.h>
#include "json/json.hpp"
#include <fstream>
#pragma comment(lib, "version.lib")

#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif


#define URL_WINDOW_NAME "URL Window"

#define DLL_VERSION "1.0.0.2"

static constexpr int max_window_id = 10000;
static constexpr int base_notif_window_id = 0 * max_window_id;
static constexpr int base_friend_window_id = 1 * max_window_id;
static constexpr int base_friend_item_id = 2 * max_window_id;

static bool friends_pinned = false;
static bool achievements_pinned = false;
static bool settings_pinned = false;
static bool pending_close_overlay = false;
static bool history_pinned = false;
static bool g_allow_direct_join = false;

static std::unordered_set<uint64_t> g_muted_users;
static std::unordered_set<uint64_t> g_blocked_users;

static constexpr int kMaxVisibleMessageNotifications = 2;
static constexpr int kMaxVisibleInviteNotifications = 4;

static char g_broadcast_port_text[16] = "47584";
static int g_broadcast_port_last_saved = 47584;
static double g_broadcast_port_last_edit_time = 0.0;
static bool g_broadcast_port_dirty = false;
static std::atomic<bool> g_custom_broadcasts_dirty = false;

static float g_button_hover_anim_speed = 10.0f;

static std::unordered_map<ImGuiID, float> g_hover_anim;

static bool g_steam_panel_outer_open = false;
static bool g_steam_panel_inner_open = false;

static bool g_show_clock_hud = false;
static bool g_use_24h_clock = true;
static bool g_hide_hud_when_overlay_open = false;
static ImVec2 g_clock_hud_pos = ImVec2(0.0f, 0.0f);
static bool g_clock_hud_pos_initialized = false;

static constexpr const char* kBackgroundFxLabels[] = {
    "Off",
    "Snow",
    "Rain",
    "Particles",
    "Stars",
    "Bubbles"
};

static constexpr float kWindowSnapDistance = 18.0f;

enum class OverlayPresenceState {
    Online = 0,
    Idle,
    Offline,
};

static OverlayPresenceState g_overlay_presence = OverlayPresenceState::Online;
static double g_overlay_open_time = 0.0;

static const char* OverlayPresenceLabels[] = {
    "Online",
    "Idle",
    "Offline"
};

static bool OverlayPresenceAllowsPopups()
{
    return g_overlay_presence == OverlayPresenceState::Online;
}

static bool OverlayPresenceAcceptsInvites()
{
    return g_overlay_presence != OverlayPresenceState::Offline;
}

static ImVec4 OverlayPresenceColor()
{
    switch (g_overlay_presence) {
    case OverlayPresenceState::Online:  return ImVec4(0.28f, 0.84f, 0.46f, 1.0f);
    case OverlayPresenceState::Idle:    return ImVec4(0.92f, 0.72f, 0.24f, 1.0f);
    case OverlayPresenceState::Offline: return ImVec4(0.52f, 0.57f, 0.64f, 1.0f);
    }
    return ImVec4(1, 1, 1, 1);
}

static const char* OverlayPresenceIcon()
{
    switch (g_overlay_presence) {
    case OverlayPresenceState::Online:  return ICON_FA_CIRCLE;
    case OverlayPresenceState::Idle:    return ICON_FA_CLOCK;
    case OverlayPresenceState::Offline: return ICON_FA_MOON;
    }
    return ICON_FA_CIRCLE;
}

static constexpr const char* kOverlayJoinPrivacyKey = "gbe_join_locked";

static constexpr const char* valid_languages[] = {
    "english",
    "arabic",
    "bulgarian",
    "schinese",
    "tchinese",
    "czech",
    "danish",
    "dutch",
    "finnish",
    "french",
    "german",
    "greek",
    "hungarian",
    "italian",
    "japanese",
    "koreana",
    "norwegian",
    "polish",
    "portuguese",
    "brazilian",
    "romanian",
    "russian",
    "spanish",
    "latam",
    "swedish",
    "thai",
    "turkish",
    "ukrainian",
    "vietnamese",
    "croatian",
    "indonesian",
};


struct BackgroundFxSettings {
    bool enabled = true;
    int type = 0;

    int particle_count = 80;
    float particle_speed = 1.0f;
    float particle_size_min = 1.0f;
    float particle_size_max = 3.0f;
    float particle_alpha = 0.22f;

    ImVec4 particle_color = ImVec4(0.937f, 0.267f, 0.267f, 1.0f);
    float color_variation = 0.3f;

    float wind_strength = 0.0f;
    float turbulence = 0.0f;
    float swirl_intensity = 0.0f;

    float rain_length = 10.0f;
    float rain_tilt = 5.0f;

    float snow_drift = 18.0f;

    float star_twinkle_speed = 2.2f;

    bool use_optimization = true;
    int max_particles = 200;
};

static BackgroundFxSettings g_bg_fx_settings;

namespace RedAccentTheme
{
    static constexpr ImVec4 BgMain = ImVec4(0.039f, 0.039f, 0.043f, 0.985f);
    static constexpr ImVec4 BgElevated = ImVec4(0.082f, 0.082f, 0.094f, 0.985f);
    static constexpr ImVec4 BgPopup = ImVec4(0.094f, 0.094f, 0.108f, 0.985f);
    static constexpr ImVec4 BgOverlay = ImVec4(0.039f, 0.039f, 0.043f, 0.78f);

    static constexpr ImVec4 Accent = ImVec4(0.937f, 0.267f, 0.267f, 1.00f);
    static constexpr ImVec4 AccentHover = ImVec4(0.980f, 0.345f, 0.345f, 1.00f);
    static constexpr ImVec4 AccentActive = ImVec4(0.855f, 0.200f, 0.200f, 1.00f);
    static constexpr ImVec4 AccentDark = ImVec4(0.722f, 0.110f, 0.110f, 1.00f);
    static constexpr ImVec4 AccentDim = ImVec4(0.937f, 0.267f, 0.267f, 0.15f);
    static constexpr ImVec4 AccentVeryDim = ImVec4(0.937f, 0.267f, 0.267f, 0.08f);

    static constexpr ImVec4 Surface = ImVec4(0.078f, 0.078f, 0.090f, 1.00f);
    static constexpr ImVec4 SurfaceHover = ImVec4(0.118f, 0.118f, 0.137f, 1.00f);
    static constexpr ImVec4 SurfaceActive = ImVec4(0.157f, 0.157f, 0.180f, 1.00f);
    static constexpr ImVec4 SurfaceTransparent = ImVec4(0.078f, 0.078f, 0.090f, 0.60f);

    static constexpr ImVec4 Border = ImVec4(0.937f, 0.267f, 0.267f, 0.25f);
    static constexpr ImVec4 BorderHover = ImVec4(0.937f, 0.267f, 0.267f, 0.45f);
    static constexpr ImVec4 BorderActive = ImVec4(0.937f, 0.267f, 0.267f, 0.65f);
    static constexpr ImVec4 BorderSoft = ImVec4(0.937f, 0.267f, 0.267f, 0.15f);

    static constexpr ImVec4 Text = ImVec4(0.910f, 0.910f, 0.945f, 1.00f);
    static constexpr ImVec4 TextMuted = ImVec4(0.627f, 0.627f, 0.686f, 1.00f);
    static constexpr ImVec4 TextDim = ImVec4(0.502f, 0.502f, 0.549f, 1.00f);
    static constexpr ImVec4 TextBright = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);

    static constexpr ImVec4 CardShadow = ImVec4(0.000f, 0.000f, 0.000f, 0.20f);

    static constexpr float WindowRound = 14.0f;
    static constexpr float FrameRound = 10.0f;
    static constexpr float PopupRound = 12.0f;
    static constexpr float CardRound = 12.0f;
    static constexpr float ButtonRound = 12.0f;
}

static float AnimateHover(bool hovered, ImGuiID id, float speed = 12.0f) {
    float& hover_t = g_hover_anim[id];
    float dt = ImGui::GetIO().DeltaTime;
    float target = hovered ? 1.0f : 0.0f;

    float actual_speed = hovered ? speed : speed * 4.0f;

    hover_t = std::clamp(hover_t + (target - hover_t) * std::min(1.0f, dt * actual_speed), 0.0f, 1.0f);

    if (!hovered && hover_t == 0.0f) {
        g_hover_anim.erase(id);
    }

    return hover_t;
}

static std::string TrimString(const std::string& value);
static bool SteamButton(const char* label, const ImVec2& size, bool primary);
static std::string GetInitialsFromName(const std::string& name);
static ImU32 HashNameToColor(const std::string& name);

#ifdef _WIN32
static ULONG_PTR g_gdiplus_token = 0;
static bool g_gdiplus_started = false;

static bool EnsureGdiPlusStarted()
{
    if (g_gdiplus_started) return true;

    Gdiplus::GdiplusStartupInput startup_input;
    if (Gdiplus::GdiplusStartup(&g_gdiplus_token, &startup_input, nullptr) != Gdiplus::Ok) {
        return false;
    }

    g_gdiplus_started = true;
    return true;
}
#endif

static decltype(Overlay_Achievement{}.icon) g_profile_avatar_rsrc{};
static std::vector<unsigned char> g_profile_avatar_rgba;
static int g_profile_avatar_w = 0;
static int g_profile_avatar_h = 0;
static bool g_profile_avatar_dirty = true;

static std::filesystem::path g_avatar_picker_current_dir;
static std::filesystem::path g_avatar_picker_selected_file;
static bool g_open_avatar_picker_popup = false;
static Local_Storage* g_local_storage = nullptr;

static std::filesystem::path GetGoldbergSettingsPath()
{
    if (g_local_storage) {
        return std::filesystem::path(utf8_decode(g_local_storage->get_global_settings_path()));
    }

#ifdef _WIN32
    std::wstring appdata_path;
    WCHAR szPath[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
        appdata_path = szPath;
    } else {
        const wchar_t* appdata_env = _wgetenv(L"APPDATA");
        if (appdata_env && *appdata_env) {
            appdata_path = appdata_env;
        }
    }

    std::wstring folder_name = utf8_decode(Local_Storage::get_saves_folder_name());
    if (!appdata_path.empty()) {
        return std::filesystem::path(appdata_path) / folder_name / L"settings";
    }
    return std::filesystem::path(folder_name) / L"settings";
#else
    const char* appdata = std::getenv("APPDATA");
    std::string folder_name = Local_Storage::get_saves_folder_name();
    if (appdata && *appdata) {
        return std::filesystem::path(appdata) / folder_name / "settings";
    }

    const char* home = std::getenv("HOME");
    if (home && *home) {
        return std::filesystem::path(home) / ".config" / folder_name / "settings";
    }

    return std::filesystem::path(folder_name) / "settings";
#endif
}

static bool IsSupportedAvatarImageFile(const std::filesystem::path& p)
{
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });

    return ext == ".png" || ext == ".jpg" || ext == ".jpeg";
}

static std::filesystem::path FindGoldbergAvatarPath()
{
    const std::filesystem::path base = GetGoldbergSettingsPath();

    const std::filesystem::path png = base / "account_avatar.png";
    const std::filesystem::path jpg = base / "account_avatar.jpg";
    const std::filesystem::path jpeg = base / "account_avatar.jpeg";

    std::error_code ec;
    if (std::filesystem::exists(png, ec)) return png;
    if (std::filesystem::exists(jpg, ec)) return jpg;
    if (std::filesystem::exists(jpeg, ec)) return jpeg;

    return {};
}

#ifdef _WIN32
static bool LoadImageFileRGBA(const std::filesystem::path& path, std::vector<unsigned char>& out_rgba, int& out_w, int& out_h)
{
    if (!EnsureGdiPlusStarted()) return false;

    const std::wstring wide_path = utf8_decode(path.u8string());
    if (wide_path.empty()) return false;

    Gdiplus::Bitmap bitmap(wide_path.c_str());
    if (bitmap.GetLastStatus() != Gdiplus::Ok) return false;

    out_w = (int)bitmap.GetWidth();
    out_h = (int)bitmap.GetHeight();
    if (out_w <= 0 || out_h <= 0) return false;

    Gdiplus::Rect rect(0, 0, out_w, out_h);
    Gdiplus::BitmapData bmp_data{};

    if (bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bmp_data) != Gdiplus::Ok) {
        return false;
    }

    out_rgba.resize((size_t)out_w * (size_t)out_h * 4);

    for (int y = 0; y < out_h; ++y) {
        const unsigned char* src = (const unsigned char*)bmp_data.Scan0 + (size_t)y * (size_t)bmp_data.Stride;
        unsigned char* dst = out_rgba.data() + (size_t)y * (size_t)out_w * 4;

        for (int x = 0; x < out_w; ++x) {
            const unsigned char b = src[x * 4 + 0];
            const unsigned char g = src[x * 4 + 1];
            const unsigned char r = src[x * 4 + 2];
            const unsigned char a = src[x * 4 + 3];

            dst[x * 4 + 0] = r;
            dst[x * 4 + 1] = g;
            dst[x * 4 + 2] = b;
            dst[x * 4 + 3] = a;
        }
    }

    bitmap.UnlockBits(&bmp_data);
    return true;
}
#else
static bool LoadImageFileRGBA(const std::filesystem::path&, std::vector<unsigned char>&, int&, int&)
{
    return false;
}
#endif

bool Steam_Overlay::reload_profile_avatar_resource()
{
    if (!_renderer) return false;

    if (!g_profile_avatar_rsrc) {
        g_profile_avatar_rsrc = _renderer->CreateResource();
    }

    const std::filesystem::path avatar_path = FindGoldbergAvatarPath();
    if (avatar_path.empty()) {
        g_profile_avatar_rgba.clear();
        g_profile_avatar_w = 0;
        g_profile_avatar_h = 0;
        g_profile_avatar_dirty = false;
        return false;
    }

    if (!LoadImageFileRGBA(avatar_path, g_profile_avatar_rgba, g_profile_avatar_w, g_profile_avatar_h)) {
        g_profile_avatar_rgba.clear();
        g_profile_avatar_w = 0;
        g_profile_avatar_h = 0;
        g_profile_avatar_dirty = false;
        return false;
    }

    g_profile_avatar_rsrc->AttachResource((void*)g_profile_avatar_rgba.data(), g_profile_avatar_w, g_profile_avatar_h);

    g_profile_avatar_dirty = false;
    return g_profile_avatar_rsrc->GetResourceId() != 0;
}

bool Steam_Overlay::ensure_profile_avatar_loaded()
{
    if (g_profile_avatar_dirty) {
        reload_profile_avatar_resource();
    }

    return g_profile_avatar_rsrc && g_profile_avatar_rsrc->GetResourceId() != 0;
}

static bool CopySelectedAvatarToGoldberg(const std::filesystem::path& src_file)
{
    if (src_file.empty() || !IsSupportedAvatarImageFile(src_file)) return false;

    const std::filesystem::path settings_dir = GetGoldbergSettingsPath();

    std::error_code ec;
    std::filesystem::create_directories(settings_dir, ec);

    for (const char* name : { "account_avatar.png", "account_avatar.jpg", "account_avatar.jpeg" }) {
        std::filesystem::remove(settings_dir / name, ec);
    }

    const std::filesystem::path dst = settings_dir / ("account_avatar" + src_file.extension().string());
    std::filesystem::copy_file(src_file, dst, std::filesystem::copy_options::overwrite_existing, ec);

    if (ec) return false;

    g_profile_avatar_dirty = true;
    return true;
}

static void DrawProfileAvatarFallback(const std::string& name, const ImVec2& size)
{
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetCursorScreenPos();
    ImVec2 max = ImVec2(min.x + size.x, min.y + size.y);

    const float radius = (size.x < size.y ? size.x : size.y) * 0.5f;
    const ImVec2 center(min.x + size.x * 0.5f, min.y + size.y * 0.5f);

    dl->AddCircleFilled(center, radius, HashNameToColor(name));
    const std::string initials = GetInitialsFromName(name);
    const ImVec2 text_size = ImGui::CalcTextSize(initials.c_str());
    dl->AddText(
        ImVec2(center.x - text_size.x * 0.5f, center.y - text_size.y * 0.5f),
        ImGui::GetColorU32(ImVec4(1, 1, 1, 0.96f)),
        initials.c_str()
    );

    ImGui::Dummy(size);
}

static void InitializeAvatarPickerDir(Settings* settings)
{
    if (!g_avatar_picker_current_dir.empty()) return;

    if (settings && !settings->overlay_avatar_picker_default_path.empty()) {
        std::string path_str = settings->overlay_avatar_picker_default_path;
        std::filesystem::path custom_path = std::filesystem::path(utf8_decode(path_str));
        std::error_code ec;
        if (std::filesystem::exists(custom_path, ec)) {
            g_avatar_picker_current_dir = custom_path;
            return;
        }
    }

#ifdef _WIN32
    const wchar_t* userprofile = _wgetenv(L"USERPROFILE");
    if (userprofile && *userprofile) {
        g_avatar_picker_current_dir = std::filesystem::path(userprofile) / L"Pictures";
    }
    else {
        g_avatar_picker_current_dir = std::filesystem::path(L"C:\\");
    }
#else
    g_avatar_picker_current_dir = std::filesystem::current_path();
#endif

    std::error_code ec;
    if (!std::filesystem::exists(g_avatar_picker_current_dir, ec)) {
        g_avatar_picker_current_dir = std::filesystem::current_path();
    }
}

static bool DrawEditableProfileAvatar(Steam_Overlay* overlay, const std::string& display_name, float avatar_size)
{
    const ImVec2 size(avatar_size, avatar_size);
    const ImVec2 start = ImGui::GetCursorScreenPos();

    bool has_avatar = overlay && overlay->ensure_profile_avatar_loaded();

    if (has_avatar) {
        ImGui::Image(g_profile_avatar_rsrc->GetResourceId(), size);
    }
    else {
        DrawProfileAvatarFallback(display_name, size);
    }

    ImGui::SetCursorScreenPos(start);
    ImGui::InvisibleButton("##ProfileAvatarButton", size, ImGuiButtonFlags_MouseButtonLeft);

    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    if (hovered) {
        dl->AddRectFilled(
            start,
            ImVec2(start.x + size.x, start.y + size.y),
            ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, 0.30f)),
            size.x * 0.5f
        );

        const char* edit_icon = ICON_FA_PEN;
        ImVec2 icon_size = ImGui::CalcTextSize(edit_icon);
        dl->AddText(
            ImVec2(start.x + (size.x - icon_size.x) * 0.5f, start.y + (size.y - icon_size.y) * 0.5f),
            ImGui::GetColorU32(ImVec4(1, 1, 1, 0.95f)),
            edit_icon
        );

        ImGui::SetTooltip("%s", translationChangeAvatar[overlay ? overlay->GetCurrentLanguage() : 0]);
    }

    if (clicked) {
        InitializeAvatarPickerDir(overlay ? overlay->GetSettings() : nullptr);
        g_open_avatar_picker_popup = true;
    }

    return clicked;
}

static bool RenderAvatarPickerPopup(Steam_Overlay* overlay)
{
    bool changed = false;

    ImGui::SetNextWindowSize(ImVec2(760.0f, 580.0f), ImGuiCond_Appearing);

    ImGui::PushStyleColor(ImGuiCol_PopupBg, RedAccentTheme::BgPopup);
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

    bool avatar_picker_open = true;
    std::string modal_title = std::string(translationSelectAvatarImage[overlay ? overlay->GetCurrentLanguage() : 0]) + "###Select Avatar Image";
    if (ImGui::BeginPopupModal(modal_title.c_str(), &avatar_picker_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::Spacing();

        ImGui::TextWrapped("%s", g_avatar_picker_current_dir.u8string().c_str());
        ImGui::Spacing();

#ifdef _WIN32
        {
            std::vector<std::string> drive_items;
            DWORD mask = GetLogicalDrives();
            for (int i = 0; i < 26; ++i) {
                if (mask & (1 << i)) {
                    char letter = 'A' + i;
                    drive_items.push_back(std::string(1, letter) + ":\\");
                }
            }
            if (drive_items.empty()) {
                drive_items = { "C:\\" };
            }

            int current_drive_idx = -1;

            std::string current_dir_upper = g_avatar_picker_current_dir.u8string();
            std::transform(current_dir_upper.begin(), current_dir_upper.end(), current_dir_upper.begin(),
                [](unsigned char c) { return (char)std::toupper(c); });

            for (size_t i = 0; i < drive_items.size(); ++i) {
                std::string drive_upper = drive_items[i];
                std::transform(drive_upper.begin(), drive_upper.end(), drive_upper.begin(),
                    [](unsigned char c) { return (char)std::toupper(c); });

                if (current_dir_upper.rfind(drive_upper, 0) == 0) {
                    current_drive_idx = (int)i;
                    break;
                }
            }

            ImGui::SetNextItemWidth(140.0f);
            const char* current_drive_str = current_drive_idx >= 0 ? drive_items[current_drive_idx].c_str() : translationSelectDrive[overlay ? overlay->GetCurrentLanguage() : 0];
            if (ImGui::BeginCombo("##AvatarDriveSelect", current_drive_str)) {
                for (size_t i = 0; i < drive_items.size(); ++i) {
                    const bool selected = (current_drive_idx == (int)i);
                    std::error_code drive_ec;
                    if (!std::filesystem::exists(std::filesystem::path(drive_items[i]), drive_ec)) {
                        continue;
                    }

                    if (ImGui::Selectable(drive_items[i].c_str(), selected)) {
                        g_avatar_picker_current_dir = std::filesystem::path(drive_items[i]);
                        g_avatar_picker_selected_file.clear();
                    }

                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
        }
#endif

        std::string up_label = std::string(ICON_FA_ARROW_UP " ") + translationUp[overlay ? overlay->GetCurrentLanguage() : 0];
        if (SteamButton(up_label.c_str(), ImVec2(90.0f, 32.0f), false)) {
            if (g_avatar_picker_current_dir.has_parent_path()) {
                g_avatar_picker_current_dir = g_avatar_picker_current_dir.parent_path();
                g_avatar_picker_selected_file.clear();
            }
        }

        ImGui::Spacing();

        float footer_reserved_height = 140.0f;
        float file_list_height = ImGui::GetContentRegionAvail().y - footer_reserved_height;
        if (file_list_height < 140.0f) {
            file_list_height = 140.0f;
        }

        ImGui::BeginChild("AvatarPickerFiles", ImVec2(0.0f, file_list_height), true);

        std::vector<std::filesystem::directory_entry> dirs;
        std::vector<std::filesystem::directory_entry> files;

        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(g_avatar_picker_current_dir, ec)) {
            if (ec) break;

            if (entry.is_directory()) dirs.push_back(entry);
            else if (entry.is_regular_file() && IsSupportedAvatarImageFile(entry.path())) files.push_back(entry);
        }

        std::sort(dirs.begin(), dirs.end(), [](const auto& a, const auto& b) {
            return a.path().filename().u8string() < b.path().filename().u8string();
            });
        std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
            return a.path().filename().u8string() < b.path().filename().u8string();
            });

        for (const auto& dir : dirs) {
            std::string label = std::string(ICON_FA_FOLDER) + " " + dir.path().filename().u8string();
            if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0.0f, 28.0f))) {
                g_avatar_picker_current_dir = dir.path();
                g_avatar_picker_selected_file.clear();
            }
        }

        for (const auto& file : files) {
            const bool selected = (g_avatar_picker_selected_file == file.path());
            std::string label = std::string(ICON_FA_IMAGE) + " " + file.path().filename().u8string();
            if (ImGui::Selectable(label.c_str(), selected, 0, ImVec2(0.0f, 28.0f))) {
                g_avatar_picker_selected_file = file.path();
            }
        }

        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Spacing();

        if (!g_avatar_picker_selected_file.empty()) {
            ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationSelected[overlay ? overlay->GetCurrentLanguage() : 0]);
            ImGui::TextWrapped("%s", g_avatar_picker_selected_file.u8string().c_str());
        }
        else {
            ImGui::TextColored(RedAccentTheme::TextDim, "%s", translationSelectSupportedImage[overlay ? overlay->GetCurrentLanguage() : 0]);
        }

        ImGui::Spacing();

        if (!g_avatar_picker_selected_file.empty()) {
            std::string use_selected_label = std::string(ICON_FA_CHECK " ") + translationUseSelectedImage[overlay ? overlay->GetCurrentLanguage() : 0];
            if (SteamButton(use_selected_label.c_str(), ImVec2(210.0f, 36.0f), true)) {
                if (CopySelectedAvatarToGoldberg(g_avatar_picker_selected_file)) {
                    if (overlay) {
                        overlay->reload_profile_avatar_resource();
                    }
                    changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
        }

        std::string cancel_label = std::string(ICON_FA_XMARK " ") + translationCancel[overlay ? overlay->GetCurrentLanguage() : 0];
        if (SteamButton(cancel_label.c_str(), ImVec2(120.0f, 36.0f), false)) {
            ImGui::CloseCurrentPopup();
        }

        if (!avatar_picker_open) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    return changed;
}

static bool SteamButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f), bool primary = false)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));

    if (primary)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, RedAccentTheme::Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RedAccentTheme::AccentHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, RedAccentTheme::AccentActive);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 0.03f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.937f, 0.267f, 0.267f, 0.05f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.937f, 0.267f, 0.267f, 0.12f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.937f, 0.267f, 0.267f, 0.18f));
        ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
        ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Accent);
    }

    bool pressed = ImGui::Button(label, size);
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();

    ImGuiID id = ImGui::GetItemID();
    float hover_t = AnimateHover(hovered, id, 12.0f);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    float rounding = RedAccentTheme::ButtonRound;

    if (hover_t > 0.001f) {
        dl->AddRect(
            min,
            max,
            ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, 0.3f + hover_t * 0.3f)),
            rounding,
            0,
            1.5f
        );
    }

    if (held) {
        dl->AddRectFilled(
            min,
            max,
            ImGui::GetColorU32(ImVec4(0, 0, 0, 0.08f)),
            rounding
        );
    }

    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(3);
    return pressed;
}

static std::filesystem::path GetBroadcastsPath()
{
    return GetGoldbergSettingsPath() / "custom_broadcasts.txt";
}

static std::filesystem::path GetGBEConfigPath()
{
    return GetGoldbergSettingsPath() / "gbe.cfg";
}

static std::filesystem::path GetMainConfigPath()
{
    return GetGoldbergSettingsPath() / "configs.main.ini";
}

static int LoadBroadcastListenPort()
{
    const auto path = GetMainConfigPath();
    std::ifstream file(path);
    if (!file.is_open()) return 47584;

    std::string line;
    while (std::getline(file, line)) {
        line = TrimString(line);
        if (line.rfind("listen_port=", 0) == 0) {
            try {
                int port = std::stoi(line.substr(strlen("listen_port=")));
                if (port >= 1 && port <= 65535) return port;
            }
            catch (...) {}
            break;
        }
    }

    return 47584;
}

static void SaveBroadcastListenPort(int new_port)
{
    if (new_port < 1 || new_port > 65535) return;

    const auto path = GetMainConfigPath();
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ifstream in(path, std::ios::in);
    std::vector<std::string> lines;
    bool found = false;

    if (in.is_open()) {
        std::string line;
        while (std::getline(in, line)) {
            std::string trimmed = TrimString(line);
            if (trimmed.rfind("listen_port=", 0) == 0) {
                lines.emplace_back("listen_port=" + std::to_string(new_port));
                found = true;
            }
            else {
                lines.emplace_back(line);
            }
        }
        in.close();
    }

    if (!found) {
        lines.emplace_back("listen_port=" + std::to_string(new_port));
    }

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return;

    for (size_t i = 0; i < lines.size(); ++i) {
        out << lines[i];
        if (i + 1 < lines.size()) out << "\n";
    }
}

static void InitBroadcastPortEditor()
{
    int loaded_port = LoadBroadcastListenPort();
    g_broadcast_port_last_saved = loaded_port;
    snprintf(g_broadcast_port_text, sizeof(g_broadcast_port_text), "%d", loaded_port);
}

static void AutoSaveBroadcastPortIfNeeded()
{
    if (!g_broadcast_port_dirty) return;

    const double now = ImGui::GetTime();
    if ((now - g_broadcast_port_last_edit_time) < 0.65)
        return;

    try {
        int port = std::stoi(g_broadcast_port_text);
        if (port >= 1 && port <= 65535 && port != g_broadcast_port_last_saved) {
            SaveBroadcastListenPort(port);
            g_broadcast_port_last_saved = port;
        }
    }
    catch (...) {}

    g_broadcast_port_dirty = false;
}

static void EnsureBroadcastsFile(std::filesystem::path const& path)
{
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (!std::filesystem::exists(path, ec)) {
        std::ofstream create_file(path, std::ios::out);
    }
}

static std::string LoadBroadcastsText(std::filesystem::path const& path)
{
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) return {};

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

static void SaveBroadcastsText(std::filesystem::path const& path, std::string const& text)
{
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return;
    file << text;
}

static std::string TrimString(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

static std::vector<std::string> SplitLines(std::string const& text)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        auto trimmed = TrimString(line);
        if (!trimmed.empty()) {
            lines.push_back(trimmed);
        }
    }
    return lines;
}

static std::string JoinLines(std::vector<std::string> const& lines)
{
    std::ostringstream out;
    for (size_t i = 0; i < lines.size(); ++i) {
        out << lines[i];
        if (i + 1 < lines.size()) {
            out << "\n";
        }
    }
    return out.str();
}

static void ReplaceAllInPlace(std::string& s, const std::string& from, const std::string& to)
{
    if (from.empty()) return;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.length(), to);
        pos += to.length();
    }
}

static std::string ApplyEmojiShortcodes(std::string text)
{
    ReplaceAllInPlace(text, ":smile:", std::string(ICON_FA_FACE_SMILE));
    ReplaceAllInPlace(text, ":sad:", std::string(ICON_FA_FACE_SAD_TEAR));
    ReplaceAllInPlace(text, ":heart:", std::string(ICON_FA_HEART));
    ReplaceAllInPlace(text, ":fire:", std::string(ICON_FA_FIRE));
    ReplaceAllInPlace(text, ":star:", std::string(ICON_FA_STAR));
    ReplaceAllInPlace(text, ":check:", std::string(ICON_FA_CHECK));
    ReplaceAllInPlace(text, ":x:", std::string(ICON_FA_XMARK));
    ReplaceAllInPlace(text, ":skull:", std::string(ICON_FA_SKULL));
    ReplaceAllInPlace(text, ":rocket:", std::string(ICON_FA_ROCKET));
    ReplaceAllInPlace(text, ":gear:", std::string(ICON_FA_GEAR));
    return text;
}

static void SaveGBEConfig()
{
    const auto path = GetGBEConfigPath();

    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) return;

    file << "show_clock_hud=" << (g_show_clock_hud ? "1" : "0") << "\n";
    file << "use_24h_clock=" << (g_use_24h_clock ? "1" : "0") << "\n";
    file << "hide_hud_when_overlay_open=" << (g_hide_hud_when_overlay_open ? "1" : "0") << "\n";

    file << "bgfx_enabled=" << (g_bg_fx_settings.enabled ? "1" : "0") << "\n";
    file << "bgfx_type=" << g_bg_fx_settings.type << "\n";
    file << "bgfx_particle_count=" << g_bg_fx_settings.particle_count << "\n";
    file << "bgfx_particle_speed=" << g_bg_fx_settings.particle_speed << "\n";
    file << "bgfx_particle_size_min=" << g_bg_fx_settings.particle_size_min << "\n";
    file << "bgfx_particle_size_max=" << g_bg_fx_settings.particle_size_max << "\n";
    file << "bgfx_particle_alpha=" << g_bg_fx_settings.particle_alpha << "\n";
    file << "bgfx_color_variation=" << g_bg_fx_settings.color_variation << "\n";
    file << "bgfx_wind_strength=" << g_bg_fx_settings.wind_strength << "\n";
    file << "bgfx_turbulence=" << g_bg_fx_settings.turbulence << "\n";
    file << "bgfx_swirl_intensity=" << g_bg_fx_settings.swirl_intensity << "\n";
    file << "bgfx_rain_length=" << g_bg_fx_settings.rain_length << "\n";
    file << "bgfx_rain_tilt=" << g_bg_fx_settings.rain_tilt << "\n";
    file << "bgfx_snow_drift=" << g_bg_fx_settings.snow_drift << "\n";
    file << "bgfx_star_twinkle_speed=" << g_bg_fx_settings.star_twinkle_speed << "\n";

    for (uint64_t id : g_muted_users) {
        file << "muted=" << id << "\n";
    }

    for (uint64_t id : g_blocked_users) {
        file << "blocked=" << id << "\n";
    }
}

static void LoadGBEConfig(Steam_Overlay_Stats& stats)
{
    const auto path = GetGBEConfigPath();

    g_muted_users.clear();
    g_blocked_users.clear();

    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        line = TrimString(line);

        if (line.rfind("allow_direct_join=", 0) == 0) {
            std::string value = line.substr(strlen("allow_direct_join="));
            g_allow_direct_join = (value == "1");
        }
        else if (line.rfind("show_clock_hud=", 0) == 0) {
            std::string value = line.substr(strlen("show_clock_hud="));
            g_show_clock_hud = (value == "1");
        }
        else if (line.rfind("use_24h_clock=", 0) == 0) {
            std::string value = line.substr(strlen("use_24h_clock="));
            g_use_24h_clock = (value == "1");
        }
        else if (line.rfind("hide_hud_when_overlay_open=", 0) == 0) {
            std::string value = line.substr(strlen("hide_hud_when_overlay_open="));
            g_hide_hud_when_overlay_open = (value == "1");
        }
        else if (line.rfind("bgfx_enabled=", 0) == 0) {
            std::string value = line.substr(strlen("bgfx_enabled="));
            g_bg_fx_settings.enabled = (value == "1");
        }
        else if (line.rfind("bgfx_type=", 0) == 0) {
            g_bg_fx_settings.type = std::stoi(line.substr(strlen("bgfx_type=")));
        }
        else if (line.rfind("bgfx_particle_count=", 0) == 0) {
            g_bg_fx_settings.particle_count = std::stoi(line.substr(strlen("bgfx_particle_count=")));
        }
        else if (line.rfind("bgfx_particle_speed=", 0) == 0) {
            g_bg_fx_settings.particle_speed = std::stof(line.substr(strlen("bgfx_particle_speed=")));
        }
        else if (line.rfind("bgfx_particle_size_min=", 0) == 0) {
            g_bg_fx_settings.particle_size_min = std::stof(line.substr(strlen("bgfx_particle_size_min=")));
        }
        else if (line.rfind("bgfx_particle_size_max=", 0) == 0) {
            g_bg_fx_settings.particle_size_max = std::stof(line.substr(strlen("bgfx_particle_size_max=")));
        }
        else if (line.rfind("bgfx_particle_alpha=", 0) == 0) {
            g_bg_fx_settings.particle_alpha = std::stof(line.substr(strlen("bgfx_particle_alpha=")));
        }
        else if (line.rfind("bgfx_color_variation=", 0) == 0) {
            g_bg_fx_settings.color_variation = std::stof(line.substr(strlen("bgfx_color_variation=")));
        }
        else if (line.rfind("bgfx_wind_strength=", 0) == 0) {
            g_bg_fx_settings.wind_strength = std::stof(line.substr(strlen("bgfx_wind_strength=")));
        }
        else if (line.rfind("bgfx_turbulence=", 0) == 0) {
            g_bg_fx_settings.turbulence = std::stof(line.substr(strlen("bgfx_turbulence=")));
        }
        else if (line.rfind("bgfx_swirl_intensity=", 0) == 0) {
            g_bg_fx_settings.swirl_intensity = std::stof(line.substr(strlen("bgfx_swirl_intensity=")));
        }
        else if (line.rfind("bgfx_rain_length=", 0) == 0) {
            g_bg_fx_settings.rain_length = std::stof(line.substr(strlen("bgfx_rain_length=")));
        }
        else if (line.rfind("bgfx_rain_tilt=", 0) == 0) {
            g_bg_fx_settings.rain_tilt = std::stof(line.substr(strlen("bgfx_rain_tilt=")));
        }
        else if (line.rfind("bgfx_snow_drift=", 0) == 0) {
            g_bg_fx_settings.snow_drift = std::stof(line.substr(strlen("bgfx_snow_drift=")));
        }
        else if (line.rfind("bgfx_star_twinkle_speed=", 0) == 0) {
            g_bg_fx_settings.star_twinkle_speed = std::stof(line.substr(strlen("bgfx_star_twinkle_speed=")));
        }
        else if (line.rfind("muted=", 0) == 0) {
            std::string value = line.substr(strlen("muted="));
            try {
                g_muted_users.insert((uint64_t)std::stoull(value));
            }
            catch (...) {}
        }
        else if (line.rfind("blocked=", 0) == 0) {
            std::string value = line.substr(strlen("blocked="));
            try {
                g_blocked_users.insert((uint64_t)std::stoull(value));
            }
            catch (...) {}
        }
    }

    if (g_overlay_presence == OverlayPresenceState::Offline) {
        g_allow_direct_join = false;
    }
}

static bool IsUserMuted(uint64_t steam_id)
{
    return g_muted_users.find(steam_id) != g_muted_users.end();
}

static bool IsUserBlocked(uint64_t steam_id)
{
    return g_blocked_users.find(steam_id) != g_blocked_users.end();
}

static void SetDirectJoinEnabled(bool enabled)
{
    if (g_overlay_presence == OverlayPresenceState::Offline) {
        enabled = false;
    }

    g_allow_direct_join = enabled;
    SaveGBEConfig();
}

static void ApplyDirectJoinPrivacyNow()
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;
    steamFriends->SetRichPresence(kOverlayJoinPrivacyKey, g_allow_direct_join ? "0" : "1");
}

static ImVec4 GetFriendRowStatusColor(bool joinable, bool invite_only, bool has_pending_invite)
{
    if (has_pending_invite) return ImVec4(0.95f, 0.78f, 0.28f, 1.0f);
    if (joinable) return ImVec4(0.28f, 0.84f, 0.46f, 1.0f);
    if (invite_only) return ImVec4(0.95f, 0.78f, 0.28f, 1.0f);
    return ImVec4(0.52f, 0.57f, 0.64f, 1.0f);
}

static const char* GetFriendStateLabel(bool joinable, bool invite_only, bool has_pending_invite, bool same_game)
{
    if (has_pending_invite) return "INVITED";
    if (joinable) return same_game ? "JOINABLE" : "OPEN JOIN";
    if (invite_only) return "INVITE ONLY";
    if (same_game) return "IN GAME";
    return "UNAVAILABLE";
}

static const char* GetFriendPresenceLabel(const Friend& /*frd*/, const friend_window_state& /*state*/)
{
    switch (g_overlay_presence) {
    case OverlayPresenceState::Idle:    return "IDLE";
    case OverlayPresenceState::Offline: return "OFFLINE";
    case OverlayPresenceState::Online:
    default:                            return "ONLINE";
    }
}

static std::string GetInitialsFromName(const std::string& name)
{
    std::istringstream iss(name);
    std::string word;
    std::string initials;

    while (iss >> word) {
        if (!word.empty() && std::isalpha((unsigned char)word[0])) {
            initials.push_back((char)std::toupper((unsigned char)word[0]));
            if (initials.size() >= 2) break;
        }
    }

    if (initials.empty() && !name.empty()) {
        initials.push_back((char)std::toupper((unsigned char)name[0]));
    }

    return initials.empty() ? "?" : initials;
}

static ImU32 HashNameToColor(const std::string& name)
{
    uint32_t h = 2166136261u;
    for (unsigned char c : name) {
        h ^= c;
        h *= 16777619u;
    }

    float r = 0.25f + ((h & 0xFF) / 255.0f) * 0.35f;
    float g = 0.30f + (((h >> 8) & 0xFF) / 255.0f) * 0.30f;
    float b = 0.40f + (((h >> 16) & 0xFF) / 255.0f) * 0.35f;

    return ImGui::GetColorU32(ImVec4(r, g, b, 1.0f));
}

struct PopupAnimState {
    float t = 0.0f;
    double last_time = 0.0;
    bool has_pos = false;
    bool was_target_open = false;
    ImVec2 last_pos = ImVec2(0.0f, 0.0f);
};

static std::unordered_map<std::string, PopupAnimState> popup_anim_states;
static std::unordered_map<std::string, ImVec2> popup_anim_offsets;

static PopupAnimState& GetPopupAnimState(const char* id)
{
    const char* stable_id = id;
    const char* id_marker = strstr(id, "###");
    if (id_marker) {
        stable_id = id_marker + 3;
    }
    return popup_anim_states[std::string(stable_id)];
}

static bool BeginPopupWindowAnimated(const char* id, bool* p_open, ImGuiWindowFlags flags, bool force_open = false, float slide_px = 10.0f)
{
    const char* stable_id = id;
    const char* id_marker = strstr(id, "###");
    if (id_marker) {
        stable_id = id_marker + 3;
    }
    PopupAnimState& state = GetPopupAnimState(id);
    const double now = ImGui::GetTime();
    const float dt = state.last_time > 0.0 ? static_cast<float>(now - state.last_time) : 0.0f;

    const bool target_open = force_open || (p_open && *p_open);
    if (target_open && state.last_time > 0.0 && dt > 0.20f) {
        state.t = 0.0f;
        state.has_pos = false;
    }

    state.last_time = now;
    const float speed = target_open ? 10.0f : 14.0f;
    if (dt > 0.0f) {
        state.t += (target_open ? 1.0f : -1.0f) * dt * speed;
        state.t = std::clamp(state.t, 0.0f, 1.0f);
    }

    const bool should_draw = target_open || state.t > 0.0f;
    if (!should_draw) {
        state.has_pos = false;
        return false;
    }

    if (state.t == 0.0f) {
        popup_anim_offsets[std::string(stable_id)] = ImGui::GetMousePos();
    }

    ImGuiWindowFlags local_flags = flags;
    if (!target_open) {
        local_flags |= ImGuiWindowFlags_NoInputs;
    }

    if (state.t < 0.999f || !target_open) {
        if (state.has_pos) {
            ImGui::SetNextWindowPos(
                ImVec2(state.last_pos.x, state.last_pos.y + (1.0f - state.t) * slide_px),
                ImGuiCond_Always
            );
        }
        else {
            auto offset_it = popup_anim_offsets.find(std::string(stable_id));
            if (offset_it != popup_anim_offsets.end()) {
                ImGui::SetNextWindowPos(
                    ImVec2(offset_it->second.x, offset_it->second.y + (1.0f - state.t) * slide_px),
                    ImGuiCond_Appearing
                );
            }
        }
    }

    const float base_alpha = ImGui::GetStyle().Alpha;
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, base_alpha * state.t);

    bool opened = ImGui::Begin(id, nullptr, local_flags);

    if (opened) {
        state.last_pos = ImGui::GetWindowPos();
        state.has_pos = true;
    }

    ImGui::PopStyleVar();

    return opened;
}

static ImVec2 ClampWindowToScreen(const ImVec2& pos, const ImVec2& size, const ImVec2& display_size)
{
    ImVec2 out = pos;
    out.x = std::clamp(out.x, 0.0f, std::max(0.0f, display_size.x - size.x));
    out.y = std::clamp(out.y, 0.0f, std::max(0.0f, display_size.y - size.y));
    return out;
}
bool ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const float height = ImGui::GetFrameHeight();
    const float width = height * 1.70f;
    const float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));

    bool changed = false;
    if (ImGui::IsItemClicked()) {
        *v = !*v;
        ImGui::GetStateStorage()->SetFloat(ImGui::GetID(str_id), static_cast<float>(ImGui::GetTime()));
        changed = true;
    }

    const bool hovered = ImGui::IsItemHovered();

    float start_time = ImGui::GetStateStorage()->GetFloat(ImGui::GetID(str_id), 0.0f);
    float current_time = static_cast<float>(ImGui::GetTime());
    float animation_progress = (current_time - start_time) * 8.0f;

    float target_t = *v ? 1.0f : 0.0f;

    float t;
    if (start_time > 0.0f && animation_progress < 1.0f) {
        float start_t = target_t == 1.0f ? 0.0f : 1.0f;
        t = start_t + (target_t - start_t) * animation_progress;
    }
    else {
        t = target_t;
        if (start_time > 0.0f) {
            ImGui::GetStateStorage()->SetFloat(ImGui::GetID(str_id), 0.0f);
        }
    }

    t = std::clamp(t, 0.0f, 1.0f);

    ImVec4 bg = ImLerp(
        ImVec4(0.078f, 0.078f, 0.090f, 1.0f),
        RedAccentTheme::Accent,
        t
    );

    if (hovered) {
        bg.x = std::min(bg.x + 0.05f, 1.0f);
        bg.y = std::min(bg.y + 0.05f, 1.0f);
        bg.z = std::min(bg.z + 0.05f, 1.0f);
    }

    ImVec2 minp = p;
    ImVec2 maxp = ImVec2(p.x + width, p.y + height);

    draw_list->AddRectFilled(minp, maxp, ImGui::GetColorU32(bg), height * 0.5f);
    draw_list->AddRect(
        minp,
        maxp,
        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, hovered ? 0.45f : 0.20f)),
        height * 0.5f,
        0,
        1.5f
    );

    float circle_x = p.x + radius + t * (width - radius * 2.0f);
    draw_list->AddCircleFilled(
        ImVec2(circle_x, p.y + radius),
        radius - 2.0f,
        IM_COL32(245, 247, 255, 255)
    );

    return changed;
}

bool DrawSetting(const char* name, bool* value)
{
    const float toggle_width = ImGui::GetFrameHeight() * 1.55f;
    const float toggle_height = ImGui::GetFrameHeight();
    const float right_gap = 10.0f;

    std::string visible_name = name;
    size_t id_pos = visible_name.find("##");
    if (id_pos != std::string::npos) {
        visible_name = visible_name.substr(0, id_pos);
    }

    ImVec2 start_pos = ImGui::GetCursorPos();
    float content_w = ImGui::GetContentRegionAvail().x;

    ImGui::BeginGroup();

    float toggle_x = start_pos.x + content_w - toggle_width - right_gap;
    if (toggle_x < start_pos.x + 120.0f) {
        toggle_x = start_pos.x + 120.0f;
    }

    ImGui::SetCursorPos(start_pos);
    ImGui::AlignTextToFramePadding();
    ImGui::PushTextWrapPos(toggle_x - 10.0f);
    ImGui::TextUnformatted(visible_name.c_str());
    ImGui::PopTextWrapPos();

    float toggle_y = start_pos.y + (ImGui::GetTextLineHeight() - toggle_height) * 0.5f;
    if (toggle_y < start_pos.y) {
        toggle_y = start_pos.y;
    }

    ImGui::SetCursorPos(ImVec2(toggle_x, toggle_y));

    std::string toggle_id = std::string("##") + name;
    bool changed = ToggleButton(toggle_id.c_str(), value);

    ImGui::EndGroup();

    return changed;
}

bool BeginModernWindow(const char* title, ImVec2 size, bool* p_open, int /*current_language*/, bool* p_pinned = nullptr, bool /*show_nav_bar*/ = false, bool animate = false)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, RedAccentTheme::BgElevated);
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse;

    if (p_pinned && *p_pinned)
        flags |= ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);

    if (!(p_pinned && *p_pinned)) {
        const char* stable_id = title;
        const char* id_marker = strstr(title, "###");
        if (id_marker) {
            stable_id = id_marker + 3;
        }
        ImGuiWindow* existing_window = ImGui::FindWindowByName(stable_id);
        if (existing_window) {
            ImVec2 clamped_pos = ClampWindowToScreen(existing_window->Pos, existing_window->Size, ImGui::GetIO().DisplaySize);
            ImGui::SetNextWindowPos(clamped_pos, ImGuiCond_Always);
        }
    }

    bool open = animate
        ? BeginPopupWindowAnimated(title, p_open, flags, p_pinned && *p_pinned)
        : ImGui::Begin(title, p_open, flags);

    if (open) {
        const float header_height = 38.0f;
        const float btn_size = 28.0f;
        const float btn_y = 5.0f;
        const float spacing = 8.0f;
        const float title_y = 8.0f;

        ImVec2 win_pos = ImGui::GetWindowPos();
        ImVec2 win_size = ImGui::GetWindowSize();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddRectFilled(
            ImVec2(win_pos.x + 8.0f, win_pos.y + 10.0f),
            ImVec2(win_pos.x + win_size.x + 8.0f, win_pos.y + win_size.y + 10.0f),
            ImGui::GetColorU32(RedAccentTheme::CardShadow),
            RedAccentTheme::WindowRound
        );

        dl->AddRect(
            win_pos,
            ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
            ImGui::GetColorU32(RedAccentTheme::Border),
            RedAccentTheme::WindowRound,
            0,
            1.0f
        );

        if (p_pinned && *p_pinned) {
            dl->AddRect(
                win_pos,
                ImVec2(win_pos.x + win_size.x, win_pos.y + win_size.y),
                ImGui::GetColorU32(RedAccentTheme::AccentDim),
                RedAccentTheme::WindowRound,
                0,
                1.5f
            );
        }

        const char* display_title = title;
        char clean_title[256];
        const char* id_marker = strstr(title, "###");
        if (id_marker) {
            size_t len = id_marker - title;
            if (len >= sizeof(clean_title)) len = sizeof(clean_title) - 1;
            memcpy(clean_title, title, len);
            clean_title[len] = '\0';
            display_title = clean_title;
        }

        ImGui::SetCursorPos(ImVec2(14.0f, title_y));
        ImGui::TextColored(RedAccentTheme::Text, "%s", display_title);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.03f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.12f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Text);

        ImGui::SetCursorPos(ImVec2(win_size.x - btn_size - 9.0f, btn_y));
        if (ImGui::Button(ICON_FA_XMARK "##close", ImVec2(btn_size, btn_size))) {
            if (p_open) *p_open = false;
        }

        ImGui::PopStyleColor(5);

        if (p_pinned) {
            const bool is_pinned = *p_pinned;

            ImGui::PushStyleColor(ImGuiCol_Button, is_pinned ? RedAccentTheme::SurfaceActive : ImVec4(1, 1, 1, 0.03f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, is_pinned ? RedAccentTheme::AccentActive : ImVec4(1, 1, 1, 0.08f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, is_pinned ? RedAccentTheme::Accent : ImVec4(1, 1, 1, 0.12f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Text);

            ImGui::SetCursorPos(ImVec2(win_size.x - (btn_size * 2.0f) - spacing - 9.0f, btn_y));
            if (ImGui::Button(ICON_FA_THUMBTACK "##pin", ImVec2(btn_size, btn_size))) {
                *p_pinned = !*p_pinned;
            }

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", is_pinned ? "Pinned" : "Pin window");
            }

            ImGui::PopStyleColor(5);
        }

        ImGui::PopStyleVar(2);
        ImGui::SetCursorPosY(header_height + 4.0f);
    }

    return open;
}

void EndModernWindow()
{
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

static bool g_steam_panel_active = false;
static bool g_steam_panel_inner_active = false;

static bool BeginSteamPanel(
    const char* id,
    const char* title,
    const ImVec2& size = ImVec2(0, 0),
    ImGuiWindowFlags extra_flags = 0)
{
    g_steam_panel_active = false;
    g_steam_panel_inner_active = false;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::BgElevated);
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::BorderSoft);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));

    ImVec2 panel_size = size;
    if (panel_size.x == 0.0f) {
        panel_size.x = ImGui::GetContentRegionAvail().x;
    }

    const float header_height = 42.0f;
    const float body_offset_y = 50.0f;

    bool outer_opened = ImGui::BeginChild(
        id,
        panel_size,
        true,
        ImGuiWindowFlags_AlwaysUseWindowPadding |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    if (!outer_opened) {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return false;
    }

    g_steam_panel_active = true;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 pmin = ImGui::GetWindowPos();
    ImVec2 pmax = ImVec2(pmin.x + ImGui::GetWindowSize().x, pmin.y + ImGui::GetWindowSize().y);

    dl->AddRectFilled(
        ImVec2(pmin.x + 6.0f, pmin.y + 8.0f),
        ImVec2(pmax.x + 6.0f, pmax.y + 8.0f),
        ImGui::GetColorU32(ImVec4(0, 0, 0, 0.14f)),
        RedAccentTheme::CardRound
    );

    dl->AddRect(
        pmin,
        pmax,
        ImGui::GetColorU32(RedAccentTheme::BorderSoft),
        RedAccentTheme::CardRound,
        0,
        1.0f
    );

    ImGui::SetCursorPos(ImVec2(16.0f, 12.0f));
    ImGui::TextColored(RedAccentTheme::TextMuted, "%s", title);

    ImGui::SetCursorPos(ImVec2(10.0f, body_offset_y));

    std::string body_id = std::string(id) + "##Body";
    bool inner_opened = ImGui::BeginChild(
        body_id.c_str(),
        ImVec2(0.0f, 0.0f),
        false,
        ImGuiWindowFlags_AlwaysUseWindowPadding | extra_flags
    );

    if (!inner_opened) {
        ImGui::EndChild();
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        g_steam_panel_active = false;
        return false;
    }

    g_steam_panel_inner_active = true;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

    return true;
}

static void EndSteamPanel()
{
    if (g_steam_panel_inner_active) {
        ImGui::EndChild();
        g_steam_panel_inner_active = false;
    }

    if (g_steam_panel_active) {
        ImGui::EndChild();
        g_steam_panel_active = false;
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
}

static bool g_flat_panel_active = false;

static bool BeginFlatPanel(const char* id, const ImVec2& size = ImVec2(0, 0))
{
    g_flat_panel_active = false;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::SurfaceTransparent);
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::BorderSoft);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));

    ImVec2 panel_size = size;
    if (panel_size.x == 0.0f) {
        panel_size.x = ImGui::GetContentRegionAvail().x;
    }

    bool opened = ImGui::BeginChild(
        id,
        panel_size,
        true,
        ImGuiWindowFlags_AlwaysUseWindowPadding |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    if (!opened) {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return false;
    }

    g_flat_panel_active = true;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetWindowPos();
    ImVec2 max = ImVec2(min.x + ImGui::GetWindowSize().x, min.y + ImGui::GetWindowSize().y);

    dl->AddRect(
        min,
        max,
        ImGui::GetColorU32(RedAccentTheme::BorderSoft),
        12.0f,
        0,
        1.0f
    );

    return true;
}

static void EndFlatPanel()
{
    if (g_flat_panel_active) {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        g_flat_panel_active = false;
    }
}

static void DrawRadialBackground(ImDrawList* dl, const ImVec2& size)
{
    ImVec2 center = ImVec2(size.x * 0.3f, size.y * 0.4f);
    float max_radius = std::max(size.x, size.y) * 1.2f;

    const int num_steps = 32;
    for (int i = 0; i < num_steps; i++) {
        float t = (float)i / (float)num_steps;
        float radius = max_radius * (1.0f - t);

        ImColor color_start = ImColor(26, 26, 26, 255);
        ImColor color_end = ImColor(10, 10, 10, 255);

        ImColor color;
        color.Value.x = color_start.Value.x * (1.0f - t) + color_end.Value.x * t;
        color.Value.y = color_start.Value.y * (1.0f - t) + color_end.Value.y * t;
        color.Value.z = color_start.Value.z * (1.0f - t) + color_end.Value.z * t;

        dl->AddCircleFilled(center, radius, color, 64);
    }
}

static void DrawRedGlowEffect(ImDrawList* dl, const ImVec2& size, float time)
{
    float pulse = (sin(time * 2.0f) + 1.0f) * 0.5f;
    float alpha = 0.03f + pulse * 0.04f;

    dl->AddRectFilledMultiColor(
        ImVec2(0, 0),
        size,
        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, alpha)),
        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, alpha * 0.6f)),
        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, 0)),
        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, alpha))
    );
}

static void DrawOverlayBackgroundFx(ImDrawList* dl, const ImVec2& display_size, float overlay_alpha)
{
    if (!dl) return;
    if (overlay_alpha <= 0.001f) return;
    if (!g_bg_fx_settings.enabled) return;

    const float t = (float)ImGui::GetTime();
    const int count = g_bg_fx_settings.particle_count;

    const float speed_factor = g_bg_fx_settings.particle_speed;
    const float wind = g_bg_fx_settings.wind_strength;
    const float turbulence = g_bg_fx_settings.turbulence;
    const float swirl = g_bg_fx_settings.swirl_intensity;

    for (int i = 0; i < count; ++i) {
        float seed = (float)i * 17.371f;

        float offset_x = wind * t * 0.5f;
        float offset_y = turbulence * std::sin(t * 0.5f + seed) * 20.0f;

        if (swirl > 0.0f) {
            float angle = t * swirl * 0.5f + seed;
            float radius = (float)(i % 100) * 0.05f;
            offset_x += std::cos(angle) * radius * display_size.x * 0.3f;
            offset_y += std::sin(angle) * radius * display_size.y * 0.3f;
        }

        float x = std::fmod(seed * 53.0f + t * (8.0f + (i % 7) * speed_factor) + offset_x, display_size.x + 80.0f) - 40.0f;
        float y = std::fmod(seed * 97.0f + t * (18.0f + (i % 11) * 3.0f * speed_factor) + offset_y, display_size.y + 120.0f) - 60.0f;

        ImVec4 color = g_bg_fx_settings.particle_color;
        if (g_bg_fx_settings.color_variation > 0.0f) {
            float variation = (std::sin(seed * 3.14159f) * g_bg_fx_settings.color_variation);
            color.x = std::clamp(color.x + variation, 0.0f, 1.0f);
            color.y = std::clamp(color.y + variation * 0.8f, 0.0f, 1.0f);
            color.z = std::clamp(color.z + variation * 0.6f, 0.0f, 1.0f);
        }

        color.w = g_bg_fx_settings.particle_alpha * overlay_alpha;

        switch (g_bg_fx_settings.type) {
        case 0: {
            float radius = g_bg_fx_settings.particle_size_min + (float)(i % 10) / 10.0f * (g_bg_fx_settings.particle_size_max - g_bg_fx_settings.particle_size_min);
            float drift = std::sin(t * 0.7f + seed) * g_bg_fx_settings.snow_drift;
            dl->AddCircleFilled(
                ImVec2(x + drift, y),
                radius,
                ImGui::GetColorU32(color)
            );
            break;
        }
        case 1: {
            float len = g_bg_fx_settings.rain_length + (float)(i % 5) * 3.0f;
            float tilt = g_bg_fx_settings.rain_tilt;
            float rain_x = std::fmod(seed * 67.0f + t * (220.0f + (i % 9) * 15.0f * speed_factor), display_size.x + 140.0f) - 70.0f;
            float rain_y = std::fmod(seed * 31.0f + t * (340.0f + (i % 13) * 18.0f * speed_factor), display_size.y + 180.0f) - 90.0f;
            dl->AddLine(
                ImVec2(rain_x, rain_y),
                ImVec2(rain_x - tilt, rain_y + len),
                ImGui::GetColorU32(color),
                1.0f
            );
            break;
        }
        case 2: {
            float radius = g_bg_fx_settings.particle_size_min + (float)(i % 3) * (g_bg_fx_settings.particle_size_max - g_bg_fx_settings.particle_size_min) / 3.0f;
            float px = x + std::sin(t * 0.8f + seed * 0.25f) * 22.0f;
            float py = y + std::cos(t * 0.6f + seed * 0.18f) * 16.0f;
            dl->AddCircleFilled(
                ImVec2(px, py),
                radius,
                ImGui::GetColorU32(color)
            );
            break;
        }
        case 3: {
            float star_x = std::fmod(seed * 73.0f + t * (52.0f + (i % 5) * 6.0f * speed_factor), display_size.x + 100.0f) - 50.0f;
            float star_y = std::fmod(seed * 43.0f + t * (92.0f + (i % 7) * 7.0f * speed_factor), display_size.y + 140.0f) - 70.0f;
            float a = 0.10f + 0.12f * (0.5f + 0.5f * std::sin(t * g_bg_fx_settings.star_twinkle_speed + seed));
            ImU32 col = ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, a * overlay_alpha));
            dl->AddLine(ImVec2(star_x - 3.0f, star_y), ImVec2(star_x + 3.0f, star_y), col, 1.0f);
            dl->AddLine(ImVec2(star_x, star_y - 3.0f), ImVec2(star_x, star_y + 3.0f), col, 1.0f);
            break;
        }
        case 4: {
            float bubble_x = x + std::sin(seed + t * 0.9f) * 10.0f;
            float bubble_y = display_size.y - std::fmod(seed * 59.0f + t * (40.0f + (i % 6) * 5.0f * speed_factor), display_size.y + 120.0f) + 60.0f;
            float radius = g_bg_fx_settings.particle_size_min + (float)(i % 4) * (g_bg_fx_settings.particle_size_max - g_bg_fx_settings.particle_size_min) / 4.0f;
            dl->AddCircle(
                ImVec2(bubble_x, bubble_y),
                radius,
                ImGui::GetColorU32(color),
                0,
                1.0f
            );
            break;
        }
        }
    }
}

static void RenderClockHUD(bool overlay_open)
{
    if (!g_show_clock_hud) return;

    if (g_hide_hud_when_overlay_open && overlay_open) return;

    std::time_t now = std::time(nullptr);
    std::tm local_tm{};

#ifdef _WIN32
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    char time_buf[32]{};
    if (g_use_24h_clock) {
        std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &local_tm);
    }
    else {
        std::strftime(time_buf, sizeof(time_buf), "%I:%M:%S %p", &local_tm);
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 text_size = ImGui::CalcTextSize(time_buf);
    ImVec2 padding(12.0f, 8.0f);
    ImVec2 window_size(text_size.x + padding.x * 2.0f, text_size.y + padding.y * 2.0f);

    if (!g_clock_hud_pos_initialized) {
        g_clock_hud_pos = ImVec2(io.DisplaySize.x - window_size.x - 18.0f, 18.0f);
        g_clock_hud_pos_initialized = true;
    }

    if (g_clock_hud_pos.x > io.DisplaySize.x || g_clock_hud_pos.y > io.DisplaySize.y) {
        g_clock_hud_pos = ImVec2(io.DisplaySize.x - window_size.x - 18.0f, 18.0f);
    }

    g_clock_hud_pos = ClampWindowToScreen(g_clock_hud_pos, window_size, io.DisplaySize);

    if (!overlay_open) {
        ImGui::SetNextWindowPos(g_clock_hud_pos, ImGuiCond_Always);
    }
    else {
        ImGui::SetNextWindowPos(g_clock_hud_pos, ImGuiCond_FirstUseEver);
    }

    ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.15f, 0.88f));
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
    ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Text);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (!overlay_open) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
    }

    bool opened = ImGui::Begin("##ClockHUDWidget", nullptr, flags);

    if (opened) {
        ImGui::TextUnformatted(time_buf);

        if (overlay_open) {
            if (ImGui::IsWindowHovered() || ImGui::IsWindowFocused()) {
                g_clock_hud_pos = ImGui::GetWindowPos();
            }
        }
    }

    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

void Steam_Overlay::overlay_run_callback(void* object)
{
    Steam_Overlay* _this = reinterpret_cast<Steam_Overlay*>(object);
    _this->steam_run_callback();
}

void Steam_Overlay::overlay_networking_callback(void* object, Common_Message* msg)
{
    Steam_Overlay* _this = reinterpret_cast<Steam_Overlay*>(object);
    _this->networking_msg_received(msg);
}

void Steam_Overlay::parse_key_combo()
{
    static const std::unordered_map<InGameOverlay::ToggleKey, std::string_view> KEYS_MAP{
        { InGameOverlay::ToggleKey::SHIFT, "shift" },
        { InGameOverlay::ToggleKey::CTRL,  "ctrl"  },
        { InGameOverlay::ToggleKey::ALT,   "alt"   },
        { InGameOverlay::ToggleKey::TAB,   "tab"   },
        { InGameOverlay::ToggleKey::F1,    "fn1"   },
        { InGameOverlay::ToggleKey::F2,    "fn2"   },
        { InGameOverlay::ToggleKey::F3,    "fn3"   },
        { InGameOverlay::ToggleKey::F4,    "fn4"   },
        { InGameOverlay::ToggleKey::F5,    "fn5"   },
        { InGameOverlay::ToggleKey::F6,    "fn6"   },
        { InGameOverlay::ToggleKey::F7,    "fn7"   },
        { InGameOverlay::ToggleKey::F8,    "fn8"   },
        { InGameOverlay::ToggleKey::F9,    "fn9"   },
        { InGameOverlay::ToggleKey::F10,   "fn10"  },
        { InGameOverlay::ToggleKey::F11,   "fn11"  },
        { InGameOverlay::ToggleKey::F12,   "fn12"  },
    };

    std::unordered_set<InGameOverlay::ToggleKey> keys_combo{};
    bool use_default = false;
    if (settings->overlay_toggle_keys.empty()) {
        use_default = true;
    }
    else {
        for (const auto& key_name : settings->overlay_toggle_keys) {
            auto key_it = std::find_if(KEYS_MAP.cbegin(), KEYS_MAP.cend(), [&key_name](decltype(*KEYS_MAP.cbegin()) const& item) {
                return common_helpers::str_cmp_insensitive(item.second, key_name);
                });
            if (KEYS_MAP.cend() != key_it) {
                keys_combo.insert(key_it->first);
            }
            else {
                use_default = true;
                PRINT_DEBUG("[X] Unknown key '%s', using default key combo Shift + Tab", key_name.c_str());
                break;
            }
        }
    }

    if (use_default) {
        toggle_keys = {
            InGameOverlay::ToggleKey::SHIFT, InGameOverlay::ToggleKey::TAB
        };
    }
    else {
        toggle_keys = std::vector<InGameOverlay::ToggleKey>(keys_combo.begin(), keys_combo.end());
    }
}

Steam_Overlay::Steam_Overlay(Settings* settings, Local_Storage* local_storage, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking* network) :
    settings(settings),
    local_storage(local_storage),
    callback_results(callback_results),
    callbacks(callbacks),
    run_every_runcb(run_every_runcb),
    network(network),
    stats(Steam_Overlay_Stats(settings))
{
    g_local_storage = local_storage;
    if (settings->disable_overlay) return;

    renderer_hook_init_thread = common_helpers::KillableWorker(
        [this](void*) { return renderer_hook_proc(); },
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(renderer_detector_polling_ms),
        [this] { return !setup_overlay_called; }
    );

    renderer_detector_delay_thread = common_helpers::KillableWorker(
        [this](void*) {
            request_renderer_detector();
            set_renderer_hook_timeout();
            renderer_hook_init_thread.start();
            return true;
        },
        std::chrono::milliseconds(settings->overlay_hook_delay_sec * 1000),
        std::chrono::milliseconds(0),
        [this] { return !setup_overlay_called; }
    );

    parse_key_combo();
    LoadGBEConfig(stats);
    InitBroadcastPortEditor();
    strncpy(username_text, settings->get_local_name(), sizeof(username_text));
    username_text[sizeof(username_text) - 1] = '\0';

    this->warn_local_save =
        !settings->disable_overlay_warning_any && !settings->disable_overlay_warning_local_save && settings->overlay_warn_local_save;
    this->warn_bad_appid =
        !settings->disable_overlay_warning_any && !settings->disable_overlay_warning_bad_appid && settings->get_local_game_id().AppID() == 0;

    current_language = 0;
    const char* language = settings->get_language();

    show_user_info = settings->overlay_always_show_user_info;

    int i = 0;
    for (auto& lang : valid_languages) {
        if (common_helpers::str_cmp_insensitive(lang, language)) {
            current_language = i;
            break;
        }

        ++i;
    }
    selected_language = current_language;

    this->network->setCallback(CALLBACK_ID_STEAM_MESSAGES, settings->get_local_steam_id(), &Steam_Overlay::overlay_networking_callback, this);
    this->run_every_runcb->add(&Steam_Overlay::overlay_run_callback, this);
}

Steam_Overlay::~Steam_Overlay()
{
    if (settings->disable_overlay) return;

    UnSetupOverlay();

    this->network->rmCallback(CALLBACK_ID_STEAM_MESSAGES, settings->get_local_steam_id(), &Steam_Overlay::overlay_networking_callback, this);
    this->run_every_runcb->remove(&Steam_Overlay::overlay_run_callback, this);
}

void Steam_Overlay::request_renderer_detector()
{
    PRINT_DEBUG_ENTRY();
    future_renderer = InGameOverlay::DetectRenderer();
}

void Steam_Overlay::set_renderer_hook_timeout()
{
    renderer_hook_timeout_ctr = settings->overlay_renderer_detector_timeout_sec * 1000 / renderer_detector_polling_ms;
}

void Steam_Overlay::cleanup_renderer_hook()
{
    InGameOverlay::StopRendererDetection();
    InGameOverlay::FreeDetector();
}

bool Steam_Overlay::renderer_hook_proc()
{
    if (renderer_hook_timeout_ctr > 0 && future_renderer.wait_for(std::chrono::milliseconds(renderer_detector_polling_ms)) != std::future_status::ready) {
        return false;
    }

    cleanup_renderer_hook();
    bool final_chance = future_renderer.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready;
    if (!setup_overlay_called || !final_chance || renderer_hook_timeout_ctr <= 0) {
        PRINT_DEBUG("failed to detect renderer, ctr=%i, overlay was set up=%i",
            renderer_hook_timeout_ctr, (int)setup_overlay_called
        );
        return true;
    }

    _renderer = future_renderer.get();
    if (!_renderer) {
        PRINT_DEBUG("renderer hook was null!");
        return true;
    }
    PRINT_DEBUG("got renderer hook %p for '%s'", _renderer, _renderer->GetLibraryName());

    load_achievements_data();
    load_audio();
    create_fonts();

    auto overlay_toggle_callback = [this]() { open_overlay_hook(true); };
    _renderer->OverlayProc = [this]() { overlay_render_proc(); };
    _renderer->OverlayHookReady = [this](InGameOverlay::OverlayHookState state) {
        PRINT_DEBUG("hook state changed to <%i>", (int)state);
        overlay_state_hook(state == InGameOverlay::OverlayHookState::Ready || state == InGameOverlay::OverlayHookState::Reset);
        };

    bool started = _renderer->StartHook(overlay_toggle_callback, toggle_keys.data(), (int)toggle_keys.size(), &fonts_atlas);
    PRINT_DEBUG("started renderer hook (result=%i)", (int)started);

    return true;
}

void Steam_Overlay::create_fonts()
{
    PRINT_DEBUG_ENTRY();

    fonts_atlas.Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;

    float font_size = settings->overlay_appearance.font_size;
    float font_size_fps = settings->overlay_appearance.font_size_fps > 0.0f
        ? settings->overlay_appearance.font_size_fps
        : font_size;
    float font_size_ach_title = settings->overlay_appearance.font_size_ach_title > 0.0f
        ? settings->overlay_appearance.font_size_ach_title
        : font_size;
    float font_size_ach_desc = settings->overlay_appearance.font_size_ach_desc > 0.0f
        ? settings->overlay_appearance.font_size_ach_desc
        : font_size;

    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.PixelSnapH = true;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.SizePixels = font_size;
#if IMGUI_VERSION_NUM >= 19140
    font_cfg.GlyphExtraAdvanceX = settings->overlay_appearance.font_glyph_extra_spacing_x;
#else
    font_cfg.GlyphExtraSpacing.x = settings->overlay_appearance.font_glyph_extra_spacing_x;
#endif

    for (const auto& ach : achievements) {
        font_builder.AddText(ach.title.c_str());
        font_builder.AddText(ach.description.c_str());
    }
    for (int i = 0; i < TRANSLATION_NUMBER_OF_LANGUAGES; i++) {
        font_builder.AddText(translationChat[i]);
        font_builder.AddText(translationCopyId[i]);
        font_builder.AddText(translationTestAchievement[i]);
        font_builder.AddText(translationInvite[i]);
        font_builder.AddText(translationInviteAll[i]);
        font_builder.AddText(translationJoin[i]);
        font_builder.AddText(translationInvitedYouToJoinTheGame[i]);
        font_builder.AddText(translationAccept[i]);
        font_builder.AddText(translationRefuse[i]);
        font_builder.AddText(translationSend[i]);
        font_builder.AddText(translationUserPlaying[i]);
        font_builder.AddText(translationRenderer[i]);
        font_builder.AddText(translationShowAchievements[i]);
        font_builder.AddText(translationSettings[i]);
        font_builder.AddText(translationFriends[i]);
        font_builder.AddText(translationAchievementWindow[i]);
        font_builder.AddText(translationListOfAchievements[i]);
        font_builder.AddText(translationAchievements[i]);
        font_builder.AddText(translationHiddenAchievement[i]);
        font_builder.AddText(translationAchievedOn[i]);
        font_builder.AddText(translationNotAchieved[i]);
        font_builder.AddText(translationGlobalSettingsWindow[i]);
        font_builder.AddText(translationGlobalSettingsWindowDescription[i]);
        font_builder.AddText(translationUsername[i]);
        font_builder.AddText(translationLanguage[i]);
        font_builder.AddText(translationSelectedLanguage[i]);
        font_builder.AddText(translationRestartTheGameToApply[i]);
        font_builder.AddText(translationSave[i]);
        font_builder.AddText(translationWarning[i]);
        font_builder.AddText(translationWarningDescription_badAppid[i]);
        font_builder.AddText(translationWarningDescription_localSave[i]);
        font_builder.AddText(translationSteamOverlayURL[i]);
        font_builder.AddText(translationClose[i]);
        font_builder.AddText(translationPlaying[i]);
        font_builder.AddText(translationAutoAcceptFriendInvite[i]);
        font_builder.AddText(translationFpsCheckbox[i]);
        font_builder.AddText(translationFpsDisplay[i]);
        font_builder.AddText(translationFrametimeCheckbox[i]);
        font_builder.AddText(translationFrametimeDisplay[i]);
        font_builder.AddText(translationFrametimeUnitDisplay[i]);
        font_builder.AddText(translationPlaytimeCheckbox[i]);
        font_builder.AddText(translationPlaytimeDisplay[i]);
        font_builder.AddText(translationSelectAvatarImage[i]);
        font_builder.AddText(translationSelectDrive[i]);
        font_builder.AddText(translationUp[i]);
        font_builder.AddText(translationSelected[i]);
        font_builder.AddText(translationSelectSupportedImage[i]);
        font_builder.AddText(translationUseSelectedImage[i]);
        font_builder.AddText(translationCancel[i]);
        font_builder.AddText(translationDirectJoinForcedOffOffline[i]);
        font_builder.AddText(translationDirectJoinFriendsCanJoin[i]);
        font_builder.AddText(translationDirectJoinHidden[i]);
        font_builder.AddText(translation24HourFormat[i]);
        font_builder.AddText(translationActiveEffect[i]);
        font_builder.AddText(translationAddIp[i]);
        font_builder.AddText(translationAddIpLabel[i]);
        font_builder.AddText(translationBackgroundFx[i]);
        font_builder.AddText(translationBroadcastListenPort[i]);
        font_builder.AddText(translationBroadcasts[i]);
        font_builder.AddText(translationCategories[i]);
        font_builder.AddText(translationChangeAvatar[i]);
        font_builder.AddText(translationColorSettings[i]);
        font_builder.AddText(translationColorVariation[i]);
        font_builder.AddText(translationCurrentList[i]);
        font_builder.AddText(translationDirectJoinDisabled[i]);
        font_builder.AddText(translationDirectJoinEnabled[i]);
        font_builder.AddText(translationEffectType[i]);
        font_builder.AddText(translationEnableBgEffects[i]);
        font_builder.AddText(translationFriendsAndProfile[i]);
        font_builder.AddText(translationFxBubbles[i]);
        font_builder.AddText(translationFxParticles[i]);
        font_builder.AddText(translationFxRain[i]);
        font_builder.AddText(translationFxSnow[i]);
        font_builder.AddText(translationFxStars[i]);
        font_builder.AddText(translationGbePort[i]);
        font_builder.AddText(translationGeneral[i]);
        font_builder.AddText(translationHideHudWhenOverlayOpen[i]);
        font_builder.AddText(translationHudElements[i]);
        font_builder.AddText(translationMaxParticlesPerformance[i]);
        font_builder.AddText(translationMaxSize[i]);
        font_builder.AddText(translationMinSize[i]);
        font_builder.AddText(translationMovementSettings[i]);
        font_builder.AddText(translationNoAchievementsAvailable[i]);
        font_builder.AddText(translationNotes[i]);
        font_builder.AddText(translationNotificationHistory[i]);
        font_builder.AddText(translationOptimizePerformance[i]);
        font_builder.AddText(translationOverlayStats[i]);
        font_builder.AddText(translationParticleAlpha[i]);
        font_builder.AddText(translationParticleColor[i]);
        font_builder.AddText(translationParticleCount[i]);
        font_builder.AddText(translationParticleSettings[i]);
        font_builder.AddText(translationParticleSpeed[i]);
        font_builder.AddText(translationParticlesCount[i]);
        font_builder.AddText(translationPerformance[i]);
        font_builder.AddText(translationPreviewInfo[i]);
        font_builder.AddText(translationRainLength[i]);
        font_builder.AddText(translationRainSettings[i]);
        font_builder.AddText(translationRainTilt[i]);
        font_builder.AddText(translationRemoveAll[i]);
        font_builder.AddText(translationRemoveSelected[i]);
        font_builder.AddText(translationResetToDefaults[i]);
        font_builder.AddText(translationSnowDrift[i]);
        font_builder.AddText(translationSnowSettings[i]);
        font_builder.AddText(translationStarSettings[i]);
        font_builder.AddText(translationStarTwinkleSpeed[i]);
        font_builder.AddText(translationSwirlIntensity[i]);
        font_builder.AddText(translationSystemClock[i]);
        font_builder.AddText(translationTurbulence[i]);
        font_builder.AddText(translationWindStrength[i]);
    }
    font_builder.AddRanges(fonts_atlas.GetGlyphRangesDefault());

    font_builder.BuildRanges(&ranges);
    font_cfg.GlyphRanges = ranges.Data;

    auto add_overlay_font = [this](float size, const std::string &custom_font = "") {
        font_cfg.SizePixels = size;
        font_cfg.MergeMode = false;

        const std::string &font_path = custom_font.empty() ? settings->overlay_appearance.font_override : custom_font;
        ImFont *font = nullptr;
        if (font_path.size()) {
            font = fonts_atlas.AddFontFromFileTTF(font_path.c_str(), size, &font_cfg);
            if (font) {
                font_cfg.MergeMode = true; // merge next font into the custom font
            }
        }

        ImFont *fallback_font = fonts_atlas.AddFontFromMemoryCompressedTTF(unifont_compressed_data, unifont_compressed_size, size, &font_cfg);
        
        font_cfg.MergeMode = true;
        font_cfg.PixelSnapH = true;
        static const ImWchar icon_ranges[] = { 0xe000, 0xf8ff, 0 };

        fonts_atlas.AddFontFromMemoryCompressedTTF(
            FontAwesome6Solid_compressed_data,
            FontAwesome6Solid_compressed_size,
            size,
            &font_cfg,
            icon_ranges
        );

        return font ? font : fallback_font;
    };

    font_notif = font_default = add_overlay_font(font_size);
    font_fps = add_overlay_font(font_size_fps);
    font_ach_title = add_overlay_font(font_size_ach_title, settings->overlay_appearance.font_override_ach_title);
    font_ach_desc = add_overlay_font(font_size_ach_desc, settings->overlay_appearance.font_override_ach_desc);
    stats.font = font_fps;

    reset_LastError();
}

void Steam_Overlay::load_audio()
{
    PRINT_DEBUG_ENTRY();

    for (auto& kv : wav_files) {
        std::string file_path{};
        unsigned int file_size{};

        for (const auto& settings_path : { Local_Storage::get_game_settings_path(), local_storage->get_global_settings_path() }) {
            file_path = settings_path + Steam_Overlay::ACH_SOUNDS_FOLDER + PATH_SEPARATOR + kv.first;
            file_size = file_size_(file_path);
            if (file_size) break;
        }

        kv.second.clear();
        if (file_size) {
            kv.second.assign(file_size + 1, 0);
            int read = Local_Storage::get_file_data(file_path, (char*)&kv.second[0], file_size);
            if (read <= 0) kv.second.clear();
            PRINT_DEBUG("loaded '%s' (read %i/%u bytes)", file_path.c_str(), read, file_size);
        }
    }
}

void Steam_Overlay::load_achievements_data()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_User_Stats* steamUserStats = get_steam_client()->steam_user_stats;
    uint32 achievements_num = steamUserStats->GetNumAchievements();
    for (uint32 i = 0; i < achievements_num; ++i) {
        Overlay_Achievement ach{};
        ach.name = steamUserStats->GetAchievementName(i);
        ach.title = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "name");
        ach.description = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "desc");

        const char* hidden = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "hidden");
        ach.hidden = hidden && hidden[0] == '1';

        bool achieved = false;
        uint32 unlock_time = 0;
        if (steamUserStats->GetAchievementAndUnlockTime(ach.name.c_str(), &achieved, &unlock_time)) {
            ach.achieved = achieved;
            ach.unlock_time = unlock_time;
        }
        else {
            ach.achieved = false;
            ach.unlock_time = 0;
        }

        float pnMinProgress = 0, pnMaxProgress = 0;
        if (steamUserStats->GetAchievementProgressLimits(ach.name.c_str(), &pnMinProgress, &pnMaxProgress)) {
            ach.progress = (uint32)pnMinProgress;
            ach.max_progress = (uint32)pnMaxProgress;
        }

        if (ach.icon == nullptr) {
            ach.icon = _renderer->CreateResource();
        }
        if (ach.icon_gray == nullptr) {
            ach.icon_gray = _renderer->CreateResource();
        }

        achievements.emplace_back(ach);

        if (!setup_overlay_called) return;
    }

    PRINT_DEBUG("count=%u, loaded=%zu", achievements_num, achievements.size());

}

void Steam_Overlay::overlay_state_hook(bool ready)
{
    PRINT_DEBUG("%i", (int)ready);

    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!setup_overlay_called) return;

    is_ready = ready;

    if (ready) {
        bool not_yet = false;
        if (ImGui::GetCurrentContext() && late_init_imgui.compare_exchange_weak(not_yet, true)) {
            PRINT_DEBUG("late init ImGui");

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            style.WindowRounding = RedAccentTheme::WindowRound;
            style.ChildRounding = RedAccentTheme::CardRound;
            style.FrameRounding = RedAccentTheme::FrameRound;
            style.PopupRounding = 8.0f;
            style.ScrollbarRounding = 999.0f;
            style.GrabRounding = 999.0f;
            style.TabRounding = RedAccentTheme::ButtonRound;

            style.WindowPadding = ImVec2(12.0f, 12.0f);
            style.FramePadding = ImVec2(10.0f, 7.0f);
            style.ItemSpacing = ImVec2(8.0f, 8.0f);
            style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
            style.ScrollbarSize = 11.0f;

            style.WindowBorderSize = 1.2f;
            style.ChildBorderSize = 1.2f;
            style.FrameBorderSize = 1.2f;
            style.PopupBorderSize = 1.2f;

            colors[ImGuiCol_WindowBg] = RedAccentTheme::BgMain;
            colors[ImGuiCol_ChildBg] = RedAccentTheme::BgElevated;
            colors[ImGuiCol_PopupBg] = RedAccentTheme::BgPopup;

            colors[ImGuiCol_Border] = RedAccentTheme::Border;
            colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

            colors[ImGuiCol_Text] = RedAccentTheme::Text;
            colors[ImGuiCol_TextDisabled] = RedAccentTheme::TextDim;

            colors[ImGuiCol_TitleBg] = RedAccentTheme::BgMain;
            colors[ImGuiCol_TitleBgActive] = RedAccentTheme::BgElevated;
            colors[ImGuiCol_TitleBgCollapsed] = RedAccentTheme::BgMain;

            colors[ImGuiCol_FrameBg] = RedAccentTheme::Surface;
            colors[ImGuiCol_FrameBgHovered] = RedAccentTheme::SurfaceHover;
            colors[ImGuiCol_FrameBgActive] = RedAccentTheme::SurfaceActive;

            colors[ImGuiCol_Header] = RedAccentTheme::Surface;
            colors[ImGuiCol_HeaderHovered] = RedAccentTheme::SurfaceHover;
            colors[ImGuiCol_HeaderActive] = RedAccentTheme::SurfaceActive;

            colors[ImGuiCol_Button] = RedAccentTheme::Surface;
            colors[ImGuiCol_ButtonHovered] = RedAccentTheme::SurfaceHover;
            colors[ImGuiCol_ButtonActive] = RedAccentTheme::SurfaceActive;

            colors[ImGuiCol_Tab] = RedAccentTheme::Surface;
            colors[ImGuiCol_TabHovered] = RedAccentTheme::SurfaceHover;
            colors[ImGuiCol_TabActive] = RedAccentTheme::Accent;
            colors[ImGuiCol_TabUnfocused] = RedAccentTheme::BgElevated;
            colors[ImGuiCol_TabUnfocusedActive] = RedAccentTheme::Surface;

            colors[ImGuiCol_CheckMark] = RedAccentTheme::Accent;
            colors[ImGuiCol_SliderGrab] = RedAccentTheme::Accent;
            colors[ImGuiCol_SliderGrabActive] = RedAccentTheme::AccentHover;

            colors[ImGuiCol_Separator] = RedAccentTheme::BorderSoft;
            colors[ImGuiCol_SeparatorHovered] = RedAccentTheme::Border;
            colors[ImGuiCol_SeparatorActive] = RedAccentTheme::Border;

            colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
            colors[ImGuiCol_ScrollbarGrab] = ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.25f);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.45f);
            colors[ImGuiCol_ScrollbarGrabActive] = RedAccentTheme::Accent;

            colors[ImGuiCol_ResizeGrip] = ImVec4(1, 1, 1, 0.06f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(1, 1, 1, 0.12f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(1, 1, 1, 0.18f);

            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        if (_renderer) {
            if (renderer_frame_processing_requests > 0) {
                _renderer->HideOverlayInputs(false);
            }
            if (obscure_cursor_requests > 0) {
                _renderer->HideAppInputs(true);
                if (ImGui::GetCurrentContext()) {
                    ImGuiIO& io = ImGui::GetIO();
                    io.MouseDrawCursor = true;
                    io.WantCaptureMouse = true;
                    io.WantCaptureKeyboard = true;
                }
            }
        }
    }
}

bool Steam_Overlay::open_overlay_hook(bool toggle)
{
    if (toggle) {
        ShowOverlay(!show_overlay);
    }

    return show_overlay;
}

void Steam_Overlay::allow_renderer_frame_processing(bool state, bool force)
{
    if (state) {
        if (force) {
            renderer_frame_processing_requests = 1;
        } else {
            ++renderer_frame_processing_requests;
        }
        
        if (renderer_frame_processing_requests == 1) {
            if (_renderer) _renderer->HideOverlayInputs(false);
            PRINT_DEBUG("enabled frame processing (count=%u, force=%i)", renderer_frame_processing_requests.load(), (int)force);
        }
    }
    else {
        if (force) {
            renderer_frame_processing_requests = 0;
            if (_renderer) _renderer->HideOverlayInputs(true);
            PRINT_DEBUG("disabled frame processing (count=0, force=1)");
        }
        else if (renderer_frame_processing_requests > 0) {
            auto new_val = --renderer_frame_processing_requests;
            if (new_val == 0) {
                if (_renderer) _renderer->HideOverlayInputs(true);
                PRINT_DEBUG("disabled frame processing (count=0, force=0)");
            }
        }
    }
}

void Steam_Overlay::obscure_game_input(bool state, bool force) {
    if (state) {
        if (force) {
            obscure_cursor_requests = 1;
        } else {
            ++obscure_cursor_requests;
        }
        
        if (obscure_cursor_requests == 1) {
            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = true;
            io.WantCaptureMouse = true;
            io.WantCaptureKeyboard = true;

            if (_renderer) _renderer->HideAppInputs(true);
            PRINT_DEBUG("obscured app input (count=%u, force=%i)", obscure_cursor_requests.load(), (int)force);
        }
    }
    else {
        if (force) {
            obscure_cursor_requests = 0;
            ImGuiIO& io = ImGui::GetIO();
            io.MouseDrawCursor = false;
            io.WantCaptureMouse = false;
            io.WantCaptureKeyboard = false;
            
            if (_renderer) _renderer->HideAppInputs(false);
            PRINT_DEBUG("restored app input (count=0, force=1)");
        }
        else if (obscure_cursor_requests > 0) {
            auto new_val = --obscure_cursor_requests;
            if (new_val == 0) {
                ImGuiIO& io = ImGui::GetIO();
                io.MouseDrawCursor = false;
                io.WantCaptureMouse = false;
                io.WantCaptureKeyboard = false;

                if (_renderer) _renderer->HideAppInputs(false);
                PRINT_DEBUG("restored app input (count=0, force=0)");
            }
        }
    }
}

void Steam_Overlay::notify_sound_user_invite(friend_window_state& friend_state)
{
    if (settings->disable_overlay_friend_notification) return;

    if (!(friend_state.window_state & window_state_show)) {
        friend_state.window_state |= window_state_need_attention;

        if (g_overlay_presence != OverlayPresenceState::Online) {
            return;
        }

#ifdef __WINDOWS__
        auto wav_data = wav_files.find("overlay_friend_notification.wav");
        if (wav_files.end() != wav_data && wav_data->second.size()) {
            PlaySoundA((LPCSTR)&wav_data->second[0], NULL, SND_ASYNC | SND_MEMORY);
        }
        else {
            PlaySoundA((LPCSTR)notif_invite_wav, NULL, SND_ASYNC | SND_MEMORY);
        }
#endif
    }
}

void Steam_Overlay::notify_sound_user_achievement()
{
    if (settings->disable_overlay_achievement_notification) return;

#ifdef __WINDOWS__
    auto wav_data = wav_files.find("overlay_achievement_notification.wav");
    if (wav_files.end() != wav_data && wav_data->second.size()) {
        PlaySoundA((LPCSTR)&wav_data->second[0], NULL, SND_ASYNC | SND_MEMORY);
    }
#endif
}

void Steam_Overlay::notify_sound_auto_accept_friend_invite()
{
#ifdef __WINDOWS__
    auto wav_data = wav_files.find("overlay_friend_notification.wav");
    if (wav_files.end() != wav_data && wav_data->second.size()) {
        PlaySoundA((LPCSTR)&wav_data->second[0], NULL, SND_ASYNC | SND_MEMORY);
    }
    else {
        PlaySoundA((LPCSTR)notif_invite_wav, NULL, SND_ASYNC | SND_MEMORY);
    }
#endif
}

int find_free_id(std::vector<int>& ids, int base)
{
    std::sort(ids.begin(), ids.end());

    int id = base;
    for (auto i : ids)
    {
        if (id < i)
            break;
        id = i + 1;
    }

    return id > (base + max_window_id) ? 0 : id;
}

int find_free_friend_id(const std::map<Friend, friend_window_state, Friend_Less>& friend_windows)
{
    std::vector<int> ids{};
    ids.reserve(friend_windows.size());

    std::for_each(friend_windows.begin(), friend_windows.end(), [&ids](std::pair<Friend const, friend_window_state> const& i)
        {
            ids.emplace_back(i.second.id);
        });

    return find_free_id(ids, base_friend_window_id);
}

int find_free_notification_id(std::vector<Notification> const& notifications)
{
    std::vector<int> ids{};
    ids.reserve(notifications.size());

    std::for_each(notifications.begin(), notifications.end(), [&ids](Notification const& i)
        {
            ids.emplace_back(i.id);
        });


    return find_free_id(ids, base_friend_window_id);
}

bool Steam_Overlay::submit_notification(
    notification_type type,
    const std::string& msg,
    std::pair<const Friend, friend_window_state>* frd,
    Overlay_Achievement* ach)
{
    PRINT_DEBUG("%i", (int)type);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return false;

    int id = find_free_notification_id(notifications);
    if (id == 0) {
        PRINT_DEBUG("error no free id to create a notification window");
        return false;
    }

    Notification notif{};
    notif.active = false;
    notif.start_time = std::chrono::milliseconds(0);
    notif.id = id;
    notif.type = (uint8)type;
    notif.message = msg;
    notif.frd = frd;
    if (ach) notif.ach = *ach;

    notifications.emplace_back(notif);
    allow_renderer_frame_processing(true);

    return true;
}

void Steam_Overlay::add_chat_message_notification(std::string const& message)
{
    if (settings->disable_overlay_friend_notification) return;

    PRINT_DEBUG("'%s'", message.c_str());
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    submit_notification(notification_type::message, message);
}

void Steam_Overlay::show_test_achievement()
{
    PRINT_DEBUG_ENTRY();
    Overlay_Achievement ach{};
    ach.title = translationTestAchievement[current_language];
    ach.description = "~~~ " + ach.title + " ~~~";
    ach.achieved = true;

    if (achievements.size()) {
        size_t rand_idx = common_helpers::rand_number(achievements.size() - 1);
        auto& rand_ach = achievements[rand_idx];
        bool achieved = rand_idx < (achievements.size() / 2);
        try_load_ach_icon(rand_ach, achieved, settings->paginated_achievements_icons == 0);
        ach.icon = rand_ach.icon;
        ach.icon_gray = rand_ach.icon_gray;
    }

    bool for_progress = false;
    if (common_helpers::rand_number(1000) % 2) {
        for_progress = true;
        uint32 progress = (uint32)(common_helpers::rand_number(500) / 10 + 50);
        ach.max_progress = 100;
        ach.progress = progress;
        ach.achieved = false;
    }

    post_achievement_notification(ach, for_progress);
    notify_sound_user_achievement();
}

void Steam_Overlay::build_friend_context_menu(Friend const& frd, friend_window_state& state)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, RedAccentTheme::PopupRound);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, RedAccentTheme::BgPopup);
    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);

    if (ImGui::BeginPopupContextItem("Friends_ContextMenu", 1)) {
        bool close_popup = false;
        ImVec2 size(190.0f, 30.0f);

        ImGui::TextColored(RedAccentTheme::TextMuted, "SOCIAL");
        ImGui::Separator();

        std::string chat = std::string(ICON_FA_COMMENT) + " " + translationChat[current_language];
        if (SteamButton(chat.c_str(), size, false)) {
            close_popup = true;
            state.window_state |= window_state_show;
        }

        std::string copy = std::string(ICON_FA_COPY) + " " + translationCopyId[current_language];
        if (SteamButton(copy.c_str(), size, false)) {
            close_popup = true;
            ImGui::SetClipboardText(std::to_string(frd.id()).c_str());
        }

        ImGui::Spacing();
        ImGui::TextColored(RedAccentTheme::TextMuted, "SAFETY");
        ImGui::Separator();

        const uint64_t friend_id = (uint64)frd.id();
        const bool muted = IsUserMuted(friend_id);
        const bool blocked = IsUserBlocked(friend_id);

        if (SteamButton(muted ? "Unmute User" : "Mute User", size, false)) {
            close_popup = true;

            if (muted) g_muted_users.erase(friend_id);
            else g_muted_users.insert(friend_id);

            SaveGBEConfig();
        }

        if (SteamButton(blocked ? "Unblock User" : "Block User", size, false)) {
            close_popup = true;

            if (blocked) {
                g_blocked_users.erase(friend_id);
            }
            else {
                g_blocked_users.insert(friend_id);
                state.window_state &= ~(window_state_show | window_state_lobby_invite | window_state_rich_invite);
            }

            SaveGBEConfig();
        }

        ImGui::Spacing();
        ImGui::TextColored(RedAccentTheme::TextMuted, "GAME");
        ImGui::Separator();

        const bool has_pending_invite =
            (state.window_state & window_state_lobby_invite) ||
            (state.window_state & window_state_rich_invite);

        if (g_overlay_presence == OverlayPresenceState::Idle && has_pending_invite) {
            if (SteamButton((std::string(ICON_FA_CHECK) + " " + translationAccept[current_language]).c_str(), size, true)) {
                close_popup = true;
                state.window_state |= window_state_join;
                has_friend_action.push(frd);
            }

            if (SteamButton((std::string(ICON_FA_XMARK) + " " + translationRefuse[current_language]).c_str(), size, false)) {
                close_popup = true;
                state.window_state &= ~(window_state_lobby_invite | window_state_rich_invite);
            }

            ImGui::Spacing();
            ImGui::Separator();
        }

        {
            std::lock_guard<std::recursive_mutex> lock(global_mutex);
            Steam_Friends* steamFriends = get_steam_client()->steam_friends;
            Steam_Matchmaking* steamMatchmaking = get_steam_client()->steam_matchmaking;

            bool friend_joinable = false;

            const char* join_locked = steamFriends->get_friend_rich_presence_silent((uint64)frd.id(), kOverlayJoinPrivacyKey);
            const bool friend_locked_join = (join_locked && join_locked[0] == '1');

            if (!friend_locked_join) {
                friend_joinable =
                    std::string(
                        steamFriends->get_friend_rich_presence_silent((uint64)frd.id(), "connect")
                    ).length() > 0;
            }

            const bool invite_enabled = i_have_lobby;
            const bool join_enabled = friend_joinable;

            if (!invite_enabled) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.38f);
                ImGui::BeginDisabled();
            }
            if (SteamButton("Invite to Game", size, invite_enabled)) {
                close_popup = true;
                invite_friend((uint64)frd.id(), steamFriends, steamMatchmaking);
            }
            if (!invite_enabled) {
                ImGui::EndDisabled();
                ImGui::PopStyleVar();
            }

            if (!join_enabled) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.38f);
                ImGui::BeginDisabled();
            }
            if (SteamButton("Join Game", size, join_enabled)) {
                close_popup = true;
                state.window_state |= window_state_join;
                has_friend_action.push(frd);
            }
            if (!join_enabled) {
                ImGui::EndDisabled();
                ImGui::PopStyleVar();
            }
        }

        if (close_popup) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

void Steam_Overlay::build_friend_window(Friend const& frd, friend_window_state& state)
{
    if (!(state.window_state & window_state_show))
        return;

    bool show = true;
    bool send_chat_msg = false;

    float width = ImGui::CalcTextSize("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA").x;

    if (state.window_state & window_state_need_attention && ImGui::IsWindowFocused()) {
        state.window_state &= ~window_state_need_attention;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2{ width, ImGui::GetFontSize() * 8 + ImGui::GetFrameHeightWithSpacing() * 4 },
        ImVec2{ std::numeric_limits<float>::max() , std::numeric_limits<float>::max() });

    ImGui::SetNextWindowBgAlpha(1.0f);
    std::string friend_window_id = std::move("###" + std::to_string(state.id));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
    if (ImGui::Begin(
        (state.window_title + friend_window_id).c_str(),
        &show,
        ImGuiWindowFlags_NoCollapse)) {
        if (state.window_state & window_state_need_attention && ImGui::IsWindowFocused()) {
            state.window_state &= ~window_state_need_attention;
        }

        ImGui::InputTextMultiline("##chat_history", &state.chat_history[0], state.chat_history.length(), { -1.0f, -2.0f * ImGui::GetFontSize() }, ImGuiInputTextFlags_ReadOnly);

        float wnd_width = ImGui::GetContentRegionAvail().x;
        ImGuiStyle& style = ImGui::GetStyle();
        wnd_width -= ImGui::CalcTextSize(translationSend[current_language]).x + style.FramePadding.x * 2 + style.ItemSpacing.x + 1;

        uint64_t frd_id = frd.id();
        ImGui::PushID((const char*)&frd_id, (const char*)&frd_id + sizeof(frd_id));
        ImGui::PushItemWidth(wnd_width);

        if (ImGui::InputText("##chat_line", state.chat_input, max_chat_len, ImGuiInputTextFlags_EnterReturnsTrue)) {
            send_chat_msg = true;
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::PopItemWidth();
        ImGui::PopID();

        ImGui::SameLine();

        if (ImGui::Button(translationSend[current_language])) {
            send_chat_msg = true;
        }

        if (send_chat_msg) {
            state.chat_input[max_chat_len - 1] = '\0';

            std::string emoji_text = ApplyEmojiShortcodes(state.chat_input);
            strncpy(state.chat_input, emoji_text.c_str(), max_chat_len - 1);
            state.chat_input[max_chat_len - 1] = '\0';

            if (!(state.window_state & window_state_send_message)) {
                has_friend_action.push(frd);
                state.window_state |= window_state_send_message;
            }
        }
    }

    if (!show) {
        state.window_state &= ~window_state_show;
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}

std::chrono::milliseconds Steam_Overlay::get_notification_duration(notification_type type)
{
    switch (type)
    {
    case notification_type::message:
        return std::chrono::milliseconds(settings->overlay_appearance.notification_duration_chat);

    case notification_type::invite:
        return std::chrono::milliseconds(settings->overlay_appearance.notification_duration_invitation);

    case notification_type::achievement:
        return std::chrono::milliseconds(settings->overlay_appearance.notification_duration_achievement);

    case notification_type::achievement_progress:
        return std::chrono::milliseconds(settings->overlay_appearance.notification_duration_progress);

    case notification_type::auto_accept_invite:
        return Notification::default_show_time;
    }

    PRINT_DEBUG("ERROR unhandled type %i", (int)type);
    return Notification::default_show_time;
}

void Steam_Overlay::set_next_notification_pos(std::pair<float, float> scrn_size, std::chrono::milliseconds elapsed, std::chrono::milliseconds duration, const Notification& noti, struct NotificationsCoords& coords)
{
    const float scrn_width = scrn_size.first;
    const float scrn_height = scrn_size.second;

    auto& global_style = ImGui::GetStyle();
    const float padding_all_sides = 2 * (global_style.WindowPadding.y + global_style.WindowPadding.x);

    const float noti_width = scrn_width * Notification::width_percent;
    const float msg_height = ImGui::CalcTextSize(
        noti.message.c_str(),
        noti.message.c_str() + noti.message.size(),
        false,
        noti_width - padding_all_sides - global_style.ItemSpacing.x
    ).y;
    float noti_height = msg_height;

    Overlay_Appearance::NotificationPosition pos = Overlay_Appearance::default_pos;
    switch ((notification_type)noti.type) {
    case notification_type::achievement_progress:
    case notification_type::achievement: {
        pos = settings->overlay_appearance.ach_earned_pos;

        const float new_msg_height = ImGui::CalcTextSize(
            noti.message.c_str(),
            noti.message.c_str() + noti.message.size(),
            false,
            noti_width - padding_all_sides - global_style.ItemSpacing.x - settings->overlay_appearance.icon_size
        ).y;
        const float new_noti_height = new_msg_height;

        float biggest_noti_height = settings->overlay_appearance.icon_size;
        if (biggest_noti_height < new_noti_height) biggest_noti_height = new_noti_height;

        noti_height = biggest_noti_height;

        if ((notification_type)noti.type == notification_type::achievement_progress) {
            if (!noti.ach.value().achieved && noti.ach.value().max_progress > 0) {
                noti_height += settings->overlay_appearance.font_size + global_style.WindowPadding.y;
            }
        }
    }
                                       break;

    case notification_type::invite: {
        pos = settings->overlay_appearance.invite_pos;

        const float header_height = settings->overlay_appearance.font_size + global_style.ItemSpacing.y + 6.0f;
        const float button_height = 30.0f;
        const float button_block_height = button_height + global_style.ItemSpacing.y + 8.0f;
        const float text_wrap_width = noti_width - padding_all_sides - global_style.ItemSpacing.x;

        const float msg_height = ImGui::CalcTextSize(
            noti.message.c_str(),
            noti.message.c_str() + noti.message.size(),
            false,
            text_wrap_width
        ).y;

        noti_height = header_height + msg_height + button_block_height + (global_style.WindowPadding.y * 2.0f) + 20.0f;
    }
                                  break;

    case notification_type::message: {
        pos = settings->overlay_appearance.chat_msg_pos;

        const float header_height = settings->overlay_appearance.font_size + 4.0f;
        const float body_height = settings->overlay_appearance.font_size;

        noti_height = header_height + body_height + (global_style.WindowPadding.y * 2.0f) + 2.0f;
    }
                                   break;
    default: PRINT_DEBUG("ERROR: unhandled notification type %i", (int)noti.type); break;
    }
    if ((notification_type)noti.type != notification_type::invite) {
        noti_height += 2 * global_style.WindowPadding.y;
    }

    float x = 0.0f;
    float y = 0.0f;
    float animate_size = 0.0f;
    const float margin_y = settings->overlay_appearance.notification_margin_y;
    const float margin_x = settings->overlay_appearance.notification_margin_x;

    switch (pos) {
    case Overlay_Appearance::NotificationPosition::top_left:
        animate_size = animate_factor(elapsed, duration) * noti_width;
        x = margin_x - animate_size;
        y = coords.top_left.second + margin_y;
        coords.top_left.second = y + noti_height;
        break;
    case Overlay_Appearance::NotificationPosition::top_center:
        animate_size = animate_factor(elapsed, duration) * noti_height;
        x = (scrn_width / 2) - (noti_width / 2);
        y = coords.top_center.second + margin_y - animate_size;
        coords.top_center.second = y + noti_height;
        break;
    case Overlay_Appearance::NotificationPosition::top_right:
        animate_size = animate_factor(elapsed, duration) * noti_width;
        x = (scrn_width - noti_width - margin_x) + animate_size;
        y = coords.top_right.second + margin_y;
        coords.top_right.second = y + noti_height;
        break;

    case Overlay_Appearance::NotificationPosition::bot_left:
        animate_size = animate_factor(elapsed, duration) * noti_width;
        x = margin_x - animate_size;
        y = scrn_height - coords.bot_left.second - margin_y - noti_height;
        coords.bot_left.second = scrn_height - y;
        break;
    case Overlay_Appearance::NotificationPosition::bot_center:
        animate_size = animate_factor(elapsed, duration) * noti_height;
        x = (scrn_width / 2) - (noti_width / 2);
        y = scrn_height - coords.bot_center.second - margin_y - noti_height + animate_size;
        coords.bot_center.second = scrn_height - y;
        break;
    case Overlay_Appearance::NotificationPosition::bot_right:
        animate_size = animate_factor(elapsed, duration) * noti_width;
        x = (scrn_width - noti_width - margin_x) + animate_size;
        y = scrn_height - coords.bot_right.second - margin_y - noti_height;
        coords.bot_right.second = scrn_height - y;
        break;

    default: break;
    }

    ImGui::SetNextWindowPos(ImVec2(x, y));
    ImGui::SetNextWindowSize(ImVec2(noti_width, noti_height));
}

float Steam_Overlay::animate_factor(std::chrono::milliseconds elapsed, std::chrono::milliseconds duration)
{
    if (settings->overlay_appearance.notification_animation <= 0) return 0.0f;

    std::chrono::milliseconds animation_duration(settings->overlay_appearance.notification_animation);

    float factor = 0.0f;
    if (elapsed < animation_duration) {
        float t = static_cast<float>(elapsed.count()) / animation_duration.count();
        t = std::clamp(t, 0.0f, 1.0f);
        // Cubic ease-out: progress = 1 - (1 - t)^3, factor = (1 - t)^3
        float one_minus_t = 1.0f - t;
        factor = one_minus_t * one_minus_t * one_minus_t;
    }
    else {
        auto steady_time = animation_duration + duration;
        if (elapsed > steady_time) {
            float t = static_cast<float>((elapsed - steady_time).count()) / animation_duration.count();
            t = std::clamp(t, 0.0f, 1.0f);
            // Cubic ease-in: factor = t^3
            factor = t * t * t;
        }
    }

    return factor;
}

void Steam_Overlay::add_ach_progressbar(const Overlay_Achievement& ach)
{
    if (!ach.achieved && ach.max_progress > 0) {
        char buf[32]{};
        sprintf(buf, "%u/%u", ach.progress, ach.max_progress);
        ImGui::ProgressBar((float)ach.progress / ach.max_progress, { -1 , settings->overlay_appearance.font_size }, buf);
    }
}

ImVec4 Steam_Overlay::get_notification_bg_rgba_safe()
{
    if (settings->overlay_appearance.notification_r >= 0 &&
        settings->overlay_appearance.notification_g >= 0 &&
        settings->overlay_appearance.notification_b >= 0 &&
        settings->overlay_appearance.notification_a >= 0)
    {
        return ImVec4(
            settings->overlay_appearance.notification_r,
            settings->overlay_appearance.notification_g,
            settings->overlay_appearance.notification_b,
            settings->overlay_appearance.notification_a
        );
    }

    return ImVec4(0.10f, 0.12f, 0.18f, 1.0f);
}

void Steam_Overlay::build_notifications(float width, float height)
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::queue<Friend> friend_actions_temp{};

    // Count currently active, non-expired notifications
    int active_count = 0;
    for (const auto& item : notifications) {
        if (item.active && !item.expired) {
            active_count++;
        }
    }

    // Activate inactive, non-expired notifications up to the cap of 4
    if (active_count < 4) {
        for (auto& item : notifications) {
            if (!item.active && !item.expired) {
                item.active = true;
                item.start_time = now;
                active_count++;

                // If this is an invite, obscure the game input now that it becomes active
                if ((notification_type)item.type == notification_type::invite) {
                    obscure_game_input(true);
                }

                if (active_count >= 4) {
                    break;
                }
            }
        }
    }

    ImGui::PushFont(font_notif);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, settings->overlay_appearance.notification_rounding);

    NotificationsCoords coords{};
    int visible_message_count = 0;
    int visible_invite_count = 0;

    for (auto it = notifications.begin(); it != notifications.end(); ++it) {
        if (!it->active) {
            continue;
        }
        auto noti_duration = get_notification_duration((notification_type)it->type);
        if (noti_duration.count() <= 0) {
            it->expired = true;
            continue;
        }

        auto total_allowed_duration = noti_duration + std::chrono::milliseconds(settings->overlay_appearance.notification_animation * 2);
        auto elapsed_notif = now - it->start_time;
        if (elapsed_notif > total_allowed_duration) {
            it->expired = true;
            continue;
        }

        if ((notification_type)it->type == notification_type::message) {
            if (visible_message_count >= kMaxVisibleMessageNotifications) {
                continue;
            }
            ++visible_message_count;
        }

        if ((notification_type)it->type == notification_type::invite) {
            if (visible_invite_count >= kMaxVisibleInviteNotifications) {
                continue;
            }
            ++visible_invite_count;
        }

        float settings_noti_alpha = settings->overlay_appearance.notification_a >= 0.0f && settings->overlay_appearance.notification_a <= 1.0f
            ? settings->overlay_appearance.notification_a
            : 1.0f;

        ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, RedAccentTheme::BgPopup);
        ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Text);

        ImGuiWindowFlags extra_flags = ImGuiWindowFlags_NoFocusOnAppearing;
        switch ((notification_type)it->type) {
        case notification_type::achievement_progress:
        case notification_type::achievement:
        case notification_type::auto_accept_invite:
            extra_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoInputs;
            break;

        case notification_type::message:
            extra_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
            break;

        case notification_type::invite:
            break;

        default:
            PRINT_DEBUG("error unhandled flags for type %i", (int)it->type);
            break;
        }

        std::string wnd_name = "NotiPopupShow" + std::to_string(it->id);

        set_next_notification_pos({ width, height }, elapsed_notif, noti_duration, *it, coords);
        bool notif_open = ImGui::Begin(wnd_name.c_str(), nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | extra_flags);

        if (notif_open) {
            ImGui::PushStyleColor(ImGuiCol_Button, RedAccentTheme::Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RedAccentTheme::AccentHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, RedAccentTheme::AccentActive);

            switch ((notification_type)it->type) {
            case notification_type::achievement_progress:
            case notification_type::achievement: {
                const auto& ach = it->ach.value();
                auto& icon_rsrc = (notification_type)it->type == notification_type::achievement
                    ? ach.icon
                    : ach.icon_gray;
                const bool can_show_icon = (icon_rsrc->GetResourceId() != 0);
                bool notif_table_open = false;

                if (can_show_icon) {
                    notif_table_open = ImGui::BeginTable("imgui_table", 2);
                }

                if (notif_table_open) {
                    ImGui::TableSetupColumn("imgui_table_image", ImGuiTableColumnFlags_WidthFixed, settings->overlay_appearance.icon_size);
                    ImGui::TableSetupColumn("imgui_table_text");
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, settings->overlay_appearance.icon_size);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Image(icon_rsrc->GetResourceId(), ImVec2(settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size));

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
                    ImGui::PushFont(font_ach_title);
                    ImGui::TextWrapped("%s", ach.title.c_str());
                    ImGui::PopFont();
                    if (ach.description.size()) {
                        ImGui::PushFont(font_ach_desc);
                        ImGui::TextWrapped("%s", ach.description.c_str());
                        ImGui::PopFont();
                    }
                    ImGui::PopTextWrapPos();

                    ImGui::EndTable();
                }
                else {
                    ImGui::PushFont(font_ach_title);
                    ImGui::TextWrapped("%s", ach.title.c_str());
                    ImGui::PopFont();
                    if (ach.description.size()) {
                        ImGui::PushFont(font_ach_desc);
                        ImGui::TextWrapped("%s", ach.description.c_str());
                        ImGui::PopFont();
                    }
                }

                if ((notification_type)it->type == notification_type::achievement_progress) {
                    add_ach_progressbar(ach);
                }
            }
                                               break;

            case notification_type::invite: {
                ImGui::TextColored(RedAccentTheme::Accent, ICON_FA_USER_GROUP " GAME INVITE");
                ImGui::Separator();

                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextWrapped("%s", it->message.c_str());
                ImGui::PopTextWrapPos();

                ImGui::Dummy(ImVec2(0.0f, 4.0f));

                if (SteamButton(translationJoin[current_language], ImVec2(-1.0f, 30.0f), true)) {
                    it->frd->second.window_state |= window_state_join;
                    friend_actions_temp.push(it->frd->first);
                    it->expired = true;
                }
            }
                                          break;

            case notification_type::message: {
                std::string sender_name = it->message;
                const std::string suffix = " sent you a message";
                const size_t suffix_pos = sender_name.rfind(suffix);
                if (suffix_pos != std::string::npos) {
                    sender_name.erase(suffix_pos);
                }

                ImGui::TextColored(RedAccentTheme::Accent, ICON_FA_COMMENT " MESSAGE");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));

                ImGui::TextColored(RedAccentTheme::Text, "%s", sender_name.c_str());
                ImGui::SameLine();
                ImGui::TextColored(RedAccentTheme::TextDim, " sent you a message");

                ImGui::PopStyleVar();

                if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseClicked(0)) {
                    it->expired = true;
                }
                break;
            }

            case notification_type::auto_accept_invite:
                ImGui::TextColored(RedAccentTheme::Accent, ICON_FA_CHECK " AUTO ACCEPT");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextWrapped("%s", it->message.c_str());
                break;

            default:
                PRINT_DEBUG("error unhandled notification for type %i", (int)it->type);
                break;
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::End();

        ImGui::PopStyleColor(3);
    }

    ImGui::PopStyleVar();
    ImGui::PopFont();

    notifications.erase(std::remove_if(notifications.begin(), notifications.end(), [this](const Notification& item) {
        if (item.expired) {
            PRINT_DEBUG("removing a notification");
            allow_renderer_frame_processing(false);
            switch ((notification_type)item.type) {
            case notification_type::invite:
                obscure_game_input(false);
                break;

            case notification_type::achievement_progress:
            case notification_type::achievement:
            case notification_type::auto_accept_invite:
            case notification_type::message:
                break;

            default:
                PRINT_DEBUG("error unhandled remove for type %i", (int)item.type);
                break;
            }

            // Archive to notification history (lightweight copy, no pointers/GPU resources)
            {
                NotificationHistoryEntry entry{};
                // Use actual achievement unlock time when available,
                // otherwise fall back to the notification display time.
                if (item.ach.has_value() && item.ach->unlock_time > 0) {
                    entry.timestamp = std::chrono::milliseconds(
                        static_cast<long long>(item.ach->unlock_time) * 1000);
                } else {
                    entry.timestamp = item.start_time;
                }
                entry.type = item.type;
                entry.message = item.message;
                if (notification_history.size() >= MAX_NOTIFICATION_HISTORY) {
                    notification_history.pop_front();
                }
                notification_history.push_back(std::move(entry));
                notification_history_cache_dirty = true;
            }

            return true;
        }

        return false;
        }), notifications.end());

    if (!friend_actions_temp.empty()) {
        while (!friend_actions_temp.empty()) {
            has_friend_action.push(friend_actions_temp.front());
            friend_actions_temp.pop();
        }
    }
}

void Steam_Overlay::add_auto_accept_invite_notification()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    char tmp[TRANSLATION_BUFFER_SIZE]{};
    snprintf(tmp, sizeof(tmp), "%s", translationAutoAcceptFriendInvite[current_language]);

    submit_notification(notification_type::auto_accept_invite, tmp);
    notify_sound_auto_accept_friend_invite();
}

void Steam_Overlay::add_invite_notification(std::pair<const Friend, friend_window_state>& wnd_state)
{
    if (settings->disable_overlay_friend_notification) return;

    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    char tmp[TRANSLATION_BUFFER_SIZE]{};
    auto& first_friend = wnd_state.first;
    auto& name = first_friend.name();
    snprintf(tmp, sizeof(tmp), "%s invited you to join", name.c_str());

    if (OverlayPresenceAllowsPopups()) {
        submit_notification(notification_type::invite, tmp, &wnd_state);
    }
}

void Steam_Overlay::post_achievement_notification(Overlay_Achievement& ach, bool for_progress)
{
    if (settings->disable_overlay_achievement_notification) return;

    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    // Get current time
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    // Calculate scheduled show time based on rate limiting
    std::chrono::milliseconds scheduled_show_time;
    int delay_ms = settings->achievement_notification_delay_ms;

    if (delay_ms <= 0) {
        // No delay - show immediately
        scheduled_show_time = now;
    } else {
        // Apply rate limiting: earliest show time is last_scheduled_show_time + delay
        scheduled_show_time = std::max(now, last_scheduled_show_time + std::chrono::milliseconds(delay_ms));
    }

    // Create scheduled achievement entry
    ScheduledAchievement scheduled_ach;
    scheduled_ach.ach = ach;
    scheduled_ach.for_progress = for_progress;
    scheduled_ach.trigger_time = now;
    scheduled_ach.scheduled_show_time = scheduled_show_time;

    // Add to queue
    achievement_queue.push_back(scheduled_ach);

    // Update last scheduled show time for next item
    last_scheduled_show_time = scheduled_show_time;

    PRINT_DEBUG("Achievement queued: '%s', scheduled for %lld ms, delay=%d ms", 
                ach.name.c_str(), (long long)scheduled_show_time.count(), delay_ms);
}

void Steam_Overlay::process_achievement_queue()
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;
    if (achievement_queue.empty()) return;

    // Get current time
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    // Process all ready achievements
    while (!achievement_queue.empty()) {
        auto& scheduled_ach = achievement_queue.front();

        // Check if it's time to show this notification
        if (scheduled_ach.scheduled_show_time <= now) {
            // Show the notification
            bool achieved = !scheduled_ach.for_progress;
            // force upload to GPU if the pagination is request-based
            try_load_ach_icon(scheduled_ach.ach, achieved, settings->paginated_achievements_icons == 0);

            submit_notification(
                scheduled_ach.for_progress ? notification_type::achievement_progress : notification_type::achievement,
                scheduled_ach.ach.title + "\n" + scheduled_ach.ach.description,
                {},
                &scheduled_ach.ach
            );

            // Play sound when notification is actually shown (delayed with queue)
            notify_sound_user_achievement();

            PRINT_DEBUG("Achievement shown: '%s' at %lld ms", 
                        scheduled_ach.ach.name.c_str(), (long long)now.count());

            // Remove from queue
            achievement_queue.pop_front();
        } else {
            // This achievement is not ready yet, and queue is ordered by scheduled time,
            // so no more achievements will be ready either
            break;
        }
    }
}

bool Steam_Overlay::try_load_ach_icon(Overlay_Achievement& ach, bool achieved, bool upload_new_icon_to_gpu)
{
    if (!_renderer) return false;
    if (settings->paginated_achievements_icons < 0) return false;
    if (!settings->overlay_upload_achs_icons_to_gpu) return false;

    auto& icon_rsrc = achieved ? ach.icon : ach.icon_gray;
    if (icon_rsrc->GetResourceId() != 0) return true;

    if (!upload_new_icon_to_gpu) return false;

    int& icon_handle = achieved ? ach.icon_handle : ach.icon_gray_handle;
    if (Settings::UNLOADED_IMAGE_HANDLE == icon_handle) {
        icon_handle = get_steam_client()->steam_user_stats->get_achievement_icon_handle(ach.name, achieved);
    }
    auto image_info = settings->get_image(icon_handle);
    if (image_info) {
        icon_rsrc->AttachResource((void*)image_info->data.c_str(), image_info->width, image_info->height);

        PRINT_DEBUG("'%s' (result=%i)", ach.name.c_str(), (int)icon_rsrc->GetResourceId() != 0);
    }

    return icon_rsrc->GetResourceId() != 0;
}

void Steam_Overlay::overlay_render_proc()
{
    std::lock_guard lock(overlay_mutex);

    if (!Ready()) return;

    process_achievement_queue();
    
    if (pending_close_overlay) {
        if (!ImGui::GetCurrentContext() || !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            pending_close_overlay = false;
            ShowOverlay(false);
        }
    }

    if (show_overlay) {
        if (obscure_cursor_requests == 0) {
            obscure_game_input(true);
        }
        if (renderer_frame_processing_requests == 0) {
            allow_renderer_frame_processing(true);
        }
    }

    if (show_overlay) {
        render_main_window();
    }

    if (notifications.size()) {
        ImGuiIO& io = ImGui::GetIO();
        build_notifications(io.DisplaySize.x, io.DisplaySize.y);
    }

    if (stats.show_any_stats()) {
        stats.render_stats(current_language);
    }

    if (g_show_clock_hud) {
        RenderClockHUD(show_overlay);
    }

    load_next_ach_icon();
}

uint32 Steam_Overlay::apply_global_style_color()
{
    return 0;
}

void Steam_Overlay::render_main_window()
{
    char tmp[TRANSLATION_BUFFER_SIZE]{};
    snprintf(tmp, sizeof(tmp), translationRenderer[current_language],
        (_renderer == nullptr ? "Unknown" : _renderer->GetLibraryName()));

    ImGuiIO& io = ImGui::GetIO();
    AutoSaveBroadcastPortIfNeeded();
    const float fade_duration = 0.18f;
    float overlay_alpha = 1.0f;
    if (g_overlay_open_time > 0.0) {
        overlay_alpha = std::clamp(
            static_cast<float>((ImGui::GetTime() - g_overlay_open_time) / fade_duration),
            0.0f, 1.0f
        );
    }

    static bool show_friends = false;
    static bool show_achievements = false;
    static bool show_settings = false;
    static int friends_root_tab = 1;
    static int settings_tab = 0;
    static float time_acc = 0.0f;
    time_acc += io.DeltaTime;

    ImGui::PushFont(font_default);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, overlay_alpha);

    ImGuiWindowFlags bgFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoFocusOnAppearing;
if (ImGui::Begin("GBEOverlayBackground", nullptr, bgFlags)) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
        // DrawRadialBackground(dl, io.DisplaySize);

        // DrawRedGlowEffect(dl, io.DisplaySize, time_acc);

        dl->AddRectFilledMultiColor(
            ImVec2(0.0f, 0.0f),
            ImVec2(io.DisplaySize.x, io.DisplaySize.y),
            ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.30f)),
            ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.15f)),
            ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.35f)),
            ImGui::GetColorU32(ImVec4(0.00f, 0.00f, 0.00f, 0.20f))
        );

        DrawOverlayBackgroundFx(dl, io.DisplaySize, overlay_alpha);

        dl->AddRectFilled(
            ImVec2(0.0f, 0.0f),
            ImVec2(io.DisplaySize.x, io.DisplaySize.y),
            IM_COL32(0, 0, 0, (int)(160 * overlay_alpha))
        );

        const float topbar_y = 16.0f;
        const float icon_btn_size = 40.0f;
        const float icon_spacing = 12.0f;
        const float close_btn_size = 40.0f;

        const float menu_total_w =
            icon_btn_size * 5.0f +
            icon_spacing * 4.0f;

        const float menu_start_x = (io.DisplaySize.x - menu_total_w) * 0.5f;

        auto DrawTopIconButton = [&](const char* id, const char* icon, const char* tooltip, bool active) -> bool
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, active ? 1.2f : 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));

                ImGui::PushStyleColor(ImGuiCol_Button,
                    active ? RedAccentTheme::AccentDim : ImVec4(0.12f, 0.14f, 0.18f, 0.28f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                    active ? RedAccentTheme::AccentDim : ImVec4(0.18f, 0.22f, 0.30f, 0.36f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                    active ? RedAccentTheme::AccentDim : ImVec4(0.18f, 0.22f, 0.30f, 0.42f));
                ImGui::PushStyleColor(ImGuiCol_Border,
                    active ? RedAccentTheme::BorderActive : RedAccentTheme::BorderSoft);
                ImGui::PushStyleColor(ImGuiCol_Text,
                    active ? RedAccentTheme::Accent : RedAccentTheme::Text);

                bool pressed = ImGui::Button((std::string(icon) + id).c_str(), ImVec2(icon_btn_size, icon_btn_size));

                ImGuiID btn_id = ImGui::GetItemID();
                float hover_t = AnimateHover(ImGui::IsItemHovered(), btn_id, 12.0f);

                ImDrawList* btn_dl = ImGui::GetWindowDrawList();
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                float rounding = RedAccentTheme::ButtonRound;

                if (hover_t > 0.001f || active) {
                    btn_dl->AddRect(
                        min,
                        max,
                        ImGui::GetColorU32(ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.4f + hover_t * 0.3f)),
                        rounding,
                        0,
                        1.5f
                    );
                }

                if (hover_t > 0.01f) {
                    btn_dl->AddRectFilled(
                        ImVec2(min.x - 2.0f, min.y - 2.0f),
                        ImVec2(max.x + 2.0f, max.y + 2.0f),
                        ImGui::GetColorU32(ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.05f * hover_t)),
                        rounding + 2.0f
                    );
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.2f);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, RedAccentTheme::BgPopup);
                    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
                    ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Accent);

                    ImGui::SetTooltip("%s", tooltip);

                    ImGui::PopStyleColor(3);
                    ImGui::PopStyleVar(3);
                }

                ImGui::PopStyleColor(5);
                ImGui::PopStyleVar(3);
                return pressed;
            };

        ImGui::SetCursorPos(ImVec2(22.0f, 20.0f));
        ImGui::BeginGroup();

        ImDrawList* title_dl = ImGui::GetWindowDrawList();
        ImVec2 title_pos = ImGui::GetCursorScreenPos();

        const char* title_text = ICON_FA_CROWN " GBE_Overlay";
        const char* status_text = OverlayPresenceLabels[(int)g_overlay_presence];
        const char* status_icon = OverlayPresenceIcon();

        ImVec2 title_size = ImGui::CalcTextSize(title_text);
        ImVec2 status_icon_size = ImGui::CalcTextSize(status_icon);
        ImVec2 status_full_size = ImGui::CalcTextSize((std::string(status_icon) + " " + status_text).c_str());

        title_dl->AddText(ImVec2(title_pos.x + 2.0f, title_pos.y + 2.0f), IM_COL32(0, 0, 0, 100), title_text);
        title_dl->AddText(ImVec2(title_pos.x + 1.0f, title_pos.y + 1.0f), IM_COL32(0, 0, 0, 150), title_text);
        title_dl->AddText(title_pos, ImGui::GetColorU32(RedAccentTheme::Accent), title_text);

        ImVec2 status_pos = ImVec2(title_pos.x + title_size.x + 12.0f, title_pos.y);
        title_dl->AddText(ImVec2(status_pos.x + 1.0f, status_pos.y + 1.0f), IM_COL32(0, 0, 0, 100), (std::string(status_icon) + " " + status_text).c_str());
        title_dl->AddText(status_pos, ImGui::GetColorU32(OverlayPresenceColor()), status_icon);
        title_dl->AddText(ImVec2(status_pos.x + status_icon_size.x + 4.0f, status_pos.y), ImGui::GetColorU32(ImVec4(0.91f, 0.91f, 0.95f, 0.95f)), status_text);

        ImVec2 underline_start = ImVec2(title_pos.x, title_pos.y + title_size.y + 4.0f);
        ImVec2 underline_end = ImVec2(title_pos.x + title_size.x + status_full_size.x + 12.0f, title_pos.y + title_size.y + 4.0f);
        title_dl->AddRectFilled(underline_start, ImVec2(underline_end.x, underline_end.y + 2.0f), ImGui::GetColorU32(ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.35f)), 2.0f);
        title_dl->AddRectFilled(ImVec2(underline_start.x, underline_start.y - 1.0f), ImVec2(underline_end.x, underline_start.y), ImGui::GetColorU32(ImVec4(RedAccentTheme::Accent.x, RedAccentTheme::Accent.y, RedAccentTheme::Accent.z, 0.15f)), 1.0f);

        ImGui::EndGroup();

        ImGui::SetCursorPos(ImVec2(menu_start_x, topbar_y));

        if (DrawTopIconButton("##TopFriends", ICON_FA_USERS, "Friends", show_friends || friends_pinned))
        {
            const bool was_closed = !show_friends && !friends_pinned;
            show_friends = !show_friends;
            if (show_friends && was_closed && friends_root_tab != 0 && friends_root_tab != 1) {
                friends_root_tab = 1;
            }
        }

        ImGui::SameLine(0.0f, icon_spacing);

        if (DrawTopIconButton("##TopMessages", ICON_FA_COMMENTS, "Messages", show_friends && friends_root_tab == 0))
        {
            const bool was_closed = !show_friends && !friends_pinned;
            show_friends = true;
            if (was_closed || friends_root_tab != 0) {
                friends_root_tab = 0;
            }
        }

        ImGui::SameLine(0.0f, icon_spacing);

        if (DrawTopIconButton("##TopAchievements", ICON_FA_TROPHY, "Achievements", show_achievements || achievements_pinned))
        {
            show_achievements = !show_achievements;
        }

        ImGui::SameLine(0.0f, icon_spacing);

        if (DrawTopIconButton("##TopSettings", ICON_FA_GEAR, "Settings", show_settings || settings_pinned))
        {
            show_settings = !show_settings;
        }

        ImGui::SameLine(0.0f, icon_spacing);

        if (DrawTopIconButton("##TopHistory", ICON_FA_CLOCK, translationNotificationHistory[current_language], show_notification_history || history_pinned))
        {
            show_notification_history = !show_notification_history;
        }

        ImGui::SetCursorPos(ImVec2(io.DisplaySize.x - close_btn_size - 18.0f, topbar_y));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, RedAccentTheme::AccentDim);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, RedAccentTheme::AccentDim);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, RedAccentTheme::AccentDim);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, RedAccentTheme::Accent);

        if (ImGui::Button(ICON_FA_XMARK "##TopBarCloseOverlay", ImVec2(close_btn_size, close_btn_size))) {
            pending_close_overlay = true;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Close Overlay");
        }

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(3);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);

    if (show_friends || friends_pinned) {
        if (BeginModernWindow((std::string(translationFriendsAndProfile[current_language]) + "###FriendsAndProfileWindow").c_str(), ImVec2(560, 650), &show_friends, current_language, &friends_pinned, false, true)) {

            if (BeginFlatPanel("FriendsTopTabsSimple", ImVec2(0, 58))) {
                const float gap = 10.0f;
                const float btn_w = (ImGui::GetContentRegionAvail().x - gap) * 0.5f;

                if (SteamButton((std::string(ICON_FA_USERS) + " Friends").c_str(), ImVec2(btn_w, 34), friends_root_tab == 1)) {
                    friends_root_tab = 1;
                }

                ImGui::SameLine(0.0f, gap);

                if (SteamButton((std::string(ICON_FA_USER) + " Profile").c_str(), ImVec2(btn_w, 34), friends_root_tab == 0)) {
                    friends_root_tab = 0;
                }
            }
            EndFlatPanel();

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::SurfaceTransparent);
            ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::BorderSoft);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));

            if (ImGui::BeginChild("FriendsSummaryStrip", ImVec2(0, 54), true,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysUseWindowPadding))
            {
                char friends_count_text[64]{};
                snprintf(friends_count_text, sizeof(friends_count_text), "%d FRIENDS CONNECTED", (int)friends.size());

                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(RedAccentTheme::TextMuted, "SOCIAL OVERVIEW");

                float count_w = ImGui::CalcTextSize(friends_count_text).x;
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - count_w - 18.0f);
                ImGui::TextColored(RedAccentTheme::TextDim, "%s", friends_count_text);
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            ImGui::Dummy(ImVec2(0.0f, 8.0f));

            if (friends_root_tab == 0) {
                if (BeginSteamPanel("ProfileCard", ICON_FA_USER " MY PROFILE", ImVec2(0, 0))) {
                    const std::string local_name = settings->get_local_name();
                    const float avatar_size = 104.0f;

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::BgElevated);
                    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::BorderSoft);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 14.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 18.0f));

                    if (ImGui::BeginChild("ProfileHeroCard", ImVec2(0, 240), true, ImGuiWindowFlags_NoScrollbar)) {
                        ImGui::BeginGroup();
                        DrawEditableProfileAvatar(this, local_name, avatar_size);
                        ImGui::EndGroup();

                        ImGui::SameLine(0.0f, 18.0f);

                        ImGui::BeginGroup();
                        ImGui::Dummy(ImVec2(0.0f, 6.0f));
                        ImGui::TextColored(RedAccentTheme::Text, "%s", local_name.c_str());
                        ImGui::Spacing();
                        ImVec4 status_col = OverlayPresenceColor();
                        const char* status_icon = OverlayPresenceIcon();
                        const char* status_text = OverlayPresenceLabels[(int)g_overlay_presence];

                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(status_col.x, status_col.y, status_col.z, 0.14f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(status_col.x, status_col.y, status_col.z, 0.42f));
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 7.0f));

                        if (ImGui::BeginChild("##ProfileStatusBadge", ImVec2(170.0f, 34.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
                            ImGui::TextColored(status_col, "%s", status_icon);
                            ImGui::SameLine(0.0f, 8.0f);
                            ImGui::TextColored(RedAccentTheme::Text, "%s", status_text);
                        }
                        ImGui::EndChild();

                        ImGui::PopStyleVar(2);
                        ImGui::PopStyleColor(2);
                        ImGui::TextColored(RedAccentTheme::TextDim, "SteamID64: %" PRIu64, settings->get_local_steam_id().ConvertToUint64());
                        ImGui::TextColored(RedAccentTheme::TextDim, "%s: %s", translationLanguage[current_language], valid_languages[current_language]);
                        ImGui::Spacing();

                        std::string change_avatar_btn = std::string(ICON_FA_PEN " ") + translationChangeAvatar[current_language];
                        if (SteamButton(change_avatar_btn.c_str(), ImVec2(170.0f, 34.0f), false)) {
                            InitializeAvatarPickerDir(settings);
                            g_open_avatar_picker_popup = true;
                        }

                        ImGui::EndGroup();
                    }
                    ImGui::EndChild();

                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                    ImGui::TextColored(
                        g_allow_direct_join ? ImVec4(0.42f, 0.85f, 0.52f, 1.0f) : RedAccentTheme::Accent,
                        "%s",
                        g_allow_direct_join ? translationDirectJoinEnabled[current_language] : translationDirectJoinDisabled[current_language]
                    );

                    if (g_overlay_presence == OverlayPresenceState::Online) {
                        ImGui::TextColored(RedAccentTheme::TextDim, "All invites allowed and popup notifications enabled.");
                    }
                    else if (g_overlay_presence == OverlayPresenceState::Idle) {
                        ImGui::TextColored(RedAccentTheme::TextDim, "Invites allowed, but popup notifications are muted.");
                    }
                    else {
                        ImGui::TextColored(RedAccentTheme::TextDim, "No incoming invites accepted and no popup notifications.");
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(RedAccentTheme::TextMuted, "JOIN PRIVACY");
                    ImGui::Spacing();

                    bool old_allow_direct_join_profile = g_allow_direct_join;

                    if (g_overlay_presence == OverlayPresenceState::Offline) {
                        ImGui::BeginDisabled();
                    }

                    DrawSetting("Allow Direct Join", &g_allow_direct_join);

                    if (g_overlay_presence == OverlayPresenceState::Offline) {
                        ImGui::EndDisabled();
                    }

                    if (old_allow_direct_join_profile != g_allow_direct_join) {
                        SetDirectJoinEnabled(g_allow_direct_join);
                        ApplyDirectJoinPrivacyNow();
                    }

                    ImGui::Spacing();
                    ImGui::TextColored(
                        RedAccentTheme::TextDim,
                        g_overlay_presence == OverlayPresenceState::Offline
                        ? translationDirectJoinForcedOffOffline[current_language]
                        : (g_allow_direct_join
                            ? translationDirectJoinFriendsCanJoin[current_language]
                            : translationDirectJoinHidden[current_language])
                    );

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::TextColored(RedAccentTheme::TextMuted, "CHANGE PRESENCE");
                    ImGui::Spacing();

                    int presence_idx = static_cast<int>(g_overlay_presence);
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::BeginCombo("##profile_presence", OverlayPresenceLabels[presence_idx])) {
                        for (int i = 0; i < IM_ARRAYSIZE(OverlayPresenceLabels); ++i) {
                            const bool selected = (presence_idx == i);
                            if (ImGui::Selectable(OverlayPresenceLabels[i], selected)) {
                                presence_idx = i;
                                g_overlay_presence = static_cast<OverlayPresenceState>(i);

                                if (g_overlay_presence == OverlayPresenceState::Offline) {
                                    SetDirectJoinEnabled(false);
                                }
                                else {
                                    SaveGBEConfig();
                                }

                                ApplyDirectJoinPrivacyNow();
                            }

                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                EndSteamPanel();
            }

            if (friends_root_tab == 1) {
                static char friend_search[128] = { 0 };
                static int friend_sort_mode = 0;
                static int friend_filter_mode = 0;

                if (BeginSteamPanel("FriendsListCard", translationFriends[current_language], ImVec2(0, 0))) {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

                    ImGui::BeginGroup();
                    ImGui::TextColored(RedAccentTheme::TextMuted, "ONLINE FRIENDS");
                    ImGui::SameLine();

                    float right_controls_w = 330.0f;
                    float start_x = ImGui::GetWindowWidth() - right_controls_w - 20.0f;
                    if (start_x < ImGui::GetCursorPosX() + 20.0f) {
                        start_x = ImGui::GetCursorPosX() + 20.0f;
                    }

                    ImGui::SetCursorPosX(start_x);
                    ImGui::SetNextItemWidth(160.0f);
                    ImGui::InputTextWithHint("##friend_search_inline", "Search friends...", friend_search, sizeof(friend_search));

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(78.0f);
                    const char* sort_items[] = { "A-Z", "Join", "Game" };
                    ImGui::Combo("##friend_sort_inline", &friend_sort_mode, sort_items, IM_ARRAYSIZE(sort_items));

                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(84.0f);
                    const char* filter_items[] = { "All", "Join", "Invite", "Pending" };
                    ImGui::Combo("##friend_filter_inline", &friend_filter_mode, filter_items, IM_ARRAYSIZE(filter_items));
                    ImGui::EndGroup();

                    ImGui::Spacing();
                    ImGui::PopStyleVar();

                    float header_right_x = ImGui::GetWindowWidth() - 150.0f;

                    if (i_have_lobby) {
                        std::string inviteAll(translationInviteAll[current_language]);
                        inviteAll.append("##PopupInviteAllFriends");

                        ImGui::SameLine();
                        ImGui::SetCursorPosX(header_right_x - 110.0f);
                        if (SteamButton(inviteAll.c_str(), ImVec2(110, 30), true)) {
                            invite_all_friends_clicked = true;
                        }
                    }

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 28.0f);
                    ImGui::TextColored(RedAccentTheme::TextDim, "%d", (int)friends.size());

                    ImGui::Spacing();

                    std::vector<std::reference_wrapper<std::pair<Friend const, friend_window_state>>> visible_friends;
                    visible_friends.reserve(friends.size());

                    for (auto& kv : friends) {
                        if (IsUserBlocked((uint64)kv.first.id())) {
                            continue;
                        }

                        const bool has_pending_invite =
                            (kv.second.window_state & window_state_lobby_invite) ||
                            (kv.second.window_state & window_state_rich_invite);

                        const char* join_locked = get_steam_client()->steam_friends->get_friend_rich_presence_silent((uint64)kv.first.id(), kOverlayJoinPrivacyKey);
                        const bool invite_only = (join_locked && join_locked[0] == '1');
                        const bool same_game = (kv.first.appid() == settings->get_local_game_id().AppID());

                        bool passes_filter = true;
                        switch (friend_filter_mode) {
                        case 1: passes_filter = kv.second.joinable; break;
                        case 2: passes_filter = invite_only; break;
                        case 3: passes_filter = has_pending_invite; break;
                        default: break;
                        }

                        if (!passes_filter) continue;

                        std::string lower_name = kv.first.name();
                        std::string lower_query = friend_search;
                        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), [](unsigned char c) { return (char)std::tolower(c); });
                        std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), [](unsigned char c) { return (char)std::tolower(c); });

                        if (!lower_query.empty() && lower_name.find(lower_query) == std::string::npos) {
                            continue;
                        }

                        visible_friends.push_back(kv);
                    }

                    std::sort(visible_friends.begin(), visible_friends.end(),
                        [&](auto& a_ref, auto& b_ref) {
                            auto& a = a_ref.get();
                            auto& b = b_ref.get();

                            const bool a_same_game = (a.first.appid() == settings->get_local_game_id().AppID());
                            const bool b_same_game = (b.first.appid() == settings->get_local_game_id().AppID());

                            if (friend_sort_mode == 1 && a.second.joinable != b.second.joinable) {
                                return a.second.joinable > b.second.joinable;
                            }

                            if (friend_sort_mode == 2 && a_same_game != b_same_game) {
                                return a_same_game > b_same_game;
                            }

                            return a.first.name() < b.first.name();
                        });

                    if (!visible_friends.empty()) {
                        if (ImGui::BeginListBox("##friends_listbox", ImVec2(-1.0f, -1.0f))) {
                            for (auto& item_ref : visible_friends) {
                                auto& i = item_ref.get();

                                ImGui::PushID(i.second.id - base_friend_window_id + base_friend_item_id);
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));

                                ImGui::Selectable(
                                    ("##friend_row_" + std::to_string(i.second.id)).c_str(),
                                    false,
                                    ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns,
                                    ImVec2(0.0f, 60.0f)
                                );

                                ImRect row_rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
                                ImDrawList* row_dl = ImGui::GetWindowDrawList();

                                const bool has_pending_invite =
                                    (i.second.window_state & window_state_lobby_invite) ||
                                    (i.second.window_state & window_state_rich_invite);

                                const char* join_locked = get_steam_client()->steam_friends->get_friend_rich_presence_silent((uint64)i.first.id(), kOverlayJoinPrivacyKey);
                                const bool invite_only = (join_locked && join_locked[0] == '1');
                                const bool same_game = (i.first.appid() == settings->get_local_game_id().AppID());
                                const bool show_as_joinable = i.second.joinable && !invite_only;
                                ImVec4 row_status_col = GetFriendRowStatusColor(show_as_joinable, invite_only, has_pending_invite);
                                const char* state_label = GetFriendStateLabel(show_as_joinable, invite_only, has_pending_invite, same_game);
                                const char* presence_label = GetFriendPresenceLabel(i.first, i.second);

                                if (ImGui::IsItemHovered()) {
                                    row_dl->AddRectFilled(
                                        row_rect.Min,
                                        row_rect.Max,
                                        ImGui::GetColorU32(ImVec4(0.937f, 0.267f, 0.267f, 0.08f)),
                                        10.0f
                                    );
                                }

                                const std::string initials = GetInitialsFromName(i.first.name());
                                const ImU32 avatar_col = HashNameToColor(i.first.name());

                                row_dl->AddCircleFilled(
                                    ImVec2(row_rect.Min.x + 24.0f, row_rect.Min.y + 30.0f),
                                    17.0f,
                                    avatar_col
                                );

                                row_dl->AddText(
                                    ImVec2(row_rect.Min.x + 17.5f, row_rect.Min.y + 21.0f),
                                    ImGui::GetColorU32(ImVec4(1, 1, 1, 0.96f)),
                                    initials.c_str()
                                );

                                row_dl->AddCircleFilled(
                                    ImVec2(row_rect.Min.x + 45.0f, row_rect.Min.y + 42.0f),
                                    5.0f,
                                    ImGui::GetColorU32(row_status_col)
                                );

                                row_dl->AddText(
                                    ImVec2(row_rect.Min.x + 62.0f, row_rect.Min.y + 10.0f),
                                    ImGui::GetColorU32(RedAccentTheme::Text),
                                    i.first.name().c_str()
                                );

                                row_dl->AddText(
                                    ImVec2(row_rect.Min.x + 62.0f, row_rect.Min.y + 32.0f),
                                    ImGui::GetColorU32(RedAccentTheme::TextDim),
                                    presence_label
                                );

                                if (has_pending_invite) {
                                    const char* invite_hint = "Has invited you";
                                    ImVec2 invite_hint_size = ImGui::CalcTextSize(invite_hint);
                                    row_dl->AddText(
                                        ImVec2(row_rect.Max.x - invite_hint_size.x - 14.0f, row_rect.Min.y + 8.0f),
                                        ImGui::GetColorU32(ImVec4(0.95f, 0.78f, 0.28f, 0.95f)),
                                        invite_hint
                                    );
                                }

                                ImVec2 badge_size = ImGui::CalcTextSize(state_label);
                                row_dl->AddText(
                                    ImVec2(row_rect.Max.x - badge_size.x - 14.0f, row_rect.Min.y + 21.0f),
                                    ImGui::GetColorU32(row_status_col),
                                    state_label
                                );

                                build_friend_context_menu(i.first, i.second);

                                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                    i.second.window_state |= window_state_show;
                                }

                                ImGui::PopStyleVar();
                                ImGui::PopID();

                                build_friend_window(i.first, i.second);
                            }

                            ImGui::EndListBox();
                        }
                    }
                    else {
                        const char* empty_icon = ICON_FA_USER_GROUP;
                        const char* empty_title = "No friends to show";
                        const char* empty_desc = "Try a different filter or wait for friends to connect.";

                        float icon_w = ImGui::CalcTextSize(empty_icon).x;
                        float title_w = ImGui::CalcTextSize(empty_title).x;
                        float desc_w = ImGui::CalcTextSize(empty_desc).x;

                        float max_w = (std::max)((std::max)(icon_w, title_w), desc_w);

                        ImGui::Dummy(ImVec2(0.0f, 28.0f));
                        ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - max_w) * 0.5f + ImGui::GetCursorPosX());

                        ImGui::BeginGroup();
                        ImGui::SetCursorPosX((ImGui::GetCursorPosX() + (max_w - icon_w) * 0.5f));
                        ImGui::TextColored(RedAccentTheme::TextDim, "%s", empty_icon);

                        ImGui::SetCursorPosX((ImGui::GetCursorPosX() + (max_w - title_w) * 0.5f));
                        ImGui::TextColored(RedAccentTheme::Text, "%s", empty_title);

                        ImGui::SetCursorPosX((ImGui::GetCursorPosX() + (max_w - desc_w) * 0.5f));
                        ImGui::TextColored(RedAccentTheme::TextDim, "%s", empty_desc);
                        ImGui::EndGroup();
                    }
                }
                EndSteamPanel();
            }
            if (g_open_avatar_picker_popup) {
                std::string modal_title = std::string(translationSelectAvatarImage[current_language]) + "###Select Avatar Image";
                ImGui::OpenPopup(modal_title.c_str());
                g_open_avatar_picker_popup = false;
            }
            RenderAvatarPickerPopup(this);
        }
        EndModernWindow();
    }

    if (show_achievements || achievements_pinned) {
        if (BeginModernWindow((std::string(translationAchievements[current_language]) + "###AchievementsWindow").c_str(), ImVec2(640, 620), &show_achievements, current_language, &achievements_pinned, false, true)) {

            if (SteamButton(translationTestAchievement[current_language], ImVec2(220, 32), true)) {
                show_test_achievement();
            }

            ImGui::Spacing();

            if (achievements.size()) {
                ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationListOfAchievements[current_language]);
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::BeginChild("AchListChild", ImVec2(0, 0), false);

                // Build sorted index lists: unlocked by time desc, locked in API order
                std::vector<size_t> unlocked_idx, locked_idx;
                unlocked_idx.reserve(achievements.size());
                locked_idx.reserve(achievements.size());
                for (size_t i = 0; i < achievements.size(); ++i) {
                    if (achievements[i].achieved)
                        unlocked_idx.push_back(i);
                    else
                        locked_idx.push_back(i);
                }
                std::sort(unlocked_idx.begin(), unlocked_idx.end(),
                    [this](size_t a, size_t b) {
                        return achievements[a].unlock_time > achievements[b].unlock_time;
                    });

                auto render_ach = [this](Overlay_Achievement& x) {
                    bool has_table = false;
                    bool achieved = x.achieved;
                    bool hidden = x.hidden && !achieved;

                    try_load_ach_icon(x, true, settings->paginated_achievements_icons == 0);
                    try_load_ach_icon(x, false, settings->paginated_achievements_icons == 0);

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::BgElevated);
                    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);

                    std::string achCardId = "##ach_card_" + x.name;
                    ImGui::BeginChild(achCardId.c_str(), ImVec2(0, 110), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

                    if (x.icon->GetResourceId() != 0 || x.icon_gray->GetResourceId() != 0) {
                        if (ImGui::BeginTable(x.name.c_str(), 2, ImGuiTableFlags_SizingFixedFit)) {
                            has_table = true;

                            ImGui::TableSetupColumn("img", ImGuiTableColumnFlags_WidthFixed, settings->overlay_appearance.icon_size + 8.0f);
                            ImGui::TableSetupColumn("txt", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, settings->overlay_appearance.icon_size);

                            ImGui::TableSetColumnIndex(0);

                            auto& icon_rsrc = achieved ? x.icon : x.icon_gray;
                            if (icon_rsrc->GetResourceId() != 0) {
                                ImGui::Image(
                                    icon_rsrc->GetResourceId(),
                                    ImVec2(settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size)
                                );
                            }

                            ImGui::TableSetColumnIndex(1);
                        }
                    }

                    ImGui::TextColored(RedAccentTheme::Text, "%s", x.title.c_str());

                    if (hidden) {
                        ImGui::TextColored(RedAccentTheme::TextDim, "%s", translationHiddenAchievement[current_language]);
                        ImGui::SameLine();
                        ImGui::PushID(&x);
                        ImGui::SmallButton("Show");
                        bool show = ImGui::IsItemActive();
                        ImGui::PopID();
                        if (show) {
                            ImGui::PushFont(font_ach_desc);
                            ImGui::TextWrapped("%s", x.description.c_str());
                            ImGui::PopFont();
                        }
                    }
                    else {
                        ImGui::PushFont(font_ach_desc);
                        ImGui::TextWrapped("%s", x.description.c_str());
                        ImGui::PopFont();
                    }

                    if (achieved) {
                        char buffer[80]{};
                        time_t unlock_time = (time_t)x.unlock_time;
                        size_t written = std::strftime(
                            buffer,
                            sizeof(buffer),
                            settings->overlay_appearance.ach_unlock_datetime_format.c_str(),
                            std::localtime(&unlock_time)
                        );

                        if (!written) {
                            std::strftime(buffer, sizeof(buffer), "%Y/%m/%d - %H:%M:%S", std::localtime(&unlock_time));
                        }

                        ImGui::TextColored(ImVec4(0.42f, 0.85f, 0.52f, 1.0f),
                            translationAchievedOn[current_language], buffer);
                    }
                    else {
                        ImGui::TextColored(RedAccentTheme::Accent, "%s", translationNotAchieved[current_language]);
                    }

                    add_ach_progressbar(x);

                    if (has_table) {
                        ImGui::EndTable();
                        has_table = false;
                    }

                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);

                    ImGui::Spacing();
                };

                // --- Unlocked section ---
                if (ImGui::CollapsingHeader(translationUnlocked[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (unlocked_idx.empty()) {
                        ImGui::TextDisabled(translationNoUnlockedAchievements[current_language]);
                        ImGui::Spacing();
                    } else {
                        for (auto idx : unlocked_idx) {
                            render_ach(achievements[idx]);
                        }
                    }
                }

                // --- Locked section ---
                if (ImGui::CollapsingHeader(translationLocked[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (locked_idx.empty()) {
                        ImGui::TextDisabled(translationAllAchievementsUnlocked[current_language]);
                        ImGui::Spacing();
                    } else {
                        for (auto idx : locked_idx) {
                            render_ach(achievements[idx]);
                        }
                    }
                }

                ImGui::EndChild();
            }
            else {
                ImGui::TextColored(RedAccentTheme::TextDim, "%s", translationNoAchievementsAvailable[current_language]);
            }
        }
        EndModernWindow();
    }

    if (show_settings || settings_pinned) {
        if (BeginModernWindow((std::string(translationSettings[current_language]) + "###SettingsWindow").c_str(), ImVec2(900, 640), &show_settings, current_language, &settings_pinned, false, true)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));

            if (ImGui::BeginTable("SettingsGrid", 2, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("SettingsSidebar", ImGuiTableColumnFlags_WidthFixed, 210.0f);
                ImGui::TableSetupColumn("SettingsContent", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                {
                    std::string categories_title = std::string(ICON_FA_LIST " ") + translationCategories[current_language];
                    if (BeginSteamPanel("SettingsSidebarCard", categories_title.c_str(), ImVec2(0, 0),
                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
                    {
                        ImGui::Dummy(ImVec2(0.0f, 2.0f));

                        if (SteamButton(translationGeneral[current_language], ImVec2(-1, 38), settings_tab == 0)) settings_tab = 0;
                        if (SteamButton(translationHudElements[current_language], ImVec2(-1, 38), settings_tab == 1)) settings_tab = 1;
                        if (SteamButton(translationBroadcasts[current_language], ImVec2(-1, 38), settings_tab == 2)) settings_tab = 2;
                        if (SteamButton(translationBackgroundFx[current_language], ImVec2(-1, 38), settings_tab == 3)) settings_tab = 3;
                    }
                    EndSteamPanel();
                }

                ImGui::TableSetColumnIndex(1);
                {
                    if (settings_tab == 0) {
                        std::string general_title = std::string(ICON_FA_SLIDERS " ") + translationGeneral[current_language];
                        if (BeginSteamPanel("GeneralCard", general_title.c_str(), ImVec2(0, 0))) {
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));
                            if (ImGui::CollapsingHeader(translationProfileSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::TextWrapped("%s", translationGlobalSettingsWindowDescription[current_language]);
                                ImGui::Spacing();

                                bool has_username_row = ImGui::BeginTable("UsernameRow", 2, ImGuiTableFlags_SizingStretchSame);
                                if (has_username_row) {
                                    ImGui::TableNextColumn();
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::Text("%s", translationUsername[current_language]);

                                    ImGui::TableNextColumn();
                                    ImGui::SetNextItemWidth(-1);
                                    ImGui::InputText("##username", username_text, sizeof(username_text), 0);

                                    ImGui::EndTable();
                                }

                                ImGui::Spacing();

                                bool has_language_row = ImGui::BeginTable("LanguageRow", 2, ImGuiTableFlags_SizingStretchSame);
                                if (has_language_row) {
                                    ImGui::TableNextColumn();
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::Text("%s", translationLanguage[current_language]);

                                    ImGui::TableNextColumn();
                                    ImGui::SetNextItemWidth(-1);

                                    if (ImGui::BeginCombo("##language_combo", valid_languages[selected_language])) {
                                        for (int i = 0; i < static_cast<int>(sizeof(valid_languages) / sizeof(valid_languages[0])); ++i) {
                                            const bool selected = (selected_language == i);
                                            if (ImGui::Selectable(valid_languages[i], selected)) {
                                                selected_language = i;
                                            }
                                            if (selected) {
                                                ImGui::SetItemDefaultFocus();
                                            }
                                        }
                                        ImGui::EndCombo();
                                    }

                                    ImGui::EndTable();
                                }

                                ImGui::Spacing();
                                ImGui::TextColored(
                                    RedAccentTheme::TextDim,
                                    translationSelectedLanguage[current_language],
                                    valid_languages[selected_language]
                                );
                            }

                            ImGui::Spacing();

                            if (ImGui::CollapsingHeader(translationNotes[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                ImGui::TextColored(
                                    RedAccentTheme::Accent,
                                    "%s",
                                    translationRestartTheGameToApply[current_language]
                                );
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            if (SteamButton(translationSave[current_language], ImVec2(-1, 36), true)) {
                                save_settings = true;
                            }
                        }
                        ImGui::PopStyleVar();
                        EndSteamPanel();
                    }

                    if (settings_tab == 1) {
                        std::string hud_title = std::string(ICON_FA_CHART_SIMPLE " ") + translationHudElements[current_language];
                        if (BeginSteamPanel(
                            "HUDCard",
                            hud_title.c_str(),
                            ImVec2(0, 0)
                        )) {
                            if (ImGui::CollapsingHeader(translationOverlayStats[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                bool old_show_fps = stats.show_fps;
                                bool old_show_frametime = stats.show_frametime;
                                bool old_show_playtime = stats.show_playtime;
                                bool old_show_clock_hud = g_show_clock_hud;
                                bool old_use_24h_clock = g_use_24h_clock;
                                bool old_hide_hud_when_overlay_open = g_hide_hud_when_overlay_open;

                                if (DrawSetting(translationFpsCheckbox[current_language], &stats.show_fps)) {
                                    if (stats.show_fps) allow_renderer_frame_processing(true);
                                    else allow_renderer_frame_processing(false);
                                }
                                ImGui::Spacing();

                                if (DrawSetting(translationFrametimeCheckbox[current_language], &stats.show_frametime)) {
                                    if (stats.show_frametime) allow_renderer_frame_processing(true);
                                    else allow_renderer_frame_processing(false);
                                }
                                ImGui::Spacing();

                                if (DrawSetting(translationPlaytimeCheckbox[current_language], &stats.show_playtime)) {
                                    if (stats.show_playtime) allow_renderer_frame_processing(true);
                                    else allow_renderer_frame_processing(false);
                                }
                                ImGui::Spacing();

                                if (DrawSetting(translationSystemClock[current_language], &g_show_clock_hud)) {
                                    if (g_show_clock_hud) allow_renderer_frame_processing(true);
                                    else allow_renderer_frame_processing(false);
                                }
                                ImGui::Spacing();

                                DrawSetting(translation24HourFormat[current_language], &g_use_24h_clock);
                                ImGui::Spacing();

                                DrawSetting(translationHideHudWhenOverlayOpen[current_language], &g_hide_hud_when_overlay_open);

                                if (old_show_fps != stats.show_fps ||
                                    old_show_frametime != stats.show_frametime ||
                                    old_show_playtime != stats.show_playtime ||
                                    old_show_clock_hud != g_show_clock_hud ||
                                    old_use_24h_clock != g_use_24h_clock ||
                                    old_hide_hud_when_overlay_open != g_hide_hud_when_overlay_open)
                                {
                                    SaveGBEConfig();
                                }
                            }
                        }
                        EndSteamPanel();
                    }

                    if (settings_tab == 2) {
                        std::string broadcasts_title = std::string(ICON_FA_TOWER_BROADCAST " ") + translationBroadcasts[current_language];
                        if (BeginSteamPanel("BroadcastsCard", broadcasts_title.c_str(), ImVec2(0, 0))) {
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));

                            static bool broadcasts_loaded = false;
                            static std::string broadcasts_text;
                            static std::vector<std::string> broadcasts_list;
                            static std::filesystem::file_time_type broadcasts_last_write_time{};
                            static char broadcast_input[128] = { 0 };
                            static int broadcast_selected = -1;

                            const std::filesystem::path broadcasts_path = GetBroadcastsPath();

                            if (!broadcasts_loaded) {
                                EnsureBroadcastsFile(broadcasts_path);
                                broadcasts_text = LoadBroadcastsText(broadcasts_path);
                                broadcasts_list = SplitLines(broadcasts_text);

                                std::error_code ec;
                                broadcasts_last_write_time = std::filesystem::last_write_time(broadcasts_path, ec);
                                if (ec) {
                                    broadcasts_last_write_time = std::filesystem::file_time_type::min();
                                }

                                broadcasts_loaded = true;
                            }

                            {
                                std::error_code ec;
                                auto current_write_time = std::filesystem::last_write_time(broadcasts_path, ec);

                                if (!ec && current_write_time != broadcasts_last_write_time) {
                                    broadcasts_text = LoadBroadcastsText(broadcasts_path);
                                    broadcasts_list = SplitLines(broadcasts_text);
                                    broadcasts_last_write_time = current_write_time;
                                    g_custom_broadcasts_dirty = true;

                                    if (broadcast_selected >= static_cast<int>(broadcasts_list.size())) {
                                        broadcast_selected = -1;
                                    }
                                }
                            }

                            ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationBroadcastListenPort[current_language]);
                            ImGui::SetNextItemWidth(160.0f);

                            if (ImGui::InputText("##BroadcastListenPort", g_broadcast_port_text, sizeof(g_broadcast_port_text), ImGuiInputTextFlags_CharsDecimal)) {
                                g_broadcast_port_last_edit_time = ImGui::GetTime();
                                g_broadcast_port_dirty = true;
                            }
                            ImGui::SameLine();
                            ImGui::TextColored(RedAccentTheme::TextDim, "%s", translationGbePort[current_language]);

                            ImGui::Spacing();

                            ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationAddIpLabel[current_language]);
                            ImGui::SetNextItemWidth(-1);
                            ImGui::InputTextWithHint("##broadcast_ip_input", "e.g. 192.168.1.50", broadcast_input, sizeof(broadcast_input));

                            if (SteamButton(translationAddIp[current_language], ImVec2(120, 32), true)) {
                                std::string candidate = TrimString(broadcast_input);
                                if (!candidate.empty()) {
                                    bool exists = std::find(broadcasts_list.begin(), broadcasts_list.end(), candidate) != broadcasts_list.end();
                                    if (!exists) {
                                        broadcasts_list.push_back(candidate);
                                        broadcasts_text = JoinLines(broadcasts_list);
                                        SaveBroadcastsText(broadcasts_path, broadcasts_text);

                                        std::error_code ec;
                                        broadcasts_last_write_time = std::filesystem::last_write_time(broadcasts_path, ec);
                                        if (ec) {
                                            broadcasts_last_write_time = std::filesystem::file_time_type::min();
                                        }

                                        g_custom_broadcasts_dirty = true;
                                        broadcast_input[0] = '\0';
                                    }
                                }
                            }

                            ImGui::Spacing();
                            ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationCurrentList[current_language]);

                            ImGui::BeginChild(
                                "BroadcastsList",
                                ImVec2(-1.0f, ImGui::GetContentRegionAvail().y - 78.0f),
                                true,
                                ImGuiWindowFlags_NoScrollWithMouse
                            );
                            for (size_t i = 0; i < broadcasts_list.size(); ++i) {
                                bool selected = (broadcast_selected == static_cast<int>(i));
                                if (ImGui::Selectable(broadcasts_list[i].c_str(), selected)) {
                                    broadcast_selected = static_cast<int>(i);
                                }
                            }
                            ImGui::EndChild();

                            ImGui::Spacing();

                            const float remove_btn_w = 172.0f;
                            const float save_btn_w = 140.0f;
                            const float footer_btn_h = 34.0f;
                            const float footer_gap = 12.0f;
                            const float footer_total_w = remove_btn_w + save_btn_w + footer_gap;
                            const float footer_start_x = (ImGui::GetContentRegionAvail().x - footer_total_w) * 0.5f;

                            if (footer_start_x > 0.0f) {
                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + footer_start_x);
                            }

                            const char* remove_selected_txt = translationRemoveSelected[current_language];
                            const char* remove_all_txt = translationRemoveAll[current_language];

                            float btn_h = 32.0f;
                            float gap = 10.0f;

                            float remove_selected_w = ImGui::CalcTextSize(remove_selected_txt).x + 28.0f;
                            float remove_all_w = ImGui::CalcTextSize(remove_all_txt).x + 28.0f;

                            float total_w = remove_selected_w + remove_all_w + gap;
                            float full_w = ImGui::GetWindowSize().x;
                            float cursor_x = ImGui::GetCursorPosX();

                            float padding = ImGui::GetStyle().WindowPadding.x * 2.0f;

                            float start_x = (full_w - padding - total_w) * 0.5f - cursor_x;

                            if (start_x > 0.0f)
                                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

                            if (SteamButton(remove_selected_txt, ImVec2(remove_selected_w, btn_h), false)) {
                                if (broadcast_selected >= 0 && broadcast_selected < (int)broadcasts_list.size()) {
                                    broadcasts_list.erase(broadcasts_list.begin() + broadcast_selected);
                                    broadcast_selected = -1;
                                    broadcasts_text = JoinLines(broadcasts_list);
                                    SaveBroadcastsText(broadcasts_path, broadcasts_text);
                                    g_custom_broadcasts_dirty = true;
                                }
                            }

                            ImGui::SameLine(0.0f, gap);

                            if (SteamButton(remove_all_txt, ImVec2(remove_all_w, btn_h), false)) {
                                if (!broadcasts_list.empty()) {
                                    broadcasts_list.clear();
                                    broadcast_selected = -1;
                                    broadcasts_text.clear();
                                    SaveBroadcastsText(broadcasts_path, broadcasts_text);
                                    g_custom_broadcasts_dirty = true;
                                }
                            }
                        }
                        ImGui::PopStyleVar();
                        EndSteamPanel();
                    }

                    if (settings_tab == 3) {
                        std::string bg_fx_title = std::string(ICON_FA_WAND_MAGIC_SPARKLES " ") + translationBackgroundFx[current_language];
                        if (BeginSteamPanel("BackgroundFxCard", bg_fx_title.c_str(), ImVec2(0, 0))) {
                            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));

                            DrawSetting(translationEnableBgEffects[current_language], &g_bg_fx_settings.enabled);
                            ImGui::Spacing();

                            if (g_bg_fx_settings.enabled) {
                                ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationEffectType[current_language]);
                                const char* fx_types[] = {
                                    translationFxSnow[current_language],
                                    translationFxRain[current_language],
                                    translationFxParticles[current_language],
                                    translationFxStars[current_language],
                                    translationFxBubbles[current_language]
                                };
                                ImGui::SetNextItemWidth(-1);
                                if (ImGui::BeginCombo("##fx_type", fx_types[g_bg_fx_settings.type])) {
                                    for (int i = 0; i < 5; ++i) {
                                        if (ImGui::Selectable(fx_types[i], g_bg_fx_settings.type == i)) {
                                            g_bg_fx_settings.type = i;
                                            SaveGBEConfig();
                                        }
                                    }
                                    ImGui::EndCombo();
                                }
                                ImGui::Spacing();

                                if (ImGui::CollapsingHeader(translationParticleSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                    ImGui::SliderInt(translationParticleCount[current_language], &g_bg_fx_settings.particle_count, 10, g_bg_fx_settings.max_particles);
                                    ImGui::SliderFloat(translationParticleSpeed[current_language], &g_bg_fx_settings.particle_speed, 0.2f, 3.0f);
                                    ImGui::SliderFloat(translationMinSize[current_language], &g_bg_fx_settings.particle_size_min, 0.5f, 5.0f);
                                    ImGui::SliderFloat(translationMaxSize[current_language], &g_bg_fx_settings.particle_size_max, 1.0f, 8.0f);
                                    ImGui::SliderFloat(translationParticleAlpha[current_language], &g_bg_fx_settings.particle_alpha, 0.05f, 0.8f);
                                }

                                if (ImGui::CollapsingHeader(translationColorSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                    ImGui::ColorEdit4(translationParticleColor[current_language], (float*)&g_bg_fx_settings.particle_color);
                                    ImGui::SliderFloat(translationColorVariation[current_language], &g_bg_fx_settings.color_variation, 0.0f, 0.8f);
                                }

                                if (ImGui::CollapsingHeader(translationMovementSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                    ImGui::SliderFloat(translationWindStrength[current_language], &g_bg_fx_settings.wind_strength, -2.0f, 2.0f);
                                    ImGui::SliderFloat(translationTurbulence[current_language], &g_bg_fx_settings.turbulence, 0.0f, 1.5f);
                                    ImGui::SliderFloat(translationSwirlIntensity[current_language], &g_bg_fx_settings.swirl_intensity, 0.0f, 2.0f);
                                }

                                if (g_bg_fx_settings.type == 0) {
                                    if (ImGui::CollapsingHeader(translationSnowSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                        ImGui::SliderFloat(translationSnowDrift[current_language], &g_bg_fx_settings.snow_drift, 0.0f, 40.0f);
                                    }
                                }
                                else if (g_bg_fx_settings.type == 1) {
                                    if (ImGui::CollapsingHeader(translationRainSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                        ImGui::SliderFloat(translationRainLength[current_language], &g_bg_fx_settings.rain_length, 5.0f, 25.0f);
                                        ImGui::SliderFloat(translationRainTilt[current_language], &g_bg_fx_settings.rain_tilt, 0.0f, 15.0f);
                                    }
                                }
                                else if (g_bg_fx_settings.type == 3) {
                                    if (ImGui::CollapsingHeader(translationStarSettings[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                        ImGui::SliderFloat(translationStarTwinkleSpeed[current_language], &g_bg_fx_settings.star_twinkle_speed, 0.5f, 5.0f);
                                    }
                                }

                                if (ImGui::CollapsingHeader(translationPerformance[current_language], ImGuiTreeNodeFlags_DefaultOpen)) {
                                    DrawSetting(translationOptimizePerformance[current_language], &g_bg_fx_settings.use_optimization);
                                    if (g_bg_fx_settings.use_optimization) {
                                        ImGui::SliderInt(translationMaxParticlesPerformance[current_language], &g_bg_fx_settings.max_particles, 50, 300);
                                        if (g_bg_fx_settings.particle_count > g_bg_fx_settings.max_particles) {
                                            g_bg_fx_settings.particle_count = g_bg_fx_settings.max_particles;
                                        }
                                    }
                                }

                                ImGui::Spacing();
                                ImGui::Separator();
                                ImGui::Spacing();

                                ImGui::TextColored(RedAccentTheme::TextMuted, "%s", translationPreviewInfo[current_language]);
                                ImGui::TextColored(RedAccentTheme::TextDim, translationActiveEffect[current_language], fx_types[g_bg_fx_settings.type]);
                                ImGui::TextColored(RedAccentTheme::TextDim, translationParticlesCount[current_language], g_bg_fx_settings.particle_count, g_bg_fx_settings.max_particles);

                                ImGui::Spacing();
                                if (SteamButton(translationResetToDefaults[current_language], ImVec2(180, 34), false)) {
                                    g_bg_fx_settings = BackgroundFxSettings();
                                    SaveGBEConfig();
                                }
                            }

                            ImGui::PopStyleVar();
                        }
                        EndSteamPanel();
                    }
                }

                ImGui::EndTable();
            }

            ImGui::PopStyleVar();
        }
        EndModernWindow();
    }

    if (show_notification_history || history_pinned) {
        if (BeginModernWindow((std::string(translationNotificationHistory[current_language]) + "###NotificationHistoryWindow").c_str(), ImVec2(560, 480), &show_notification_history, current_language, &history_pinned, false, true)) {
            if (SteamButton("Clear All", ImVec2(120, 32), false)) {
                notification_history.clear();
                notification_history_cache.clear();
                notification_history_cache_dirty = false;
            }
            ImGui::Separator();
            ImGui::Spacing();

            if (notification_history.empty()) {
                ImGui::TextDisabled("No notifications yet");
            } else {
                ImGui::BeginChild("##history_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);

                // Rebuild cache only when history actually changes
                if (notification_history_cache_dirty) {
                    notification_history_cache.clear();
                    notification_history_cache.reserve(notification_history.size());

                    for (auto it = notification_history.rbegin(); it != notification_history.rend(); ++it) {
                        // Format timestamp HH:MM:SS in local timezone
                        const time_t total_sec = std::chrono::duration_cast<std::chrono::seconds>(it->timestamp).count();
                        struct tm local_tm_buf{};
#ifdef _MSC_VER
                        localtime_s(&local_tm_buf, &total_sec);
#else
                        localtime_r(&total_sec, &local_tm_buf);
#endif
                        const auto hr = local_tm_buf.tm_hour;
                        const auto min = local_tm_buf.tm_min;
                        const auto sec = local_tm_buf.tm_sec;

                        // Type label
                        const char *type_label = "?";
                        switch ((notification_type)it->type) {
                            case notification_type::message: type_label = "Chat"; break;
                            case notification_type::invite: type_label = "Invite"; break;
                            case notification_type::achievement: type_label = "Achievement"; break;
                            case notification_type::achievement_progress: type_label = "Progress"; break;
                            case notification_type::auto_accept_invite: type_label = "Auto-Invite"; break;
                        }

                        // For achievements the message contains "title\ndescription"
                        // Replace newline with inline separator for compact display
                        std::string display_msg = it->message;
                        if (it->type == static_cast<uint8>(notification_type::achievement) ||
                            it->type == static_cast<uint8>(notification_type::achievement_progress)) {
                            size_t pos = display_msg.find('\n');
                            if (pos != std::string::npos) {
                                display_msg.replace(pos, 1, " - ");
                            }
                        }

                        char time_buf[16];
                        snprintf(time_buf, sizeof(time_buf), "[%02d:%02d:%02d]", hr, min, sec);
                        std::string line = std::string(time_buf) + "  " + type_label + "  " + display_msg;

                        notification_history_cache.push_back(std::move(line));
                    }
                    notification_history_cache_dirty = false;
                }

                // Render from cache
                int id_counter = 0;
                for (const auto &line : notification_history_cache) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, RedAccentTheme::BgElevated);
                    ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
                    
                    std::string entryId = "##hist_entry_" + std::to_string(id_counter++);
                    ImGui::BeginChild(entryId.c_str(), ImVec2(0, 48), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
                    ImGui::TextWrapped("%s", line.c_str());
                    ImGui::EndChild();
                    
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);
                    ImGui::Spacing();
                }
                ImGui::EndChild();
            }
        }
        EndModernWindow();
    }

    if (show_url.size()) {
        std::string url = show_url;
        bool show = true;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, RedAccentTheme::BgPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

        ImGui::SetNextWindowBgAlpha(1.0f);
        if (ImGui::Begin(URL_WINDOW_NAME, &show)) {
            ImGui::Text("%s", translationSteamOverlayURL[current_language]);
            ImGui::Spacing();

            ImGui::PushItemWidth(-1);
            ImGui::InputText("##url_copy", (char*)url.data(), url.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopItemWidth();

            ImGui::Spacing();

            if (SteamButton(translationClose[current_language], ImVec2(120, 30), false) || !show) {
                show_url = "";
            }
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }


    bool show_warning = warn_local_save || warn_bad_appid;
    if (show_warning) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, RedAccentTheme::BgPopup);
        ImGui::PushStyleColor(ImGuiCol_Border, RedAccentTheme::Border);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(ImGui::GetFontSize() * 32, ImGui::GetFontSize() * 12),
            ImVec2(8192, 8192)
        );
        ImGui::SetNextWindowFocus();

        if (ImGui::Begin(translationWarning[current_language], &show_warning)) {
            if (warn_bad_appid) {
                ImGui::TextColored(RedAccentTheme::Accent, "%s", translationWarning[current_language]);
                ImGui::TextWrapped("%s", translationWarningDescription_badAppid[current_language]);
                ImGui::Spacing();
            }

            if (warn_local_save) {
                ImGui::TextColored(RedAccentTheme::Accent, "%s", translationWarning[current_language]);
                ImGui::TextWrapped("%s", translationWarningDescription_localSave[current_language]);
            }
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        if (!show_warning) {
            warn_local_save = false;
            warn_bad_appid = false;
        }
    }

    ImGui::PopFont();
}

void Steam_Overlay::load_next_ach_icon()
{
    if (!settings->overlay_upload_achs_icons_to_gpu || settings->paginated_achievements_icons <= 0 || achievements.empty()) return;

    size_t linear_idx = last_loaded_ach_icon / 2;
    if (linear_idx >= achievements.size()) {
        last_loaded_ach_icon = 0;
        linear_idx = 0;
    }

    auto& ach = achievements.at(linear_idx);
    ++last_loaded_ach_icon;

    bool achieved = last_loaded_ach_icon % 2 != 0;
    bool loaded = try_load_ach_icon(ach, achieved, true);
}

void Steam_Overlay::SetupOverlay()
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    bool not_called_yet = false;
    if (setup_overlay_called.compare_exchange_weak(not_called_yet, true)) {
        if (settings->overlay_hook_delay_sec > 0) {
            PRINT_DEBUG("waiting %i seconds", settings->overlay_hook_delay_sec);
            renderer_detector_delay_thread.start();
        }
        else {
            request_renderer_detector();
            set_renderer_hook_timeout();
            renderer_hook_init_thread.start();
        }
    }
}

void Steam_Overlay::UnSetupOverlay()
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    bool already_called = true;
    if (setup_overlay_called.compare_exchange_weak(already_called, false)) {
        is_ready = false;

        renderer_hook_init_thread.kill();
        renderer_detector_delay_thread.kill();

        if (_renderer) {
            _renderer->OverlayHookReady = [](InGameOverlay::OverlayHookState) {};
            _renderer->OverlayProc = []() {};

            allow_renderer_frame_processing(false, true);
            obscure_game_input(false, true);

            PRINT_DEBUG("releasing any images resources");
            for (auto& ach : achievements) {
                if (ach.icon->GetResourceId() != 0) {
                    ach.icon->Unload();
                }

                if (ach.icon_gray->GetResourceId() != 0) {
                    ach.icon_gray->Unload();
                }
            }

            _renderer->~RendererHook_t();
            _renderer = nullptr;
        }

        cleanup_renderer_hook();
    }

    PRINT_DEBUG("done *********");
}

bool Steam_Overlay::Ready() const
{
    return !settings->disable_overlay && is_ready && late_init_imgui;
}

bool Steam_Overlay::NeedPresent() const
{
    PRINT_DEBUG_ENTRY();
    return !settings->disable_overlay;
}

void Steam_Overlay::SetNotificationPosition(ENotificationPosition eNotificationPosition)
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG("TODO %i", (int)eNotificationPosition);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    notif_position = eNotificationPosition;
}

void Steam_Overlay::SetNotificationInset(int nHorizontalInset, int nVerticalInset)
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG("TODO x=%i y=%i", nHorizontalInset, nVerticalInset);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    h_inset = nHorizontalInset;
    v_inset = nVerticalInset;
}

void Steam_Overlay::OpenOverlayInvite(CSteamID lobbyId)
{
    PRINT_DEBUG("TODO %llu", lobbyId.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    ShowOverlay(true);
}

void Steam_Overlay::OpenOverlay(const char* pchDialog)
{
    PRINT_DEBUG("TODO '%s'", pchDialog);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    if ((strncmp(pchDialog, "Friends", sizeof("Friends") - 1) == 0) && (settings->overlayAutoAcceptInvitesCount() > 0)) {
        PRINT_DEBUG("won't open overlay's friends list because some friends are defined in the auto accept list");
        add_auto_accept_invite_notification();
    }
    else {
        ShowOverlay(true);
    }
}

void Steam_Overlay::OpenOverlayWebpage(const char* pchURL)
{
    PRINT_DEBUG("TODO '%s'", pchURL);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    show_url = pchURL;
    ShowOverlay(true);
}

bool Steam_Overlay::ShowOverlay() const
{
    return show_overlay;
}

void Steam_Overlay::ShowOverlay(bool state)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready() || show_overlay == state) return;
    if (state) {
        g_overlay_open_time = ImGui::GetTime();
    }

    show_overlay = state;
    overlay_state_changed = true;

    PRINT_DEBUG("%i", (int)state);

    if (state) {
        Steam_Overlay::allow_renderer_frame_processing(true);
        Steam_Overlay::obscure_game_input(true);
    }
    else {
        Steam_Overlay::allow_renderer_frame_processing(false);
        Steam_Overlay::obscure_game_input(false, true);
    }
}

void Steam_Overlay::SetLobbyInvite(Friend friendId, uint64 lobbyId)
{
    if (!OverlayPresenceAcceptsInvites()) return;
    if (IsUserBlocked((uint64)friendId.id())) return;
    PRINT_DEBUG("%" PRIu64 " %llu", friendId.id(), lobbyId);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    auto i = friends.find(friendId);
    if (i != friends.end())
    {
        auto& frd = i->second;
        frd.lobbyId = lobbyId;
        frd.window_state |= window_state_lobby_invite;
        frd.window_state &= ~window_state_rich_invite;
        add_invite_notification(*i);
        notify_sound_user_invite(i->second);
    }
}

void Steam_Overlay::SetRichInvite(Friend friendId, const char* connect_str)
{
    if (!OverlayPresenceAcceptsInvites()) return;
    if (IsUserBlocked((uint64)friendId.id())) return;
    PRINT_DEBUG("%" PRIu64 " '%s'", friendId.id(), connect_str);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready()) return;

    auto i = friends.find(friendId);
    if (i != friends.end())
    {
        auto& frd = i->second;
        strncpy(frd.connect, connect_str, k_cchMaxRichPresenceValueLength - 1);
        frd.window_state |= window_state_rich_invite;
        frd.window_state &= ~window_state_lobby_invite;
        add_invite_notification(*i);
        notify_sound_user_invite(i->second);
    }
}

void Steam_Overlay::FriendConnect(Friend _friend)
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG("%" PRIu64 "", _friend.id());
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    int id = find_free_friend_id(friends);
    if (id != 0) {
        auto& item = friends[_friend];
        item.window_title = std::move(_friend.name() + " " + translationPlaying[current_language] + " " + std::to_string(_friend.appid()));
        item.window_state = window_state_none;
        item.id = id;
        memset(item.chat_input, 0, max_chat_len);
        item.joinable = false;
    }
    else {
        PRINT_DEBUG("error no free id to create a friend window");
    }
}

void Steam_Overlay::FriendDisconnect(Friend _friend)
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG("%" PRIu64 "", _friend.id());
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

    auto it = friends.find(_friend);
    if (it != friends.end())
        friends.erase(it);
}

void Steam_Overlay::AddAchievementNotification(const std::string& ach_name, nlohmann::json const& ach, bool for_progress)
{
    if (settings->disable_overlay) return;

    PRINT_DEBUG("'%s' %i", ach_name.c_str(), (int)for_progress);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    for (auto& a : achievements) {
        if (a.name == ach_name) {
            try {
                std::lock_guard<std::recursive_mutex> lock2(global_mutex);

                a.achieved = ach.value("earned", false);
                a.unlock_time = ach.value("earned_time", static_cast<uint32>(0));
                a.progress = ach.value("progress", static_cast<uint32>(0));
                a.max_progress = ach.value("max_progress", static_cast<uint32>(0));
            }
            catch (...) {}

            // Only queue/show notification if overlay is ready
            if (!Ready()) return;

            if (a.achieved && !for_progress) {
                post_achievement_notification(a, for_progress);
            }
            else if (for_progress && !settings->disable_overlay_achievement_progress) {
                post_achievement_notification(a, for_progress);
            }
            break;
        }
    }
}

void Steam_Overlay::steam_run_callback_update_my_lobby()
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;

    if (std::string(steamFriends->get_friend_rich_presence_silent(settings->get_local_steam_id(), "connect")).length() > 0) {
        i_have_lobby = true;
    }
    else if (settings->get_lobby().IsValid()) {
        i_have_lobby = true;
    }
    else {
        i_have_lobby = false;
    }

    steamFriends->SetRichPresence(kOverlayJoinPrivacyKey, g_allow_direct_join ? "0" : "1");
}

bool Steam_Overlay::is_friend_joinable(std::pair<const Friend, friend_window_state>& f)
{
    PRINT_DEBUG("%" PRIu64 "", f.first.id());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;

    const char* join_locked = steamFriends->get_friend_rich_presence_silent((uint64)f.first.id(), kOverlayJoinPrivacyKey);
    if (join_locked && join_locked[0] == '1') {
        PRINT_DEBUG("%" PRIu64 " false (friend locked direct join)", f.first.id());
        return false;
    }

    if (std::string(steamFriends->get_friend_rich_presence_silent((uint64)f.first.id(), "connect")).length() > 0) {
        PRINT_DEBUG("%" PRIu64 " true (connect string)", f.first.id());
        return true;
    }

    FriendGameInfo_t friend_game_info{};
    steamFriends->GetFriendGamePlayed((uint64)f.first.id(), &friend_game_info);
    if (friend_game_info.m_steamIDLobby.IsValid() && (f.second.window_state & window_state_lobby_invite)) {
        PRINT_DEBUG("%" PRIu64 " true (friend in a game)", f.first.id());
        return true;
    }

    PRINT_DEBUG("%" PRIu64 " false", f.first.id());
    return false;
}

void Steam_Overlay::invite_friend(uint64 friend_id, class Steam_Friends* steamFriends, class Steam_Matchmaking* steamMatchmaking)
{
    std::string connect_str = steamFriends->get_friend_rich_presence_silent(settings->get_local_steam_id(), "connect");
    if (connect_str.length() > 0) {
        steamFriends->InviteUserToGame(friend_id, connect_str.c_str());
        PRINT_DEBUG("sent game invitation to friend with id = %llu", friend_id);
    }
    else if (settings->get_lobby().IsValid()) {
        steamMatchmaking->InviteUserToLobby(settings->get_lobby(), friend_id);
        PRINT_DEBUG("sent lobby invitation to friend with id = %llu", friend_id);
    }
}

void Steam_Overlay::steam_run_callback_friends_actions()
{
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;
    Steam_Matchmaking* steamMatchmaking = get_steam_client()->steam_matchmaking;

    std::for_each(friends.begin(), friends.end(), [this](std::pair<Friend const, friend_window_state>& i) {
        i.second.joinable = is_friend_joinable(i);
        });

    while (!has_friend_action.empty()) {
        auto friend_info = friends.find(has_friend_action.front());
        if (friend_info != friends.end()) {
            uint64 friend_id = (uint64)friend_info->first.id();
            if (friend_info->second.window_state & window_state_send_message) {
                char* input = friend_info->second.chat_input;
                char* end_input = input + strlen(input);
                char* printable_char = std::find_if(input, end_input, [](char c) { return std::isgraph(c); });

                if (printable_char != end_input) {
                    Common_Message msg;
                    Steam_Messages* steam_messages = new Steam_Messages;
                    steam_messages->set_type(Steam_Messages::FRIEND_CHAT);
                    steam_messages->set_message(friend_info->second.chat_input);
                    msg.set_allocated_steam_messages(steam_messages);
                    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
                    msg.set_dest_id(friend_id);
                    network->sendTo(&msg, true);

                    friend_info->second.chat_history.append(get_steam_client()->settings_client->get_local_name()).append(": ").append(input).append("\n", 1);
                }
                *input = 0;

                friend_info->second.window_state &= ~window_state_send_message;
            }
            if (friend_info->second.window_state & window_state_invite) {
                invite_friend(friend_id, steamFriends, steamMatchmaking);

                friend_info->second.window_state &= ~window_state_invite;
            }
            if (friend_info->second.window_state & window_state_join) {
                std::string connect = steamFriends->get_friend_rich_presence_silent(friend_id, "connect");
                if (friend_info->second.window_state & window_state_lobby_invite) {
                    GameLobbyJoinRequested_t data;
                    data.m_steamIDLobby.SetFromUint64(friend_info->second.lobbyId);
                    data.m_steamIDFriend.SetFromUint64(friend_id);
                    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

                    friend_info->second.window_state &= ~window_state_lobby_invite;
                }
                else {
                    if (friend_info->second.window_state & window_state_rich_invite) {
                        GameRichPresenceJoinRequested_t data = {};
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        strncpy(data.m_rgchConnect, friend_info->second.connect, k_cchMaxRichPresenceValueLength - 1);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

                        friend_info->second.window_state &= ~window_state_rich_invite;
                    }
                    else if (connect.length() > 0) {
                        GameRichPresenceJoinRequested_t data = {};
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        strncpy(data.m_rgchConnect, connect.c_str(), k_cchMaxRichPresenceValueLength - 1);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                    }
                    else if (connect.length() > 0) {
                        GameRichPresenceJoinRequested_t data = {};
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        strncpy(data.m_rgchConnect, connect.c_str(), k_cchMaxRichPresenceValueLength - 1);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                    }

                    // Not sure about this but it fixes sonic racing transformed invites
                    FriendGameInfo_t friend_game_info = {};
                    steamFriends->GetFriendGamePlayed(friend_id, &friend_game_info);
                    uint64 lobby_id = friend_game_info.m_steamIDLobby.ConvertToUint64();
                    if (lobby_id) {
                        GameLobbyJoinRequested_t data;
                        data.m_steamIDLobby.SetFromUint64(lobby_id);
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                    }
                }

                friend_info->second.window_state &= ~window_state_join;
            }
        }
        has_friend_action.pop();
    }
}

void Steam_Overlay::steam_run_callback()
{
    if (!Ready()) return;

    if (overlay_state_changed) {
        overlay_state_changed = false;

        GameOverlayActivated_t data{};
        data.m_bActive = show_overlay;
        data.m_bUserInitiated = true;
        data.m_dwOverlayPID = 123;
        data.m_nAppID = settings->get_local_game_id().AppID();
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }

    Steam_Friends* steamFriends = get_steam_client()->steam_friends;
    Steam_Matchmaking* steamMatchmaking = get_steam_client()->steam_matchmaking;

    if (save_settings) {
        save_settings = false;

        const char* language_text = valid_languages[selected_language];
        save_global_settings(get_steam_client()->local_storage, username_text, language_text);
        get_steam_client()->settings_client->set_local_name(username_text);
        get_steam_client()->settings_server->set_local_name(username_text);
        get_steam_client()->settings_client->set_language(language_text);
        get_steam_client()->settings_server->set_language(language_text);
        steamFriends->resend_friend_data();
    }

    bool broadcasts_dirty = true;
    if (g_custom_broadcasts_dirty.compare_exchange_strong(broadcasts_dirty, false)) {
        steamFriends->resend_friend_data();
    }

    steam_run_callback_update_my_lobby();

    bool yes_clicked = true;
    if (invite_all_friends_clicked.compare_exchange_weak(yes_clicked, false)) {
        PRINT_DEBUG("Steam_Overlay will send invitations to [%zu] friends if they're using the same app", friends.size());
        uint32 current_appid = settings->get_local_game_id().AppID();
        for (auto& fr : friends) {
            if (fr.first.appid() == current_appid) {
                uint64 friend_id = (uint64)fr.first.id();
                invite_friend(friend_id, steamFriends, steamMatchmaking);
            }
        }
    }

    if (overlay_mutex.try_lock()) {
        if (Ready()) {
            steam_run_callback_friends_actions();
        }
        overlay_mutex.unlock();
    }
}

void Steam_Overlay::networking_msg_received(Common_Message* msg)
{
    if (msg->has_steam_messages()) {
        std::lock_guard<std::recursive_mutex> lock(overlay_mutex);

        Friend frd;
        frd.set_id(msg->source_id());
        auto friend_info = friends.find(frd);
        if (friend_info != friends.end()) {
            const uint64_t sender_id = (uint64)friend_info->first.id();

            if (IsUserBlocked(sender_id)) {
                return;
            }

            if (IsUserMuted(sender_id)) {
                return;
            }

            Steam_Messages const& steam_message = msg->steam_messages();
            friend_info->second.chat_history.append(friend_info->first.name() + ": " + steam_message.message()).append("\n", 1);
            if (!(friend_info->second.window_state & window_state_show)) {
                friend_info->second.window_state |= window_state_need_attention;
            }

            if (OverlayPresenceAllowsPopups()) {
                add_chat_message_notification(friend_info->first.name() + " sent you a message");
            }
            notify_sound_user_invite(friend_info->second);
        }
    }
}

#endif // EMU_OVERLAY
