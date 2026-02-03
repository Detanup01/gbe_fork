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

#ifndef __INCLUDED_STEAM_GAMESTATS_H__
#define __INCLUDED_STEAM_GAMESTATS_H__

#include "base.h"

//-----------------------------------------------------------------------------
// Purpose: Functions for recording game play sessions and details thereof
//-----------------------------------------------------------------------------
class Steam_GameStats :
public ISteamGameStats
{
private:
  // how much time to wait before removing ended sessions
  constexpr const static int MAX_DEAD_SESSION_SECONDS = 15; // TODO not sure what would be sensible in this case

  enum class AttributeType_t
  {
    Int, Str, Float, Int64,
  };

  struct Attribute_t
  {
  private:
    AttributeType_t type;
  
  public:
    union {
      int32 n_data;
      std::string s_data;
      float f_data;
      int64 ll_data;
    };

    Attribute_t() = delete;
    Attribute_t(AttributeType_t type);
    Attribute_t(const Attribute_t &other);
    Attribute_t(Attribute_t &&other);
    ~Attribute_t();

    Attribute_t& operator=(const Attribute_t &other);
    Attribute_t& operator=(Attribute_t &&other);

    AttributeType_t get_type() const noexcept;
  };

  struct Row_t
  {
    bool committed = false;
    std::map<std::string, Attribute_t> attributes{};
  };

  struct Table_t
  {
    std::vector<Row_t> rows{};
  };

  struct Session_t
  {
    bool ended = false;
    bool saved_to_disk = false;
    uint64 account_id{};

    EGameStatsAccountType nAccountType{};
    RTime32 rtTimeStarted{};
    RTime32 rtTimeEnded{};
    int nReasonCode{};
    std::map<std::string, Attribute_t> attributes{};

    std::vector<std::pair<std::string, Table_t>> tables{};
  };

  class Settings *settings{};
  class Networking *network{};
  class SteamCallResults *callback_results{};
  class SteamCallBacks *callbacks{};
  class RunEveryRunCB *run_every_runcb{};
  
  std::map<uint64, Session_t> sessions{};


  uint64 create_session_id() const;
  bool valid_stats_account_type(int8 nAccountType) const noexcept;
  Table_t *get_or_create_session_table(Session_t &session, const char *table_name);
  Attribute_t *get_or_create_session_att(const char *att_name, Session_t &session, AttributeType_t type_if_create);
  Attribute_t *get_or_create_row_att(uint64 ulRowID, const char *att_name, Table_t &table, AttributeType_t type_if_create);
  Session_t* get_last_active_session();

  void save_session_to_disk(Steam_GameStats::Session_t &session, uint64 session_id);
  void steam_run_callback();

  // user connect/disconnect
  void network_callback_low_level(Common_Message *msg);

  static void steam_gamestats_network_low_level(void *object, Common_Message *msg);
  static void steam_gamestats_run_every_runcb(void *object);


public:
  Steam_GameStats(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
  ~Steam_GameStats();
  
  SteamAPICall_t GetNewSession( int8 nAccountType, uint64 ulAccountID, int32 nAppID, RTime32 rtTimeStarted );
  SteamAPICall_t EndSession( uint64 ulSessionID, RTime32 rtTimeEnded, int nReasonCode );
  EResult AddSessionAttributeInt( uint64 ulSessionID, const char* pstrName, int32 nData );
  EResult AddSessionAttributeString( uint64 ulSessionID, const char* pstrName, const char *pstrData );
  EResult AddSessionAttributeFloat( uint64 ulSessionID, const char* pstrName, float fData );

  EResult AddNewRow( uint64 *pulRowID, uint64 ulSessionID, const char *pstrTableName );
  EResult CommitRow( uint64 ulRowID );
  EResult CommitOutstandingRows( uint64 ulSessionID );
  EResult AddRowAttributeInt( uint64 ulRowID, const char *pstrName, int32 nData );
  EResult AddRowAtributeString( uint64 ulRowID, const char *pstrName, const char *pstrData );
  EResult AddRowAttributeFloat( uint64 ulRowID, const char *pstrName, float fData );

  EResult AddSessionAttributeInt64( uint64 ulSessionID, const char *pstrName, int64 llData );
  EResult AddRowAttributeInt64( uint64 ulRowID, const char *pstrName, int64 llData );

};

#endif // __INCLUDED_STEAM_GAMESTATS_H__
