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

#ifndef __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__
#define __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__

#include "base.h"

class Ugc_Remote_Storage_Bridge
{
public:
    struct QueryInfo {
        PublishedFileId_t mod_id{}; // mod id
        bool is_primary_file{}; // was this query for the primary mod file or preview file
    };

private:
    class Settings *settings{};
    // key: UGCHandle_t which is the file handle (primary or preview)
    // value: the mod id, true if UGCHandle_t of primary file | false if UGCHandle_t of preview file
    std::map<UGCHandle_t, QueryInfo> steam_ugc_queries{};
    std::set<PublishedFileId_t> subscribed{}; // just to keep the running state of subscription

public:
    Ugc_Remote_Storage_Bridge(class Settings *settings);
    ~Ugc_Remote_Storage_Bridge();
    
    // called from Steam_UGC::SendQueryUGCRequest() after a successful query
    void add_ugc_query_result(UGCHandle_t file_handle, PublishedFileId_t fileid, bool handle_of_primary_file);
    bool remove_ugc_query_result(UGCHandle_t file_handle);
    std::optional<QueryInfo> get_ugc_query_result(UGCHandle_t file_handle) const;

    void add_subbed_mod(PublishedFileId_t id);
    void remove_subbed_mod(PublishedFileId_t id);
    size_t subbed_mods_count() const;
    bool has_subbed_mod(PublishedFileId_t id) const;
    std::set<PublishedFileId_t>::iterator subbed_mods_itr_begin() const;
    std::set<PublishedFileId_t>::iterator subbed_mods_itr_end() const;

};


#endif // __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__

