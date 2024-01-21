#include "overlay/steam_overlay.h"
#include "overlay/steam_overlay_translations.h"

#ifdef EMU_OVERLAY

#include <thread>
#include <string>
#include <sstream>
#include <cctype>
#include <imgui.h>

#include "dll/dll.h"
#include "dll/settings_parser.h"

#include "overlay/Renderer_Detector.h"

static constexpr int max_window_id = 10000;
static constexpr int base_notif_window_id  = 0 * max_window_id;
static constexpr int base_friend_window_id = 1 * max_window_id;
static constexpr int base_friend_item_id   = 2 * max_window_id;

static constexpr char *valid_languages[] = {
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
    "croatian"
};

#define URL_WINDOW_NAME "URL Window"

int find_free_id(std::vector<int> & ids, int base)
{
    std::sort(ids.begin(), ids.end());

    int id = base;
    for (auto i : ids)
    {
        if (id < i)
            break;
        id = i + 1;
    }

    return id > (base+max_window_id) ? 0 : id;
}

int find_free_friend_id(std::map<Friend, friend_window_state, Friend_Less> const& friend_windows)
{
    std::vector<int> ids;
    ids.reserve(friend_windows.size());

    std::for_each(friend_windows.begin(), friend_windows.end(), [&ids](std::pair<Friend const, friend_window_state> const& i)
    {
        ids.emplace_back(i.second.id);
    });
    
    return find_free_id(ids, base_friend_window_id);
}

int find_free_notification_id(std::vector<Notification> const& notifications)
{
    std::vector<int> ids;
    ids.reserve(notifications.size());

    std::for_each(notifications.begin(), notifications.end(), [&ids](Notification const& i)
    {
        ids.emplace_back(i.id);
    });
    

    return find_free_id(ids, base_friend_window_id);
}

#ifdef __WINDOWS__
#include "windows/Windows_Hook.h"
#endif

#include "overlay/notification.h"
char *notif_achievement_wav_custom;
char *notif_invite_wav_custom;
bool notif_achievement_wav_custom_inuse = false;
bool notif_invite_wav_custom_inuse = false;

void Steam_Overlay::steam_overlay_run_every_runcb(void* object)
{
    Steam_Overlay* _this = reinterpret_cast<Steam_Overlay*>(object);
    _this->RunCallbacks();
}

void Steam_Overlay::steam_overlay_callback(void* object, Common_Message* msg)
{
    Steam_Overlay* _this = reinterpret_cast<Steam_Overlay*>(object);
    _this->Callback(msg);
}

Steam_Overlay::Steam_Overlay(Settings* settings, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Networking* network) :
    settings(settings),
    callback_results(callback_results),
    callbacks(callbacks),
    run_every_runcb(run_every_runcb),
    network(network),
    setup_overlay_called(false),
    show_overlay(false),
    is_ready(false),
    notif_position(ENotificationPosition::k_EPositionBottomLeft),
    h_inset(0),
    v_inset(0),
    overlay_state_changed(false),
    i_have_lobby(false),
    show_achievements(false),
    show_settings(false),
    _renderer(nullptr),
    fonts_atlas(nullptr)
{
    strncpy(username_text, settings->get_local_name(), sizeof(username_text));

    // we need these copies to show the warning only once, then disable the flag
    // avoid manipulating settings->xxx
    this->warn_forced_setting =
        !settings->disable_overlay_warning_any && !settings->disable_overlay_warning_forced_setting && settings->overlay_warn_forced_setting;
    this->warn_local_save =
        !settings->disable_overlay_warning_any && !settings->disable_overlay_warning_local_save && settings->overlay_warn_local_save;
    this->warn_bad_appid =
        !settings->disable_overlay_warning_any && !settings->disable_overlay_warning_bad_appid && settings->get_local_game_id().AppID() == 0;

    this->disable_user_input = this->warn_forced_setting;

    current_language = 0;
    const char *language = settings->get_language();

    int i = 0;
    for (auto l : valid_languages) {
        if (strcmp(l, language) == 0) {
            current_language = i;
            break;
        }

        ++i;
    }

    run_every_runcb->add(&Steam_Overlay::steam_overlay_run_every_runcb, this);
    this->network->setCallback(CALLBACK_ID_STEAM_MESSAGES, settings->get_local_steam_id(), &Steam_Overlay::steam_overlay_callback, this);
}

Steam_Overlay::~Steam_Overlay()
{
    run_every_runcb->remove(&Steam_Overlay::steam_overlay_run_every_runcb, this);
}

bool Steam_Overlay::Ready() const
{
    return is_ready;
}

bool Steam_Overlay::NeedPresent() const
{
    return true;
}

void Steam_Overlay::SetNotificationPosition(ENotificationPosition eNotificationPosition)
{
    notif_position = eNotificationPosition;
}

void Steam_Overlay::SetNotificationInset(int nHorizontalInset, int nVerticalInset)
{
    h_inset = nHorizontalInset;
    v_inset = nVerticalInset;
}

void Steam_Overlay::SetupOverlay()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!setup_overlay_called)
    {
        setup_overlay_called = true;
        future_renderer = ingame_overlay::DetectRenderer();
    }
}


void Steam_Overlay::UnSetupOverlay()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    ingame_overlay::StopRendererDetection();
    if (!Ready() && future_renderer.valid()) {
        if (future_renderer.wait_for(std::chrono::milliseconds{500}) ==  std::future_status::ready) {
            future_renderer.get();
            ingame_overlay::FreeDetector();
        }
    }
}

void Steam_Overlay::HookReady(bool ready)
{
    PRINT_DEBUG("%s %u\n", __FUNCTION__, ready);
    {
        // TODO: Uncomment this and draw our own cursor (cosmetics)
        ImGuiIO &io = ImGui::GetIO();
        //io.WantSetMousePos = false;
        //io.MouseDrawCursor = false;
        //io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        io.IniFilename = NULL;

        is_ready = ready;
    }
}

