/* Copyright (C) 2019 Mr Goldberg, , Nemirtingas
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "dll/source_query.h"
#include "dll/dll.h"

using lock_t = std::lock_guard<std::mutex>;

enum class source_query_magic : uint32_t
{
    simple = 0xFFFFFFFFul,
    multi  = 0xFFFFFFFEul, // <--- TODO ?
};

enum class source_query_header : uint8_t
{
    A2S_INFO   = 'T',
    A2S_PLAYER = 'U',
    A2S_RULES  = 'V',
};

enum class source_response_header : uint8_t
{
    A2S_CHALLENGE = 'A',
    A2S_INFO      = 'I',
    A2S_PLAYER    = 'D',
    A2S_RULES     = 'E',
};

enum class source_server_type : uint8_t
{
    dedicated     = 'd',
    non_dedicated = 'i',
    source_tc     = 'p',
};

enum class source_server_env : uint8_t
{
    linux   = 'l',
    windows = 'w',
    old_mac = 'm',
    mac     = 'o',
};

enum class source_server_visibility : uint8_t
{
    _public  = 0,
    _private = 1,
};

enum class source_server_vac : uint8_t
{
    unsecured = 0,
    secured   = 1,
};

enum source_server_extra_flag : uint8_t
{
    none      = 0x00,
    gameid    = 0x01,
    steamid   = 0x10,
    keywords  = 0x20,
    spectator = 0x40,
    port      = 0x80,
};

#if defined(STEAM_WIN32)
static constexpr source_server_env my_server_env = source_server_env::windows;
#else
static constexpr source_server_env my_server_env = source_server_env::linux;
#endif

#pragma pack(push)
#pragma pack(1)

constexpr char a2s_info_payload[]      = "Source Engine Query";
constexpr size_t a2s_info_payload_size = sizeof(a2s_info_payload);

struct source_query_data
{
    source_query_magic magic;
    source_query_header header;
    union
    {
        char a2s_info_payload[a2s_info_payload_size];
        uint32_t challenge;
    };
};

static constexpr size_t source_query_header_size = sizeof(source_query_magic) + sizeof(source_query_header);
static constexpr size_t a2s_query_info_size      = source_query_header_size + sizeof(source_query_data::a2s_info_payload);
static constexpr size_t a2s_query_challenge_size = source_query_header_size + sizeof(source_query_data::challenge);

#pragma pack(pop)

void serialize_response(std::vector<uint8_t>& buffer, const void* _data, size_t len)
{
    const uint8_t* data = reinterpret_cast<const uint8_t*>(_data);

    buffer.insert(buffer.end(), data, data + len);
}

template<typename T>
void serialize_response(std::vector<uint8_t>& buffer, T const& v)
{
    uint8_t const* data = reinterpret_cast<uint8_t const*>(&v);
    serialize_response(buffer, data, sizeof(T));
}

template<>
void serialize_response<std::string>(std::vector<uint8_t>& buffer, std::string const& v)
{
    uint8_t const* str = reinterpret_cast<uint8_t const*>(v.c_str());
    serialize_response(buffer, str, v.length()+1);
}

template<typename T, int N>
void serialize_response(std::vector <uint8_t>& buffer, char(&str)[N])
{
    serialize_response(buffer, reinterpret_cast<uint8_t const*>(str), N);
}

void get_challenge(std::vector<uint8_t> &challenge_buff)
{
    // TODO: generate the challenge id
    serialize_response(challenge_buff, source_query_magic::simple);
    serialize_response(challenge_buff, source_response_header::A2S_CHALLENGE);
    serialize_response(challenge_buff, static_cast<uint32_t>(0x00112233ul));
}

std::vector<uint8_t> Source_Query::handle_source_query(const void* buffer, size_t len, Gameserver const& gs)
{
    std::vector<uint8_t> output_buffer;

    if (len < source_query_header_size) // its not at least 5 bytes long (0xFF 0xFF 0xFF 0xFF 0x??)
        return output_buffer;

    source_query_data const& query = *reinterpret_cast<source_query_data const*>(buffer);

    // || gs.max_player_count() == 0
    if (gs.offline() || query.magic != source_query_magic::simple)
        return output_buffer;

    switch (query.header)
    {
    case source_query_header::A2S_INFO:
        if (len >= a2s_query_info_size && !strncmp(query.a2s_info_payload, a2s_info_payload, a2s_info_payload_size))
        {
            std::vector<std::pair<CSteamID, Gameserver_Player_Info_t>> const& players = *get_steam_client()->steam_gameserver->get_players();

            serialize_response(output_buffer, source_query_magic::simple);
            serialize_response(output_buffer, source_response_header::A2S_INFO);
            serialize_response(output_buffer, static_cast<uint8_t>(2));
            serialize_response(output_buffer, gs.server_name());
            serialize_response(output_buffer, gs.map_name());
            serialize_response(output_buffer, gs.mod_dir());
            serialize_response(output_buffer, gs.product());
            serialize_response(output_buffer, static_cast<uint16_t>(gs.appid()));
            serialize_response(output_buffer, static_cast<uint8_t>(players.size()));
            serialize_response(output_buffer, static_cast<uint8_t>(gs.max_player_count()));
            serialize_response(output_buffer, static_cast<uint8_t>(gs.bot_player_count()));
            serialize_response(output_buffer, (gs.dedicated_server() ? source_server_type::dedicated : source_server_type::non_dedicated));;
            serialize_response(output_buffer, my_server_env);
            serialize_response(output_buffer, (gs.password_protected() ? source_server_visibility::_private : source_server_visibility::_public));
            serialize_response(output_buffer, (gs.secure() ? source_server_vac::secured : source_server_vac::unsecured));
            serialize_response(output_buffer, std::to_string(gs.version()));

            uint8_t flags = source_server_extra_flag::none;

            if (gs.port() != 0)
                flags |= source_server_extra_flag::port;

            if (gs.spectator_port() != 0)
                flags |= source_server_extra_flag::spectator;

            if(CGameID(gs.appid()).IsValid())
                flags |= source_server_extra_flag::gameid;

            if (flags != source_server_extra_flag::none)
                serialize_response(output_buffer, flags);

            if (flags & source_server_extra_flag::port)
                serialize_response(output_buffer, static_cast<uint16_t>(gs.port()));

            // add steamid

            if (flags & source_server_extra_flag::spectator)
            {
                serialize_response(output_buffer, static_cast<uint16_t>(gs.spectator_port()));
                serialize_response(output_buffer, gs.spectator_server_name());
            }

            // keywords

            if (flags & source_server_extra_flag::gameid)
                serialize_response(output_buffer, CGameID(gs.appid()).ToUint64());

        }
        break;

    case source_query_header::A2S_PLAYER:
        if (len >= a2s_query_challenge_size)
        {
            if (query.challenge == 0xFFFFFFFFul)
            {
                get_challenge(output_buffer);
            }
            else if (query.challenge == 0x00112233ul)
            {
                std::vector<std::pair<CSteamID, Gameserver_Player_Info_t>> const& players = *get_steam_client()->steam_gameserver->get_players();

                serialize_response(output_buffer, source_query_magic::simple);
                serialize_response(output_buffer, source_response_header::A2S_PLAYER);
                serialize_response(output_buffer, static_cast<uint8_t>(players.size())); // num_players

                for (int i = 0; i < players.size(); ++i)
                {
                    serialize_response(output_buffer, static_cast<uint8_t>(i)); // player index
                    serialize_response(output_buffer, players[i].second.name); // player name
                    serialize_response(output_buffer, players[i].second.score); // player score
                    serialize_response(output_buffer, static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - players[i].second.join_time).count()));
                }

            }
        }
        break;

    case source_query_header::A2S_RULES:
        if (len >= a2s_query_challenge_size)
        {
            if (query.challenge == 0xFFFFFFFFul)
            {
                get_challenge(output_buffer);
            }
            else if (query.challenge == 0x00112233ul)
            {
                auto values = gs.values();

                serialize_response(output_buffer, source_query_magic::simple);
                serialize_response(output_buffer, source_response_header::A2S_RULES);
                serialize_response(output_buffer, static_cast<uint16_t>(values.size()));

                for (auto const& i : values)
                {
                    serialize_response(output_buffer, i.first);
                    serialize_response(output_buffer, i.second);
                }
            }
        }
        break;
    }
    return output_buffer;
}
