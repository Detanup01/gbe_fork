# What is the interface ISteamGameServerStats
This interface is relevant for game servers only, and responsible for sharing stats and achievements with the client. Some games (like appid 541300) might not handle user stats on their own, but rather expect the server to send the updated/changed user data.  

While playing a game, the game client will send the player's current progress to the server, example: *the player ate an apple*, or *the player killed 5 enemies*, etc...  
The server keeps track of the player progress, and once the player hits a certain condition (ex: eating an apple for the first time), the server will share an achievement with the game client, or send the current progress as a user statistics metric (ex: killing 5/10/50 enemies).  


# Overview of the interface functions
* On startup, the server will ask Valve servers to *prepare* the user data by calling `RequestUserStats()` and wait for the response
* The server then, at any point, can ask for specific user stat/achievement
  - `GetUserStat()` will return the current value for a given stat name, ex: amount of killed enemies
  - `GetUserAchievement()` will return whether the user has unlocked a given achievement or not
* The server can also change/update this data
  - `SetUserStat()` and `UpdateUserAvgRateStat()` will update the stat value
  - `SetUserAchievement()` and `ClearUserAchievement()` will grant or remove from the user a given achievement
* Finally, the server should upload the changed data back to Valve servers by calling `StoreUserStats()`


# How it is implemented in the emu
For starters, the emu doesn't offer a mechanism to emulate a central server, also all user stats and achievements are saved locally on the user's computer.  

Let's say we have 3 people currently playing a game on the same network which has a dedicated server, and the server is utilizing this interface to share stats.  

We'll implement the interface in a way such that each person is their own central server, and each will either broadcast their data, or only send it to the dedicated server:  


# Implementation of `RequestUserStats()`
When the server asks for the user data via `RequestUserStats()` we'll send a request to that user, wait for their response, and finally trigger a callresult + a callback.
We'll also store each user data, in a hash map (dictionary) for later so when we change that data we know whose data are we changing.  

It is fairly straightforward, one request from the server, with its corresponding response from the player/user. It is a directed one-to-one message.  
```
|-------------|  ====>>  |-------------|
|    server   |          | game client |
|-------------|  <<====  |-------------|
```

- Send a `protobuff` message to the user asking for all their stats
  ```proto
  enum Types {
      ...
      Request_AllUserStats = 0;
      ...
  }
  // sent from server as a request, response sent by the user
  message InitialAllStats {
      ...
      uint64 steam_api_call = 1;
      ...
  }
  
  Types type = 1;
  oneof data_messages {
      InitialAllStats initial_user_stats = 2;
      ...
  }
  ```
  ```c++
  // SteamAPICall_t Steam_GameServerStats::RequestUserStats( CSteamID steamIDUser )
  
  auto initial_stats_msg = new GameServerStats_Messages::InitialAllStats();
  initial_stats_msg->set_steam_api_call(new_request.steamAPICall);

  auto gameserverstats_messages = new GameServerStats_Messages();
  gameserverstats_messages->set_typ(GameServerStats_Messages::Request_AllUserStats);
  gameserverstats_messages->set_allocated_initial_user_stats(initial_stats_msg);
  
  Common_Message msg{};
  // https://protobuf.dev/reference/cpp/cpp-generated/#string
  // set_allocated_xxx() takes ownership of the allocated object, no need to delete
  msg.set_allocated_gameserver_stats_messages(gameserverstats_messages);
  msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
  msg.set_dest_id(new_request.steamIDUser.ConvertToUint64());
  network->sendTo(&msg, true);
  ```

