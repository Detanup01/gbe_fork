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

enum notification_type
{
    notification_type_message = 0,
    notification_type_invite,
    notification_type_achievement,
    notification_type_auto_accept_invite,
};

struct Notification
{
    static constexpr float width_percent = 0.25f; // percentage from total width
    static constexpr float height = 6.5f;
    static constexpr std::chrono::milliseconds fade_in   = std::chrono::milliseconds(2000);
    static constexpr std::chrono::milliseconds fade_out  = std::chrono::milliseconds(2000);
    static constexpr std::chrono::milliseconds show_time = std::chrono::milliseconds(6000) + fade_in + fade_out;
    static constexpr std::chrono::milliseconds fade_out_start = show_time - fade_out;

    int id;
    uint8 type;
    std::chrono::seconds start_time;
    std::string message;
    std::pair<const Friend, friend_window_state>* frd;
    std::weak_ptr<uint64_t> icon;
};

struct Overlay_Achievement
{
    std::string name;
    std::string title;
    std::string description;
    std::string icon_name;
    std::string icon_gray_name;
    bool hidden;
    bool achieved;
    uint32 unlock_time;
    std::weak_ptr<uint64_t> icon;
    std::weak_ptr<uint64_t> icon_gray;

    // avoids spam loading on failure
    constexpr const static int ICON_LOAD_MAX_TRIALS = 3;
    uint8_t icon_load_trials = ICON_LOAD_MAX_TRIALS;
    uint8_t icon_gray_load_trials = ICON_LOAD_MAX_TRIALS;
};

struct NotificationsIndexes
{
    int top_left = 0, top_center = 0, top_right = 0;
    int bot_left = 0, bot_center = 0, bot_right = 0;
};

class Steam_Overlay
{
    constexpr static const char ACH_FALLBACK_DIR[] = "achievement_images";

    Settings* settings;
    SteamCallResults* callback_results;
    SteamCallBacks* callbacks;
    RunEveryRunCB* run_every_runcb;
    Networking* network;

    // friend id, show client window (to chat and accept invite maybe)
    std::map<Friend, friend_window_state, Friend_Less> friends;

    // avoids spam loading on failure
    constexpr const static int LOAD_ACHIEVEMENTS_MAX_TRIALS = 3;
    std::atomic<int32_t> load_achievements_trials = LOAD_ACHIEVEMENTS_MAX_TRIALS;
    bool is_ready = false;
    bool show_overlay;
    ENotificationPosition notif_position;
    int h_inset, v_inset;
    std::string show_url;
    std::vector<Overlay_Achievement> achievements;
    // index of the next achievement whose icons will be loaded
    // used by the callback
    int next_ach_to_load = 0;
    bool show_achievements, show_settings;

    // disable input when force_*.txt file is used
    bool disable_user_input;
    // warn when force_*.txt file is used
    bool warn_forced_setting;
    // warn when using local save
    bool warn_local_save;
    // warn when app ID = 0
    bool warn_bad_appid;

    char username_text[256];
    std::atomic<bool> save_settings;

    int current_language = 0;

    std::string warning_message{};

    // Callback infos
    std::queue<Friend> has_friend_action{};
    std::vector<Notification> notifications{};
    // used when the button "Invite all" is clicked
    std::atomic<bool> invite_all_friends_clicked = false;

    bool overlay_state_changed;

    std::atomic<bool> i_have_lobby;

    // changed each time a notification is posted or overlay is shown/hidden
    std::atomic_uint32_t renderer_frame_processing_requests = 0;
    // changed only when overlay is shown/hidden, true means overlay is shown
    std::atomic_uint32_t obscure_cursor_requests = 0;
    
    constexpr static const int renderer_detector_polling_ms = 100;
    std::future<InGameOverlay::RendererHook_t *> future_renderer{};
    InGameOverlay::RendererHook_t *_renderer{};

    Steam_Overlay(Steam_Overlay const&) = delete;
    Steam_Overlay(Steam_Overlay&&) = delete;
    Steam_Overlay& operator=(Steam_Overlay const&) = delete;
    Steam_Overlay& operator=(Steam_Overlay&&) = delete;

    static void steam_overlay_run_every_runcb(void* object);
    static void steam_overlay_callback(void* object, Common_Message* msg);

    void Callback(Common_Message* msg);
    void RunCallbacks();

    bool is_friend_joinable(std::pair<const Friend, friend_window_state> &f);
    bool got_lobby();

    bool submit_notification(notification_type type, const std::string &msg, std::pair<const Friend, friend_window_state> *frd = nullptr, const std::weak_ptr<uint64_t> &icon = {});

    void notify_sound_user_invite(friend_window_state& friend_state);
    void notify_sound_user_achievement();
    void notify_sound_auto_accept_friend_invite();

    // Right click on friend
    void build_friend_context_menu(Friend const& frd, friend_window_state &state);
    // Double click on friend
    void build_friend_window(Friend const& frd, friend_window_state &state);
    // Notifications like achievements, chat and invitations
    void set_next_notification_pos(float width, float height, float font_size, notification_type type, struct NotificationsIndexes &idx);
    void build_notifications(int width, int height);
    // invite a single friend
    void invite_friend(uint64 friend_id, class Steam_Friends* steamFriends, class Steam_Matchmaking* steamMatchmaking);

    void request_renderer_detector();
    void renderer_detector_delay_thread();
    void renderer_hook_init_thread();
    
    void create_fonts();
    void load_audio();

    void overlay_state_hook(bool ready);
    void allow_renderer_frame_processing(bool state, bool cleaning_up_overlay = false);
    void obscure_cursor_input(bool state);

    void overlay_proc();

    void add_auto_accept_invite_notification();

    void add_invite_notification(std::pair<const Friend, friend_window_state> &wnd_state);
    
    void add_chat_message_notification(std::string const& message);

    bool open_overlay_hook(bool toggle);

    bool try_load_ach_icon(Overlay_Achievement &ach);
    bool try_load_ach_gray_icon(Overlay_Achievement &ach);

public:
    Steam_Overlay(Settings* settings, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking *network);

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

    void AddAchievementNotification(nlohmann::json const& ach);
};

#else

class Steam_Overlay
{
public:
    Steam_Overlay(Settings* settings, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking* network) {}
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

    void AddAchievementNotification(nlohmann::json const& ach) {}
};

#endif

#endif//__INCLUDED_STEAM_OVERLAY_H__
