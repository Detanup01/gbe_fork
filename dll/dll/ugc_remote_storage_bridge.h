
#ifndef __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__
#define __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__

#include "base.h"

class Ugc_Remote_Storage_Bridge
{
public:
    struct QueryInfo {
        PublishedFileId_t mod_id; // mod id
        bool is_primary_file; // was this query for the primary mod file or preview file
    };

private:
    // key: UGCHandle_t which is the file handle (primary or preview)
    // value: the mod id, true if UGCHandle_t of primary file | false if UGCHandle_t of preview file
    std::map<UGCHandle_t, QueryInfo> steam_ugc_queries{};

public:
    // called from Steam_UGC::SendQueryUGCRequest() after a successful query
    void add_ugc_query_result(UGCHandle_t file_handle, PublishedFileId_t fileid, bool handle_of_primary_file);
    
    bool remove_ugc_query_result(UGCHandle_t file_handle);

    std::optional<QueryInfo> get_ugc_query_result(UGCHandle_t file_handle);

    ~Ugc_Remote_Storage_Bridge();
};


#endif // __INCLUDED_UGC_REMOTE_STORAGE_BRIDGE_H__