void Steam_Overlay::OpenOverlayInvite(CSteamID lobbyId)
{
    ShowOverlay(true);
}

void Steam_Overlay::OpenOverlay(const char* pchDialog)
{
    // TODO: Show pages depending on pchDialog
    if ((strncmp(pchDialog, "Friends", sizeof("Friends") - 1) == 0) && (settings->auto_accept_invites.size() > 0)) {
        PRINT_DEBUG("Not opening overlay's friends list because some friends are defined in the auto accept list\n");
        AddAutoAcceptInviteNotification();
    } else {
        ShowOverlay(true);
    }
}

void Steam_Overlay::OpenOverlayWebpage(const char* pchURL)
{
    show_url = pchURL;
    ShowOverlay(true);
}

bool Steam_Overlay::ShowOverlay() const
{
    return show_overlay;
}

bool Steam_Overlay::OpenOverlayHook(bool toggle)
{
    if (toggle) {
        ShowOverlay(!show_overlay);
    }

    return show_overlay;
}

void Steam_Overlay::ShowOverlay(bool state)
{
    if (!Ready() || show_overlay == state)
        return;

    ImGuiIO &io = ImGui::GetIO();

    if(state)
    {
        io.MouseDrawCursor = true;
    }
    else
    {
        io.MouseDrawCursor = false;
    }

#ifdef __WINDOWS__
    static RECT old_clip;

    if (state)
    {
        HWND game_hwnd = Windows_Hook::Inst()->GetGameHwnd();
        RECT cliRect, wndRect, clipRect;

        GetClipCursor(&old_clip);
        // The window rectangle has borders and menus counted in the size
        GetWindowRect(game_hwnd, &wndRect);
        // The client rectangle is the window without borders
        GetClientRect(game_hwnd, &cliRect);

        clipRect = wndRect; // Init clip rectangle

        // Get Window width with borders
        wndRect.right -= wndRect.left;
        // Get Window height with borders & menus
        wndRect.bottom -= wndRect.top;
        // Compute the border width
        int borderWidth = (wndRect.right - cliRect.right) / 2;
        // Client top clip is the menu bar width minus bottom border
        clipRect.top += wndRect.bottom - cliRect.bottom - borderWidth;
        // Client left clip is the left border minus border width
        clipRect.left += borderWidth;
        // Here goes the same for right and bottom
        clipRect.right -= borderWidth;
        clipRect.bottom -= borderWidth;

        ClipCursor(&clipRect);
    }
    else
    {
        ClipCursor(&old_clip);
    }

#else

#endif

    show_overlay = state;
    overlay_state_changed = true;
}

void Steam_Overlay::NotifyUser(friend_window_state& friend_state)
{
    if (settings->disable_overlay_friend_notification) return;
    if (!(friend_state.window_state & window_state_show) || !show_overlay)
    {
        friend_state.window_state |= window_state_need_attention;
#ifdef __WINDOWS__
        if (notif_invite_wav_custom_inuse) {
            PlaySoundA((LPCSTR)notif_invite_wav_custom, NULL, SND_ASYNC | SND_MEMORY);
        } else {
            PlaySoundA((LPCSTR)notif_invite_wav, NULL, SND_ASYNC | SND_MEMORY);
        }
#endif
    }
}

void Steam_Overlay::NotifyUserAchievement()
{
    if (settings->disable_overlay_achievement_notification) return;
    if (!show_overlay)
    {
#ifdef __WINDOWS__
        if (notif_achievement_wav_custom_inuse) {
            PlaySoundA((LPCSTR)notif_achievement_wav_custom, NULL, SND_ASYNC | SND_MEMORY);
        }
#endif
    }
}

void Steam_Overlay::NotifySoundAutoAcceptFriendInvite()
{
#ifdef __WINDOWS__
    if (notif_achievement_wav_custom_inuse) {
        PlaySoundA((LPCSTR)notif_achievement_wav_custom, NULL, SND_ASYNC | SND_MEMORY);
    } else {
        PlaySoundA((LPCSTR)notif_invite_wav, NULL, SND_ASYNC | SND_MEMORY);
    }
#endif
}

void Steam_Overlay::SetLobbyInvite(Friend friendId, uint64 lobbyId)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready())
        return;

    auto i = friends.find(friendId);
    if (i != friends.end())
    {
        auto& frd = i->second;
        frd.lobbyId = lobbyId;
        frd.window_state |= window_state_lobby_invite;
        // Make sure don't have rich presence invite and a lobby invite (it should not happen but who knows)
        frd.window_state &= ~window_state_rich_invite;
        AddInviteNotification(*i);
        NotifyUser(i->second);
    }
}

void Steam_Overlay::SetRichInvite(Friend friendId, const char* connect_str)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (!Ready())
        return;

    auto i = friends.find(friendId);
    if (i != friends.end())
    {
        auto& frd = i->second;
        strncpy(frd.connect, connect_str, k_cchMaxRichPresenceValueLength - 1);
        frd.window_state |= window_state_rich_invite;
        // Make sure don't have rich presence invite and a lobby invite (it should not happen but who knows)
        frd.window_state &= ~window_state_lobby_invite;
        AddInviteNotification(*i);
        NotifyUser(i->second);
    }
}

void Steam_Overlay::FriendConnect(Friend _friend)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    int id = find_free_friend_id(friends);
    if (id != 0)
    {
        auto& item = friends[_friend];
        item.window_title = std::move(_friend.name() + translationPlaying[current_language] + std::to_string(_friend.appid()));
        item.window_state = window_state_none;
        item.id = id;
        memset(item.chat_input, 0, max_chat_len);
        item.joinable = false;
    }
    else
        PRINT_DEBUG("No more free id to create a friend window\n");
}

void Steam_Overlay::FriendDisconnect(Friend _friend)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    auto it = friends.find(_friend);
    if (it != friends.end())
        friends.erase(it);
}