- The user will send back a `protobuff` message containing all the data, this is the user response with the enum `Type` set to `Response_AllUserStats`
  ```proto
    enum Types {
        ...
        Response_AllUserStats = 1;
        ...
    }
    
    message InitialAllStats {
        uint64 steam_api_call = 1;
        
        // optional because the server send doesn't send any data, just steam api   call id
        optional AllStats all_data = 2;
    }
    
    // this is used when updating stats, from server or user, bi-directional
    message AllStats {
        map<string, StatInfo> user_stats = 1;
        map<string, AchievementInfo> user_achievements = 2;
    }
    
    message StatInfo {
        enum Stat_Type {
            STAT_TYPE_INT = 0;
            STAT_TYPE_FLOAT = 1;
            STAT_TYPE_AVGRATE = 2;
        }
        message AvgStatInfo {
            float count_this_session = 1;
            double session_length = 2;
        }

        Stat_Type stat_type = 1;
        oneof stat_value {
            float value_float = 2;
            int32 value_int = 3;
        }
        ...
    }
    message AchievementInfo {
        bool achieved = 1;
    }

	Types type = 1;
    oneof data_messages {
        ...
        InitialAllStats initial_user_stats = 2;
        ...
    }
  ```
  ```c++
  // void Steam_User_Stats::network_stats_initial(Common_Message *msg)

  auto initial_stats_msg = new GameServerStats_Messages::InitialAllStats();
  // send back same api call id
  initial_stats_msg->set_steam_api_call(msg->gameserver_stats_messages()  initial_user_stats().steam_api_call());
  initial_stats_msg->set_allocated_all_data(all_stats_msg);

  auto gameserverstats_msg = new GameServerStats_Messages();
  gameserverstats_msg->set_type(GameServerStats_Messages::Response_AllUserStats);
  gameserverstats_msg->set_allocated_initial_user_stats(initial_stats_msg);
  
  new_msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
  new_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
  new_msg.set_dest_id(server_steamid);
  network->sendTo(&new_msg, true);
  ```
- When the user returns a reposponse, we'll trigger a callback + a callresult
  ```c++
  // void Steam_GameServerStats::network_callback_initial_stats(Common_Message *msg)
  GSStatsReceived_t data{};
  data.m_eResult = EResult::k_EResultOK;
  data.m_steamIDUser = user_steamid;
  callback_results->addCallResult(it->steamAPICall, data.k_iCallback, &data, sizeof(data));
  callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
  ```


# Implementation of `SetUserStat()`, `SetUserAchievement()`, and `ClearUserAchievement()`
The emu already asked the user earlier via `RequestUserStats()` for their data and stored the result in a map/dictionary, so whenver the server calls any of these functions we can easily update the dictionary.  

But when we send the updated data to the user we don't want to send the entire dictionary, it is wasteful but more importantly, if we keep sending the same **unchanged** data each time over and over, the Steam functions on the player's side will trigger a notification each time, meaning that the player might keep unlocking an achievement over and over or updating the same stats with the exact same values.  

To solve this, the emu keeps a dictionary of *cached* data, each piece of data has a an accompanied boolean flag called `dirty`, this flag is set to `true` only when the server updates the data, and when it's time to send the new data to the user, we only collect those whose `dirty` flag is set.
```c++
struct CachedStat {
    bool dirty = false; // true means it was changed on the server and should be sent to the user
    GameServerStats_Messages::StatInfo stat{};
};
struct CachedAchievement {
    bool dirty = false; // true means it was changed on the server and should be sent to the user
    GameServerStats_Messages::AchievementInfo ach{};
};

struct UserData {
    std::map<std::string, CachedStat> stats{};
    std::map<std::string, CachedAchievement> achievements{};
};

// dictionary of <user id, user data>
std::map<uint64, UserData> all_users_data{};
```

An example from `SetUserAchievement()`
```c++
auto ach = find_ach(steamIDUser, pchName);
if (!ach) return false;
if (ach->ach.achieved() == true) return true; // don't waste time

ach->dirty = true; // set the dirty flag
```

Another optimization made here is that the data is not sent immediately, game servers and game clients utilizing the Steam networking will always call `Steam_Client::RunCallbacks()` periodically, so we can just for that periodic call and send any *dirty* data all at once, or nothing if everything is clean! (unchanged).  


Here's the `protobuff` message, and notice how it's exactly the same as the *user/player response* for `RequestUserStats()`, with these exceptions:
1. This is sent from the server, not the user/player
2. The enum `Type` is set to `UpdateUserStatsFromServer`
3. The active member in the `oneof data_messages` is `update_user_stats`

