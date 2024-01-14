#include "dll/ugc_remote_storage_bridge.h"


void Ugc_Remote_Storage_Bridge::add_ugc_query_result(UGCHandle_t file_handle, PublishedFileId_t fileid, bool handle_of_primary_file)
{
    std::lock_guard lock(global_mutex);

    steam_ugc_queries[file_handle].mod_id = fileid;
    steam_ugc_queries[file_handle].is_primary_file = handle_of_primary_file;
}

bool Ugc_Remote_Storage_Bridge::remove_ugc_query_result(UGCHandle_t file_handle)
{
    std::lock_guard lock(global_mutex);

    return !!steam_ugc_queries.erase(file_handle);
}

std::optional<Ugc_Remote_Storage_Bridge::QueryInfo> Ugc_Remote_Storage_Bridge::get_ugc_query_result(UGCHandle_t file_handle)
{
    std::lock_guard lock(global_mutex);
    
    auto it = steam_ugc_queries.find(file_handle);
    if (steam_ugc_queries.end() == it) return std::nullopt;
    return it->second;
}

Ugc_Remote_Storage_Bridge::~Ugc_Remote_Storage_Bridge()
{
    std::lock_guard lock(global_mutex);
    
    steam_ugc_queries.clear();
}