void Steam_Overlay::AddMessageNotification(std::string const& message)
{
    std::lock_guard<std::recursive_mutex> lock(notifications_mutex);
    if (settings->disable_overlay_friend_notification) return;
    int id = find_free_notification_id(notifications);
    if (id != 0)
    {
        Notification notif;
        notif.id = id;
        notif.type = notification_type_message;
        notif.message = message;
        notif.start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        notifications.emplace_back(notif);
        have_notifications = true;
    }
    else
        PRINT_DEBUG("No more free id to create a notification window\n");
}

void Steam_Overlay::AddAchievementNotification(nlohmann::json const& ach)
{
    std::lock_guard<std::recursive_mutex> lock(notifications_mutex);
    if (!settings->disable_overlay_achievement_notification) {
        int id = find_free_notification_id(notifications);
        if (id != 0)
        {
            Notification notif;
            notif.id = id;
            notif.type = notification_type_achievement;

            // Load achievement image
            std::string file_path = Local_Storage::get_game_settings_path() + ach["icon"].get<std::string>();
            unsigned long long file_size = file_size_(file_path);
            if (!file_size) {
                file_path = Local_Storage::get_game_settings_path() + "achievement_images/" + ach["icon"].get<std::string>();
                file_size = file_size_(file_path);
            }
            if (file_size) {
                std::string img = Local_Storage::load_image_resized(file_path, "", settings->overlay_appearance.icon_size);
                if (img.length() > 0) {
                    if (_renderer) notif.icon = _renderer->CreateImageResource((void*)img.c_str(), settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size);
                }
            }

            notif.message = ach["displayName"].get<std::string>() + "\n" + ach["description"].get<std::string>();
            notif.start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
            notifications.emplace_back(notif);
            NotifyUserAchievement();
            have_notifications = true;
        }
        else
            PRINT_DEBUG("No more free id to create a notification window\n");
    }

    std::string ach_name = ach.value("name", "");
    for (auto &a : achievements) {
        if (a.name == ach_name) {
            bool achieved = false;
            uint32 unlock_time = 0;
            get_steam_client()->steam_user_stats->GetAchievementAndUnlockTime(a.name.c_str(), &achieved, &unlock_time);
            a.achieved = achieved;
            a.unlock_time = unlock_time;
        }
    }
}

void Steam_Overlay::AddInviteNotification(std::pair<const Friend, friend_window_state>& wnd_state)
{
    std::lock_guard<std::recursive_mutex> lock(notifications_mutex);
    if (settings->disable_overlay_friend_notification) return;
    int id = find_free_notification_id(notifications);
    if (id != 0)
    {
        Notification notif;
        notif.id = id;
        notif.type = notification_type_invite;
        notif.frd = &wnd_state;

        {
            char tmp[TRANSLATION_BUFFER_SIZE]{};
            auto &first_friend = wnd_state.first;
            auto &name = first_friend.name();
            snprintf(tmp, sizeof(tmp), translationInvitedYouToJoinTheGame[current_language], name.c_str(), (uint64)first_friend.id());
            notif.message = tmp;
        }

        notif.start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        notifications.emplace_back(notif);
        have_notifications = true;
    }
    else
        PRINT_DEBUG("No more free id to create a notification window\n");
}

void Steam_Overlay::AddAutoAcceptInviteNotification()
{
    std::lock_guard<std::recursive_mutex> lock(notifications_mutex);
    int id = find_free_notification_id(notifications);
    if (id != 0)
    {
        Notification notif{};
        notif.id = id;
        notif.type = notification_type_auto_accept_invite;
        
        {
            char tmp[TRANSLATION_BUFFER_SIZE]{};
            snprintf(tmp, sizeof(tmp), "%s", translationAutoAcceptFriendInvite[current_language]);
            notif.message = tmp;
        }

        notif.start_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
        notifications.emplace_back(notif);
        NotifySoundAutoAcceptFriendInvite();
        have_notifications = true;
    } else {
        PRINT_DEBUG("No free id to create an auto-accept notification window\n");
    }
}

bool Steam_Overlay::FriendJoinable(std::pair<const Friend, friend_window_state> &f)
{
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;

    if( std::string(steamFriends->GetFriendRichPresence(f.first.id(), "connect")).length() > 0 )
        return true;

    FriendGameInfo_t friend_game_info = {};
    steamFriends->GetFriendGamePlayed(f.first.id(), &friend_game_info);
    if (friend_game_info.m_steamIDLobby.IsValid() && (f.second.window_state & window_state_lobby_invite))
        return true;

    return false;
}

bool Steam_Overlay::IHaveLobby()
{
    Steam_Friends* steamFriends = get_steam_client()->steam_friends;
    if (std::string(steamFriends->GetFriendRichPresence(settings->get_local_steam_id(), "connect")).length() > 0)
        return true;

    if (settings->get_lobby().IsValid())
        return true;

    return false;
}