But the overall message has the same shape, and now contains **only** the changed data
```proto
enum Types {
    ...
    UpdateUserStatsFromServer = 2; // sent by Steam_GameServerStats
    ...
}

// this is used when updating stats, from server or user, bi-directional
message AllStats {
    map<string, StatInfo> user_stats = 1;
    map<string, AchievementInfo> user_achievements = 2;
}

message StatInfo {
    enum Stat_Type {
        STAT_TYPE_INT = 0;
        STAT_TYPE_FLOAT = 1;
        STAT_TYPE_AVGRATE = 2;
    }
    message AvgStatInfo {
        float count_this_session = 1;
        double session_length = 2;
    }

    Stat_Type stat_type = 1;
    oneof stat_value {
        float value_float = 2;
        int32 value_int = 3;
    }
    optional AvgStatInfo value_avg = 4; // only set when type != INT
}
message AchievementInfo {
    bool achieved = 1;
}

Types type = 1;
oneof data_messages {
    ...
    AllStats update_user_stats = 3;
    ...
}
```
```c++
// void Steam_GameServerStats::collect_and_send_updated_user_stats()

// collect all dirty stats

// collect all dirty achievements

// then for each user, send the dirty data at once as a single packet

auto gameserverstats_msg = new GameServerStats_Messages();
gameserverstats_msg->set_type(GameServerStats_Messages::UpdateUserStatsFromServer);
gameserverstats_msg->set_allocated_update_user_stats(updated_stats_msg);

Common_Message msg{};
// https://protobuf.dev/reference/cpp/cpp-generated/#string
// set_allocated_xxx() takes ownership of the allocated object, no need to delete
msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
msg.set_dest_id(user_steamid);
network->sendTo(&msg, true);
```

Back on the user/client side, they will receive this message and update their data, as if the game itself has updated this data.


# How data is shared with game servers if the game client updated its data
This more or less the same, with these changes
* The `protobuff` enum `Type` is set to `UpdateUserStatsFromUser`
* Since the game client doesn't know the server ID, it will broadcast the message to all game servers

```c++
// void Steam_User_Stats::send_updated_stats()

auto gameserverstats_msg = new GameServerStats_Messages();
gameserverstats_msg->set_type(GameServerStats_Messages::UpdateUserStatsFromUser);
gameserverstats_msg->set_allocated_update_user_stats(new_updates_msg);

Common_Message msg{};
// https://protobuf.dev/reference/cpp/cpp-generated/#string
// set_allocated_xxx() takes ownership of the allocated object, no need to delete
msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
// here we send to all gameservers on the network because we don't know the server steamid
network->sendToAllGameservers(&msg, true);
```


# Changes made to the networking
The networking implementation works like this  
* Store a list of all functions to trigger once a certain message is received, messages are tagged with some ID
  ```c++
  enum Callback_Ids {
      CALLBACK_ID_USER_STATUS,
      ...
      CALLBACK_ID_GAMESERVER_STATS, // this is our new member
      ...
  
      CALLBACK_IDS_MAX
  };
  ```

  Internally this is implemented as a static array, not a map/dictionary as one might expect
  ```c++
  struct Network_Callback_Container callbacks[CALLBACK_IDS_MAX];
  ```
  No need for a map here since the *keys* are static numbers known during compilation (the enum above), hence an array is equivalent to a map/dictionary here.  

  Each element of the array is just a collection of functions to be called/invoked later
  ```c++
  struct Network_Callback_Container {
    std::vector<struct Network_Callback> callbacks{};
  };
  ```
  
  ```
                                    ---------------------------
  CALLBACK_ID_USER_STATUS      ---> | &func_1 | &func_2 | ... |
                                    ---------------------------
  
                                    -----------------------------------
  CALLBACK_ID_GAMESERVER_STATS ---> | &other_fn_1 | &other_fn_1 | ... |
                                    -----------------------------------
  ```

* You can subscribe/listen to messages of that type or unsubscribe using these functions
  ```c++
  // subscribe
  Networking::setCallback(...)

  // unsubscribe
  Networking::rmCallback(...)
  ```

* The networking class has an event-based function, called whenever a network message is available, which will check for the message type, and trigger/call each subscriber
  ```c++
  void Networking::do_callbacks_message(Common_Message *msg) {
    ...
    
    if (msg->has_gameserver_stats_messages()) {
        PRINT_DEBUG("has_gameserver_stats");
        run_callbacks(CALLBACK_ID_GAMESERVER_STATS, msg);
    }
    
    ...
  }
  ```
