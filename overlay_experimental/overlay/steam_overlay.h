#ifndef __INCLUDED_STEAM_OVERLAY_H__
#define __INCLUDED_STEAM_OVERLAY_H__

#include "dll/base.h"
#include <map>
#include <queue>

#ifdef EMU_OVERLAY

#include <future>
#include <atomic>
#include <memory>
#include "InGameOverlay/RendererHook.h"
#include "InGameOverlay/ImGui/imgui.h"
#include "overlay/steam_overlay_stats.h"

static constexpr size_t max_chat_len = 768;

enum window_state
{
    window_state_none           = 0,
    window_state_show           = 1<<0,
    window_state_invite         = 1<<1,
    window_state_join           = 1<<2,
    window_state_lobby_invite   = 1<<3,
    window_state_rich_invite    = 1<<4,
    window_state_send_message   = 1<<5,
    window_state_need_attention = 1<<6,

};

struct friend_window_state
{
    int id;
    uint8 window_state;
    std::string window_title;
    union // The invitation (if any)
    {
        uint64 lobbyId;
        char connect[k_cchMaxRichPresenceValueLength];
    };
    std::string chat_history;
    char chat_input[max_chat_len];

    bool joinable;
};

struct Friend_Less
{
    bool operator()(const Friend& lhs, const Friend& rhs) const
    {
        return lhs.id() < rhs.id();
    }
};

enum class notification_type
{
    message,
    invite,
    achievement,
    achievement_progress,
    auto_accept_invite,
};

struct Overlay_Achievement
{
    std::string name{}; // internal schema name
    std::string title{}; // displayName
    std::string description{}; // description
    uint32 progress{};
    uint32 max_progress{};
    bool hidden{};
    bool achieved{};
    uint32 unlock_time{};
    InGameOverlay::RendererResource_t* icon{};
    InGameOverlay::RendererResource_t* icon_gray{};
    int icon_handle = Settings::UNLOADED_IMAGE_HANDLE;
    int icon_gray_handle = Settings::UNLOADED_IMAGE_HANDLE;
};

struct Notification
{
    static constexpr float width_percent = 0.25f; // percentage from total width
    static constexpr std::chrono::milliseconds default_show_time = std::chrono::milliseconds(6000);

    int id{};
    uint8 type{};
    bool expired = false;
    std::chrono::milliseconds start_time{};
    std::string message{};
    std::pair<const Friend, friend_window_state>* frd{};
    std::optional<Overlay_Achievement> ach{};
};

// notification coordinates { x, y }
struct NotificationsCoords
{
    std::pair<float, float> top_left{}, top_center{}, top_right{};
    std::pair<float, float> bot_left{}, bot_center{}, bot_right{};
};

class Steam_Overlay
{
    constexpr static const char ACH_SOUNDS_FOLDER[] = "sounds";
    constexpr static const int renderer_detector_polling_ms = 100;

    class Settings* settings;
    class Local_Storage* local_storage;
    class SteamCallResults* callback_results;
    class SteamCallBacks* callbacks;
    class RunEveryRunCB* run_every_runcb;
    class Networking* network;
    class Steam_Overlay_Stats stats;

    // friend id, show client window (to chat and accept invite maybe)
    std::map<Friend, friend_window_state, Friend_Less> friends{};

    bool is_ready = false;

    ENotificationPosition notif_position = ENotificationPosition::k_EPositionBottomLeft;
    int h_inset = 0;
    int v_inset = 0;
    std::string show_url{};

    std::vector<Overlay_Achievement> achievements{};
    size_t last_loaded_ach_icon{};
    
    bool show_overlay = false;
    bool show_user_info = false;
    bool show_achievements = false;
    bool show_settings = false;

    // warn when using local save
    bool warn_local_save = false;
    // warn when app ID = 0
    bool warn_bad_appid = false;

    char username_text[256]{};
    std::atomic<bool> save_settings = false;

    int current_language = 0;

    std::string warning_message{};

    // Callback infos
    std::queue<Friend> has_friend_action{};
    std::vector<Notification> notifications{};
    // used when the button "Invite all" is clicked
    std::atomic<bool> invite_all_friends_clicked = false;

    bool overlay_state_changed = false;

    std::atomic<bool> i_have_lobby = false;

    // some stuff has to be initialized once the renderer hook is ready
    std::atomic<bool> late_init_imgui = false;

    // changed each time a notification is posted or overlay is shown/hidden
    std::atomic_uint32_t renderer_frame_processing_requests = 0;
    // changed only when overlay is shown/hidden, true means overlay is shown
    std::atomic_uint32_t obscure_cursor_requests = 0;
    
    std::future<InGameOverlay::RendererHook_t *> future_renderer{};
    InGameOverlay::RendererHook_t *_renderer{};

    common_helpers::KillableWorker renderer_detector_delay_thread{};
    common_helpers::KillableWorker renderer_hook_init_thread{};
    int renderer_hook_timeout_ctr{};

    // font stuff
    ImFontAtlas fonts_atlas{};
    ImFont *font_default{};
    ImFont *font_notif{};
    ImFontConfig font_cfg{};
    ImFontGlyphRangesBuilder font_builder{};
    ImVector<ImWchar> ranges{};

    std::recursive_mutex overlay_mutex{};
    std::atomic<bool> setup_overlay_called = false;