void Steam_Overlay::BuildContextMenu(Friend const& frd, friend_window_state& state)
{
    if (ImGui::BeginPopupContextItem("Friends_ContextMenu", 1))
    {
        bool close_popup = false;

        if (ImGui::Button(translationChat[current_language]))
        {
            state.window_state |= window_state_show;
            close_popup = true;
        }
        // If we have the same appid, activate the invite/join buttons
        if (settings->get_local_game_id().AppID() == frd.appid())
        {
            std::string translationInvite_tmp;
            std::string translationJoin_tmp;
            translationInvite_tmp.append(translationInvite[current_language]);
            translationInvite_tmp.append("##PopupInvite");
            translationJoin_tmp.append(translationJoin[current_language]);
            translationJoin_tmp.append("##PopupInvite");

            if (i_have_lobby && ImGui::Button(translationInvite_tmp.c_str()))
            {
                state.window_state |= window_state_invite;
                has_friend_action.push(frd);
                close_popup = true;
            }
            if (state.joinable && ImGui::Button(translationJoin_tmp.c_str()))
            {
                state.window_state |= window_state_join;
                has_friend_action.push(frd);
                close_popup = true;
            }
        }
        if( close_popup)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Steam_Overlay::BuildFriendWindow(Friend const& frd, friend_window_state& state)
{
    if (!(state.window_state & window_state_show))
        return;

    bool show = true;
    bool send_chat_msg = false;

    float width = ImGui::CalcTextSize("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA").x;
    
    if (state.window_state & window_state_need_attention && ImGui::IsWindowFocused())
    {
        state.window_state &= ~window_state_need_attention;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2{ width, ImGui::GetFontSize()*8 + ImGui::GetFrameHeightWithSpacing()*4 },
        ImVec2{ std::numeric_limits<float>::max() , std::numeric_limits<float>::max() });

    // Window id is after the ###, the window title is the friend name
    std::string friend_window_id = std::move("###" + std::to_string(state.id));
    if (ImGui::Begin((state.window_title + friend_window_id).c_str(), &show))
    {
        if (state.window_state & window_state_need_attention && ImGui::IsWindowFocused())
        {
            state.window_state &= ~window_state_need_attention;
        }

        // Fill this with the chat box and maybe the invitation
        if (state.window_state & (window_state_lobby_invite | window_state_rich_invite))
        {
            ImGui::LabelText("##label", translationInvitedYouToJoinTheGame[current_language], frd.name().c_str(), frd.appid());
            ImGui::SameLine();
            if (ImGui::Button(translationAccept[current_language]))
            {
                state.window_state |= window_state_join;
                this->has_friend_action.push(frd);
            }
            ImGui::SameLine();
            if (ImGui::Button(translationRefuse[current_language]))
            {
                state.window_state &= ~(window_state_lobby_invite | window_state_rich_invite);
            }
        }

        ImGui::InputTextMultiline("##chat_history", &state.chat_history[0], state.chat_history.length(), { -1.0f, -2.0f * ImGui::GetFontSize() }, ImGuiInputTextFlags_ReadOnly);
        // TODO: Fix the layout of the chat line + send button.
        // It should be like this: chat input should fill the window size minus send button size (button size is fixed)
        // |------------------------------|
        // | /--------------------------\ |
        // | |                          | |
        // | |       chat history       | |
        // | |                          | |
        // | \--------------------------/ |
        // | [____chat line______] [send] |
        // |------------------------------|
        //
        // And it is like this
        // |------------------------------|
        // | /--------------------------\ |
        // | |                          | |
        // | |       chat history       | |
        // | |                          | |
        // | \--------------------------/ |
        // | [__chat line__] [send]       |
        // |------------------------------|
        float wnd_width = ImGui::GetWindowContentRegionWidth();
        ImGuiStyle &style = ImGui::GetStyle();
        wnd_width -= ImGui::CalcTextSize(translationSend[current_language]).x + style.FramePadding.x * 2 + style.ItemSpacing.x + 1;

        ImGui::PushItemWidth(wnd_width);
        if (ImGui::InputText("##chat_line", state.chat_input, max_chat_len, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            send_chat_msg = true;
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button(translationSend[current_language]))
        {
            send_chat_msg = true;
        }

        if (send_chat_msg)
        {
            if (!(state.window_state & window_state_send_message))
            {
                has_friend_action.push(frd);
                state.window_state |= window_state_send_message;
            }
        }
    }
    // User closed the friend window
    if (!show)
        state.window_state &= ~window_state_show;

    ImGui::End();
}

ImFont *font_default;
ImFont *font_notif;

void Steam_Overlay::BuildNotifications(int width, int height)
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    int i = 0;

    int font_size = ImGui::GetFontSize();

    std::queue<Friend> friend_actions_temp;

    {
        std::lock_guard<std::recursive_mutex> lock(notifications_mutex);

        for (auto it = notifications.begin(); it != notifications.end(); ++it, ++i)
        {
            auto elapsed_notif = now - it->start_time;

            if ( elapsed_notif < Notification::fade_in)
            {
                float alpha = settings->overlay_appearance.notification_a * (elapsed_notif.count() / static_cast<float>(Notification::fade_in.count()));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, alpha));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(settings->overlay_appearance.notification_r, settings->overlay_appearance.notification_g, settings->overlay_appearance.notification_b, alpha));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, alpha*2));
            }
            else if ( elapsed_notif > Notification::fade_out_start)
            {
                float alpha = settings->overlay_appearance.notification_a * ((Notification::show_time - elapsed_notif).count() / static_cast<float>(Notification::fade_out.count()));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, alpha));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(settings->overlay_appearance.notification_r, settings->overlay_appearance.notification_g, settings->overlay_appearance.notification_b, alpha));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, alpha*2));
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, settings->overlay_appearance.notification_a));
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(settings->overlay_appearance.notification_r, settings->overlay_appearance.notification_g, settings->overlay_appearance.notification_b, settings->overlay_appearance.notification_a));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, settings->overlay_appearance.notification_a*2));
            }
            
            ImGui::SetNextWindowPos(ImVec2((float)width - width * Notification::width, Notification::height * font_size * i ));
            ImGui::SetNextWindowSize(ImVec2( width * Notification::width, Notification::height * font_size ));
            ImGui::Begin(std::to_string(it->id).c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDecoration);

            switch (it->type)
            {
                case notification_type_achievement:
                    {
                        if (!it->icon.expired()) {
                            ImGui::BeginTable("imgui_table", 2);
                            ImGui::TableSetupColumn("imgui_table_image", ImGuiTableColumnFlags_WidthFixed, settings->overlay_appearance.icon_size);
                            ImGui::TableSetupColumn("imgui_table_text");
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, settings->overlay_appearance.icon_size);

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Image((ImU64)*it->icon.lock().get(), ImVec2(settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size));

                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextWrapped("%s", it->message.c_str());

                            ImGui::EndTable();
                        } else {
                            ImGui::TextWrapped("%s", it->message.c_str());
                        }
                    }
                    break;
                case notification_type_invite:
                    {
                        ImGui::TextWrapped("%s", it->message.c_str());
                        if (ImGui::Button(translationJoin[current_language]))
                        {
                            it->frd->second.window_state |= window_state_join;
                            friend_actions_temp.push(it->frd->first);
                            it->start_time = std::chrono::seconds(0);
                        }
                    }
                    break;
                case notification_type_message:
                    ImGui::TextWrapped("%s", it->message.c_str()); break;
                case notification_type_auto_accept_invite:
                    ImGui::TextWrapped("%s", it->message.c_str()); break;
            }

            ImGui::End();

            ImGui::PopStyleColor(3);
        }
        notifications.erase(std::remove_if(notifications.begin(), notifications.end(), [&now](Notification &item) {
            return (now - item.start_time) > Notification::show_time;
        }), notifications.end());

        have_notifications = !notifications.empty();
    }

    if (!friend_actions_temp.empty()) {
        std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
        while (!friend_actions_temp.empty()) {
            has_friend_action.push(friend_actions_temp.front());
            friend_actions_temp.pop();
        }
    }
}

