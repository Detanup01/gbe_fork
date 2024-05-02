/* Copyright (C) 2019 Mr Goldberg
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

#ifndef BASE_INCLUDE_H
#define BASE_INCLUDE_H

#include "common_includes.h"
#include "callsystem.h"

#define PUSH_BACK_IF_NOT_IN(vector, element) { if(std::find(vector.begin(), vector.end(), element) == vector.end()) vector.push_back(element); }


extern std::recursive_mutex global_mutex;
extern const std::chrono::time_point<std::chrono::high_resolution_clock> startup_counter;
extern const std::chrono::time_point<std::chrono::system_clock> startup_time;

void randombytes(char *buf, size_t size);
std::string get_env_variable(const std::string &name);
bool set_env_variable(const std::string &name, const std::string &value);

/// @brief Check for a timeout given some initial timepoint and a timeout in sec.
/// @param old The initial timepoint which will be compared against current time
/// @param timeout The max allowed time in seconds
/// @return true if the timepoint has exceeded the max allowed timeout, false otherwise
bool check_timedout(std::chrono::high_resolution_clock::time_point old, double timeout);

unsigned generate_account_id();
CSteamID generate_steam_anon_user();
SteamAPICall_t generate_steam_api_call_id();
CSteamID generate_steam_id_user();
CSteamID generate_steam_id_server();
CSteamID generate_steam_id_anonserver();
CSteamID generate_steam_id_lobby();
std::string get_full_lib_path();
std::string get_full_program_path();
std::string get_current_path();
std::string canonical_path(const std::string &path);
bool file_exists_(const std::string &full_path);
unsigned int file_size_(const std::string &full_path);

void set_whitelist_ips(uint32_t *from, uint32_t *to, unsigned num_ips);
#ifdef EMU_EXPERIMENTAL_BUILD
bool crack_SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID);
bool crack_SteamAPI_Init();
#endif

#endif // BASE_INCLUDE_H