    std::map<std::string, std::vector<char>> wav_files{
        { "overlay_achievement_notification.wav", std::vector<char>{} },
        { "overlay_friend_notification.wav", std::vector<char>{} },
    };

    Steam_Overlay(Steam_Overlay const&) = delete;
    Steam_Overlay(Steam_Overlay&&) = delete;
    Steam_Overlay& operator=(Steam_Overlay const&) = delete;
    Steam_Overlay& operator=(Steam_Overlay&&) = delete;

    bool submit_notification(
        notification_type type,
        const std::string &msg,
        std::pair<const Friend, friend_window_state> *frd = nullptr,
        Overlay_Achievement *ach = nullptr
    );

    void notify_sound_user_invite(friend_window_state& friend_state);
    void notify_sound_user_achievement();
    void notify_sound_auto_accept_friend_invite();

    // Right click on friend
    void build_friend_context_menu(Friend const& frd, friend_window_state &state);
    // Double click on friend
    void build_friend_window(Friend const& frd, friend_window_state &state);
    std::chrono::milliseconds get_notification_duration(notification_type type);
    // Notifications like achievements, chat and invitations
    void set_next_notification_pos(std::pair<float, float> scrn_size, std::chrono::milliseconds elapsed, std::chrono::milliseconds duration, const Notification &noti, struct NotificationsCoords &coords);
    // factor controlling the amount of sliding during the animation, 0 means disabled
    float animate_factor(std::chrono::milliseconds elapsed, std::chrono::milliseconds duration);
    void add_ach_progressbar(const Overlay_Achievement &ach);
    ImVec4 get_notification_bg_rgba_safe();
    void build_notifications(float width, float height);
    
    void request_renderer_detector();
    void set_renderer_hook_timeout();
    void cleanup_renderer_hook();
    bool renderer_hook_proc();
    
    // note: make sure to load all relevant strings before creating the font(s), otherwise some glyphs ranges will be missing
    void create_fonts();
    void load_audio();
    void load_achievements_data();

    void overlay_state_hook(bool ready);
    void allow_renderer_frame_processing(bool state, bool cleaning_up_overlay = false);
    void obscure_game_input(bool state);

    void add_auto_accept_invite_notification();
    void add_invite_notification(std::pair<const Friend, friend_window_state> &wnd_state);
    void post_achievement_notification(Overlay_Achievement &ach, bool for_progress);
    void add_chat_message_notification(std::string const& message);
    void show_test_achievement();

    bool open_overlay_hook(bool toggle);

    bool try_load_ach_icon(Overlay_Achievement &ach, bool achieved, bool upload_new_icon_to_gpu);

    void overlay_render_proc();
    void load_next_ach_icon();
    uint32 apply_global_style_color();
    void render_main_window();


    void steam_run_callback_update_my_lobby();
    bool is_friend_joinable(std::pair<const Friend, friend_window_state> &f);
    // invite a single friend
    void invite_friend(uint64 friend_id, class Steam_Friends* steamFriends, class Steam_Matchmaking* steamMatchmaking);
    void steam_run_callback_friends_actions();
    void steam_run_callback();

    void networking_msg_received(Common_Message* msg);


    static void overlay_run_callback(void* object);
    static void overlay_networking_callback(void* object, Common_Message* msg);
    
public:
    Steam_Overlay(Settings* settings, Local_Storage *local_storage, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking *network);

    ~Steam_Overlay();

    bool Ready() const;

    bool NeedPresent() const;

    void SetNotificationPosition(ENotificationPosition eNotificationPosition);

    void SetNotificationInset(int nHorizontalInset, int nVerticalInset);
    void SetupOverlay();
    void UnSetupOverlay();

    void OpenOverlayInvite(CSteamID lobbyId);
    void OpenOverlay(const char* pchDialog);
    void OpenOverlayWebpage(const char* pchURL);

    bool ShowOverlay() const;
    void ShowOverlay(bool state);

    void SetLobbyInvite(Friend friendId, uint64 lobbyId);
    void SetRichInvite(Friend friendId, const char* connect_str);

    void FriendConnect(Friend _friend);
    void FriendDisconnect(Friend _friend);

    void AddAchievementNotification(const std::string &ach_name, nlohmann::json const& ach, bool for_progress);
};

#else // EMU_OVERLAY

class Steam_Overlay
{
public:
    Steam_Overlay(Settings* settings, Local_Storage *local_storage, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking* network) {}
    ~Steam_Overlay() {}

    bool Ready() const { return false; }

    bool NeedPresent() const { return false; }

    void SetNotificationPosition(ENotificationPosition eNotificationPosition) {}

    void SetNotificationInset(int nHorizontalInset, int nVerticalInset) {}
    void SetupOverlay() {}
    void UnSetupOverlay() {}

    void OpenOverlayInvite(CSteamID lobbyId) {}
    void OpenOverlay(const char* pchDialog) {}
    void OpenOverlayWebpage(const char* pchURL) {}

    bool ShowOverlay() const { return false; }
    void ShowOverlay(bool state) {}

    void SetLobbyInvite(Friend friendId, uint64 lobbyId) {}
    void SetRichInvite(Friend friendId, const char* connect_str) {}

    void FriendConnect(Friend _friend) {}
    void FriendDisconnect(Friend _friend) {}

    void AddAchievementNotification(const std::string &ach_name, nlohmann::json const& ach, bool for_progress) {}
};

#endif // EMU_OVERLAY

#endif //__INCLUDED_STEAM_OVERLAY_H__