void Steam_Overlay::CreateFonts()
{
    if (fonts_atlas) return;

    ImFontAtlas *Fonts = new ImFontAtlas();

    ImFontConfig fontcfg;

    float font_size = settings->overlay_appearance.font_size;
    fontcfg.OversampleH = fontcfg.OversampleV = 1;
    fontcfg.PixelSnapH = true;
    fontcfg.SizePixels = font_size;

    ImFontGlyphRangesBuilder font_builder;
    for (auto & x : achievements) {
        font_builder.AddText(x.title.c_str());
        font_builder.AddText(x.description.c_str());
    }
    for (int i = 0; i < TRANSLATION_NUMBER_OF_LANGUAGES; i++) {
        font_builder.AddText(translationChat[i]);
        font_builder.AddText(translationInvite[i]);
        font_builder.AddText(translationJoin[i]);
        font_builder.AddText(translationInvitedYouToJoinTheGame[i]);
        font_builder.AddText(translationAccept[i]);
        font_builder.AddText(translationRefuse[i]);
        font_builder.AddText(translationSend[i]);
        font_builder.AddText(translationSteamOverlay[i]);
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
        font_builder.AddText(translationWarningWarningWarning[i]);
        font_builder.AddText(translationWarningDescription1[i]);
        font_builder.AddText(translationWarningDescription2[i]);
        font_builder.AddText(translationWarningDescription3[i]);
        font_builder.AddText(translationWarningDescription4[i]);
        font_builder.AddText(translationSteamOverlayURL[i]);
        font_builder.AddText(translationClose[i]);
        font_builder.AddText(translationPlaying[i]);
    }

    font_builder.AddRanges(Fonts->GetGlyphRangesDefault());

    ImVector<ImWchar> ranges;
    font_builder.BuildRanges(&ranges);

    bool need_extra_fonts = false;
    for (auto &x : ranges) {
        if (x > 0xFF) {
            need_extra_fonts = true;
            break;
        }
    }

    fontcfg.GlyphRanges = ranges.Data;
    ImFont *font = NULL;

#if defined(__WINDOWS__)
    font = Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\micross.ttf", font_size, &fontcfg);
#endif

    if (!font) {
        font = Fonts->AddFontDefault(&fontcfg);
    }

    font_notif = font_default = font;

    if (need_extra_fonts) {
        PRINT_DEBUG("loading extra fonts\n");
        fontcfg.MergeMode = true;
#if defined(__WINDOWS__)
        Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", font_size, &fontcfg);
        Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", font_size, &fontcfg);
#endif
    }

    Fonts->Build();
    fonts_atlas = (void *)Fonts;

    // ImGuiStyle& style = ImGui::GetStyle();
    // style.WindowRounding = 0.0; // Disable round window
    reset_LastError();
}

void Steam_Overlay::LoadAudio()
{
    std::string file_path;
    std::string file_name;
    unsigned long long file_size;

    for (int i = 0; i < 2; i++) {
        if (i == 0) file_name = "overlay_achievement_notification.wav";
        if (i == 1) file_name = "overlay_friend_notification.wav";

        file_path = Local_Storage::get_game_settings_path() + file_name;
        file_size = file_size_(file_path);
        if (!file_size) {
            if (settings->local_save.length() > 0) {
                file_path = settings->local_save + "/settings/" + file_name;
            } else {
                file_path = Local_Storage::get_user_appdata_path() + "/settings/" + file_name;
            }
            file_size = file_size_(file_path);
        }
        if (file_size) {
            std::ifstream myfile;
            myfile.open(utf8_decode(file_path), std::ios::binary | std::ios::in);
            if (myfile.is_open()) {
                myfile.seekg (0, myfile.end);
                int length = myfile.tellg();
                myfile.seekg (0, myfile.beg);

                if (i == 0) {
                    notif_achievement_wav_custom = new char [length];
                    myfile.read (notif_achievement_wav_custom, length);
                    notif_achievement_wav_custom_inuse = true;
                }
                if (i == 1) {
                    notif_invite_wav_custom = new char [length];
                    myfile.read (notif_invite_wav_custom, length);
                    notif_invite_wav_custom_inuse = true;
                }

                myfile.close();
            }
        }
    }
}

// Try to make this function as short as possible or it might affect game's fps.
void Steam_Overlay::OverlayProc()
{
    if (!Ready())
        return;

    ImGuiIO& io = ImGui::GetIO();

    if (have_notifications) {
        ImGui::PushFont(font_notif);
        BuildNotifications(io.DisplaySize.x, io.DisplaySize.y);
        ImGui::PopFont();
    }

    if (show_overlay)
    {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
        // Set the overlay windows to the size of the game window
        ImGui::SetNextWindowPos({ 0,0 });
        ImGui::SetNextWindowSize({ static_cast<float>(io.DisplaySize.x),
                                   static_cast<float>(io.DisplaySize.y) });

        ImGui::SetNextWindowBgAlpha(0.50);

        ImGui::PushFont(font_default);

        bool show = true;

        char tmp[TRANSLATION_BUFFER_SIZE]{};
        snprintf(tmp, sizeof(tmp), translationRenderer[current_language], (_renderer == nullptr ? "Unknown" : _renderer->GetLibraryName().c_str()));
        std::string windowTitle;
        windowTitle.append(translationSteamOverlay[current_language]);
        windowTitle.append(" (");
        windowTitle.append(tmp);
        windowTitle.append(")");

        if ((settings->overlay_appearance.background_r != -1.0) && (settings->overlay_appearance.background_g != -1.0) && (settings->overlay_appearance.background_b != -1.0) && (settings->overlay_appearance.background_a != -1.0)) {
            ImVec4 colorSet = ImVec4(settings->overlay_appearance.background_r, settings->overlay_appearance.background_g, settings->overlay_appearance.background_b, settings->overlay_appearance.background_a);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, colorSet);
        }
        if ((settings->overlay_appearance.element_r != -1.0) && (settings->overlay_appearance.element_g != -1.0) && (settings->overlay_appearance.element_b != -1.0) && (settings->overlay_appearance.element_a != -1.0)) {
            ImVec4 colorSet = ImVec4(settings->overlay_appearance.element_r, settings->overlay_appearance.element_g, settings->overlay_appearance.element_b, settings->overlay_appearance.element_a);
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, colorSet);
            ImGui::PushStyleColor(ImGuiCol_Button, colorSet);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, colorSet);
            ImGui::PushStyleColor(ImGuiCol_ResizeGrip, colorSet);
        }
        if ((settings->overlay_appearance.element_hovered_r != -1.0) && (settings->overlay_appearance.element_hovered_g != -1.0) && (settings->overlay_appearance.element_hovered_b != -1.0) && (settings->overlay_appearance.element_hovered_a != -1.0)) {
            ImVec4 colorSet = ImVec4(settings->overlay_appearance.element_hovered_r, settings->overlay_appearance.element_hovered_g, settings->overlay_appearance.element_hovered_b, settings->overlay_appearance.element_hovered_a);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorSet);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colorSet);
            ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, colorSet);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colorSet);
        }
        if ((settings->overlay_appearance.element_active_r != -1.0) && (settings->overlay_appearance.element_active_g != -1.0) && (settings->overlay_appearance.element_active_b != -1.0) && (settings->overlay_appearance.element_active_a != -1.0)) {
            ImVec4 colorSet = ImVec4(settings->overlay_appearance.element_active_r, settings->overlay_appearance.element_active_g, settings->overlay_appearance.element_active_b, settings->overlay_appearance.element_active_a);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorSet);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colorSet);
            ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, colorSet);
            ImGui::PushStyleColor(ImGuiCol_Header, colorSet);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, colorSet);
        }

        if (ImGui::Begin(windowTitle.c_str(), &show, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            ImGui::LabelText("##label", translationUserPlaying[current_language],
                settings->get_local_name(),
                settings->get_local_steam_id().ConvertToUint64(),
                settings->get_local_game_id().AppID());
            ImGui::SameLine();

            ImGui::Spacing();
            if (ImGui::Button(translationShowAchievements[current_language])) {
                show_achievements = true;
            }

            ImGui::SameLine();

            if (ImGui::Button(translationSettings[current_language])) {
                show_settings = true;
            }

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::LabelText("##label", translationFriends[current_language]);

            std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
            if (!friends.empty())
            {
                if (ImGui::ListBoxHeader("##label", friends.size()))
                {
                    std::for_each(friends.begin(), friends.end(), [this](std::pair<Friend const, friend_window_state> &i)
                    {
                        ImGui::PushID(i.second.id-base_friend_window_id+base_friend_item_id);

                        ImGui::Selectable(i.second.window_title.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
                        BuildContextMenu(i.first, i.second);
                        if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0))
                        {
                            i.second.window_state |= window_state_show;
                        }
                        ImGui::PopID();

                        BuildFriendWindow(i.first, i.second);
                    });
                    ImGui::ListBoxFooter();
                }
            }

            if (show_achievements && achievements.size()) {
                ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetFontSize() * 32, ImGui::GetFontSize() * 32), ImVec2(8192, 8192));
                bool show = show_achievements;
                if (ImGui::Begin(translationAchievementWindow[current_language], &show)) {
                    ImGui::Text(translationListOfAchievements[current_language]);
                    ImGui::BeginChild(translationAchievements[current_language]);
                    for (auto & x : achievements) {
                        bool achieved = x.achieved;
                        bool hidden = x.hidden && !achieved;

                        if (x.icon.expired()) {
                            std::string file_path = Local_Storage::get_game_settings_path() + x.icon_name;
                            unsigned long long file_size = file_size_(file_path);
                            if (!file_size) {
                                file_path = Local_Storage::get_game_settings_path() + "achievement_images/" + x.icon_name;
                                file_size = file_size_(file_path);
                            }
                            if (file_size) {
                                std::string img = Local_Storage::load_image_resized(file_path, "", settings->overlay_appearance.icon_size);
                                if (img.length() > 0) {
                                    if (_renderer) x.icon = _renderer->CreateImageResource((void*)img.c_str(), settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size);
                                }
                            }
                        }
                        if (x.icon_gray.expired()) {
                            std::string file_path = Local_Storage::get_game_settings_path() + x.icon_gray_name;
                            unsigned long long file_size = file_size_(file_path);
                            if (!file_size) {
                                file_path = Local_Storage::get_game_settings_path() + "achievement_images/" + x.icon_gray_name;
                                file_size = file_size_(file_path);
                            }
                            if (file_size) {
                                std::string img = Local_Storage::load_image_resized(file_path, "", settings->overlay_appearance.icon_size);
                                if (img.length() > 0) {
                                    if (_renderer) x.icon_gray = _renderer->CreateImageResource((void*)img.c_str(), settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size);
                                }
                            }
                        }

                        ImGui::Separator();

                        if (!x.icon.expired() && !x.icon_gray.expired()) {
                            ImGui::BeginTable(x.title.c_str(), 2);
                            ImGui::TableSetupColumn("imgui_table_image", ImGuiTableColumnFlags_WidthFixed, settings->overlay_appearance.icon_size);
                            ImGui::TableSetupColumn("imgui_table_text");
                            ImGui::TableNextRow(ImGuiTableRowFlags_None, settings->overlay_appearance.icon_size);

                            ImGui::TableSetColumnIndex(0);
                            if (achieved) {
                                ImGui::Image((ImU64)*x.icon.lock().get(), ImVec2(settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size));
                            } else {
                                ImGui::Image((ImU64)*x.icon_gray.lock().get(), ImVec2(settings->overlay_appearance.icon_size, settings->overlay_appearance.icon_size));
                            }

                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%s", x.title.c_str());
                        } else {
                            ImGui::Text("%s", x.title.c_str());
                        }

                        if (hidden) {
                            ImGui::Text(translationHiddenAchievement[current_language]);
                        } else {
                            ImGui::TextWrapped("%s", x.description.c_str());
                        }

                        if (achieved) {
                            char buffer[80] = {};
                            time_t unlock_time = (time_t)x.unlock_time;
                            std::strftime(buffer, 80, "%Y-%m-%d at %H:%M:%S", std::localtime(&unlock_time));

                            ImGui::TextColored(ImVec4(0, 255, 0, 255), translationAchievedOn[current_language], buffer);
                        } else {
                            ImGui::TextColored(ImVec4(255, 0, 0, 255), translationNotAchieved[current_language]);
                        }

                        if (!x.icon.expired() && !x.icon_gray.expired()) ImGui::EndTable();

                        ImGui::Separator();
                    }
                    ImGui::EndChild();
                }
                ImGui::End();
                show_achievements = show;
            }

            if (show_settings) {
                if (ImGui::Begin(translationGlobalSettingsWindow[current_language], &show_settings)) {
                    ImGui::Text(translationGlobalSettingsWindowDescription[current_language]);

                    ImGui::Separator();

                    ImGui::Text(translationUsername[current_language]);
                    ImGui::SameLine();
                    ImGui::InputText("##username", username_text, sizeof(username_text), disable_user_input ? ImGuiInputTextFlags_ReadOnly : 0);

                    ImGui::Separator();

                    ImGui::Text(translationLanguage[current_language]);
                    ImGui::ListBox("##language", &current_language, valid_languages, sizeof(valid_languages) / sizeof(char *), 7);
                    ImGui::Text(translationSelectedLanguage[current_language], valid_languages[current_language]);

                    ImGui::Separator();

                    if (!disable_user_input) {
                        ImGui::Text(translationRestartTheGameToApply[current_language]);
                        if (ImGui::Button(translationSave[current_language])) {
                            save_settings = true;
                            show_settings = false;
                        }
                    } else {
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                        ImGui::TextWrapped(translationWarningDescription1[current_language]);
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                    }
                }

                ImGui::End();
            }

            std::string url = show_url;
            if (url.size()) {
                bool show = true;
                if (ImGui::Begin(URL_WINDOW_NAME, &show)) {
                    ImGui::Text(translationSteamOverlayURL[current_language]);
                    ImGui::Spacing();
                    ImGui::PushItemWidth(ImGui::CalcTextSize(url.c_str()).x + 20);
                    ImGui::InputText("##url_copy", (char *)url.data(), url.size(), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                    ImGui::Spacing();
                    if (ImGui::Button(translationClose[current_language]) || !show)
                        show_url = "";
                    // ImGui::SetWindowSize(ImVec2(ImGui::CalcTextSize(url.c_str()).x + 10, 0));
                }
                ImGui::End();
            }

            bool show_warning = warn_local_save || warn_forced_setting || warn_bad_appid;
            if (show_warning) {
                ImGui::SetNextWindowSizeConstraints(ImVec2(ImGui::GetFontSize() * 32, ImGui::GetFontSize() * 32), ImVec2(8192, 8192));
                ImGui::SetNextWindowFocus();
                if (ImGui::Begin(translationWarning[current_language], &show_warning)) {
                    if (warn_bad_appid) {
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                        ImGui::TextWrapped(translationWarningDescription2[current_language]);
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                    }
                    if (warn_local_save) {
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                        ImGui::TextWrapped(translationWarningDescription3[current_language]);
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                    }
                    if (warn_forced_setting) {
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                        ImGui::TextWrapped(translationWarningDescription4[current_language]);
                        ImGui::TextColored(ImVec4(255, 0, 0, 255), translationWarningWarningWarning[current_language]);
                    }
                }
                ImGui::End();
                if (!show_warning) {
                    warn_local_save = warn_forced_setting = false;
                }
            }
        }
        ImGui::End();

        ImGui::PopFont();

        if (!show)
            ShowOverlay(false);
    } else {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
}

void Steam_Overlay::Callback(Common_Message *msg)
{
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    if (msg->has_steam_messages())
    {
        Friend frd;
        frd.set_id(msg->source_id());
        auto friend_info = friends.find(frd);
        if (friend_info != friends.end())
        {
            Steam_Messages const& steam_message = msg->steam_messages();
            // Change color to cyan for friend
            friend_info->second.chat_history.append(friend_info->first.name() + ": " + steam_message.message()).append("\n", 1);
            if (!(friend_info->second.window_state & window_state_show))
            {
                friend_info->second.window_state |= window_state_need_attention;
            }

            AddMessageNotification(friend_info->first.name() + ": " + steam_message.message());
            NotifyUser(friend_info->second);
        }
    }
}

void Steam_Overlay::RunCallbacks()
{
    if (!achievements.size()) {
        Steam_User_Stats* steamUserStats = get_steam_client()->steam_user_stats;
        uint32 achievements_num = steamUserStats->GetNumAchievements();
        if (achievements_num) {
            PRINT_DEBUG("POPULATE OVERLAY ACHIEVEMENTS\n");
            for (unsigned i = 0; i < achievements_num; ++i) {
                Overlay_Achievement ach;
                ach.name = steamUserStats->GetAchievementName(i);
                ach.title = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "name");
                ach.description = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "desc");
                const char *hidden = steamUserStats->GetAchievementDisplayAttribute(ach.name.c_str(), "hidden");
                if (strlen(hidden) && hidden[0] == '1') {
                    ach.hidden = true;
                } else {
                    ach.hidden = false;
                }

                bool achieved = false;
                uint32 unlock_time = 0;
                if (steamUserStats->GetAchievementAndUnlockTime(ach.name.c_str(), &achieved, &unlock_time)) {
                    ach.achieved = achieved;
                    ach.unlock_time = unlock_time;
                } else {
                    ach.achieved = false;
                    ach.unlock_time = 0;
                }

                ach.icon_name = steamUserStats->get_achievement_icon_name(ach.name.c_str(), true);
                ach.icon_gray_name = steamUserStats->get_achievement_icon_name(ach.name.c_str(), false);

                achievements.push_back(ach);
            }

            PRINT_DEBUG("POPULATE OVERLAY ACHIEVEMENTS DONE\n");
        }
    }

    if (!Ready() && future_renderer.valid()) {
        if (future_renderer.wait_for(std::chrono::milliseconds{0}) ==  std::future_status::ready) {
            _renderer = future_renderer.get();
            PRINT_DEBUG("got renderer %p\n", _renderer);
            CreateFonts();
            LoadAudio();
        }
    }

    if (!Ready() && _renderer) {
            _renderer->OverlayHookReady = std::bind(&Steam_Overlay::HookReady, this, std::placeholders::_1);
            _renderer->OverlayProc = std::bind(&Steam_Overlay::OverlayProc, this);
            auto callback = std::bind(&Steam_Overlay::OpenOverlayHook, this, std::placeholders::_1);
            PRINT_DEBUG("start renderer\n", _renderer);
            std::set<ingame_overlay::ToggleKey> keys = {ingame_overlay::ToggleKey::SHIFT, ingame_overlay::ToggleKey::TAB};
            _renderer->ImGuiFontAtlas = fonts_atlas;
            bool started = _renderer->StartHook(callback, keys);
            PRINT_DEBUG("tried to start renderer %u\n", started);
    }

    if (overlay_state_changed)
    {
        GameOverlayActivated_t data = { 0 };
        data.m_bActive = show_overlay;
        data.m_bUserInitiated = true;
        data.m_nAppID = settings->get_local_game_id().AppID();
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

        overlay_state_changed = false;
    }

    Steam_Friends* steamFriends = get_steam_client()->steam_friends;
    Steam_Matchmaking* steamMatchmaking = get_steam_client()->steam_matchmaking;

    if (save_settings) {
        char *language_text = valid_languages[current_language];
        save_global_settings(get_steam_client()->local_storage, username_text, language_text);
        get_steam_client()->settings_client->set_local_name(username_text);
        get_steam_client()->settings_server->set_local_name(username_text);
        get_steam_client()->settings_client->set_language(language_text);
        get_steam_client()->settings_server->set_language(language_text);
        steamFriends->resend_friend_data();
        save_settings = false;
    }

    i_have_lobby = IHaveLobby();
    std::lock_guard<std::recursive_mutex> lock(overlay_mutex);
    std::for_each(friends.begin(), friends.end(), [this](std::pair<Friend const, friend_window_state> &i)
    {
        i.second.joinable = FriendJoinable(i);
    });

    while (!has_friend_action.empty())
    {
        auto friend_info = friends.find(has_friend_action.front());
        if (friend_info != friends.end())
        {
            uint64 friend_id = friend_info->first.id();
            // The user clicked on "Send"
            if (friend_info->second.window_state & window_state_send_message)
            {
                char* input = friend_info->second.chat_input;
                char* end_input = input + strlen(input);
                char* printable_char = std::find_if(input, end_input, [](char c) {
                    return std::isgraph(c);
                });
                // Check if the message contains something else than blanks
                if (printable_char != end_input)
                {
                    // Handle chat send
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
                *input = 0; // Reset the input field
                friend_info->second.window_state &= ~window_state_send_message;
            }
            // The user clicked on "Invite"
            if (friend_info->second.window_state & window_state_invite)
            {
                std::string connect = steamFriends->GetFriendRichPresence(settings->get_local_steam_id(), "connect");
                if (connect.length() > 0)
                    steamFriends->InviteUserToGame(friend_id, connect.c_str());
                else if (settings->get_lobby().IsValid())
                    steamMatchmaking->InviteUserToLobby(settings->get_lobby(), friend_id);

                friend_info->second.window_state &= ~window_state_invite;
            }
            // The user clicked on "Join"
            if (friend_info->second.window_state & window_state_join)
            {
                std::string connect = steamFriends->GetFriendRichPresence(friend_id, "connect");
                // The user got a lobby invite and accepted it
                if (friend_info->second.window_state & window_state_lobby_invite)
                {
                    GameLobbyJoinRequested_t data;
                    data.m_steamIDLobby.SetFromUint64(friend_info->second.lobbyId);
                    data.m_steamIDFriend.SetFromUint64(friend_id);
                    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

                    friend_info->second.window_state &= ~window_state_lobby_invite;
                } else {
                // The user got a rich presence invite and accepted it
                    if (friend_info->second.window_state & window_state_rich_invite)
                    {
                        GameRichPresenceJoinRequested_t data = {};
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        strncpy(data.m_rgchConnect, friend_info->second.connect, k_cchMaxRichPresenceValueLength - 1);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

                        friend_info->second.window_state &= ~window_state_rich_invite;
                    } else if (connect.length() > 0)
                    {
                        GameRichPresenceJoinRequested_t data = {};
                        data.m_steamIDFriend.SetFromUint64(friend_id);
                        strncpy(data.m_rgchConnect, connect.c_str(), k_cchMaxRichPresenceValueLength - 1);
                        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                    }

                    //Not sure about this but it fixes sonic racing transformed invites
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

#endif
