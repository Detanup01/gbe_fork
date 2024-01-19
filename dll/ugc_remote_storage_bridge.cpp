#include "dll/ugc_remote_storage_bridge.h"


Ugc_Remote_Storage_Bridge::Ugc_Remote_Storage_Bridge(class Settings *settings)
{
    this->settings = settings;

    // subscribe to all mods initially
    subscribed = settings->modSet();
}

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

std::optional<Ugc_Remote_Storage_Bridge::QueryInfo> Ugc_Remote_Storage_Bridge::get_ugc_query_result(UGCHandle_t file_handle) const
{
    std::lock_guard lock(global_mutex);
    
    auto it = steam_ugc_queries.find(file_handle);
    if (steam_ugc_queries.end() == it) return std::nullopt;
    return it->second;
}

void Ugc_Remote_Storage_Bridge::add_subbed_mod(PublishedFileId_t id)
{
    subscribed.insert(id);
}

void Ugc_Remote_Storage_Bridge::remove_subbed_mod(PublishedFileId_t id)
{
    subscribed.erase(id);
}

size_t Ugc_Remote_Storage_Bridge::subbed_mods_count() const
{
    return subscribed.size();
}

bool Ugc_Remote_Storage_Bridge::has_subbed_mod(PublishedFileId_t id) const
{
    return !!subscribed.count(id);
}

std::set<PublishedFileId_t>::iterator Ugc_Remote_Storage_Bridge::subbed_mods_itr_begin() const
{
    return subscribed.begin();
}

std::set<PublishedFileId_t>::iterator Ugc_Remote_Storage_Bridge::subbed_mods_itr_end() const
{
    return subscribed.end();
}

Ugc_Remote_Storage_Bridge::~Ugc_Remote_Storage_Bridge()
{
    std::lock_guard lock(global_mutex);
    
    steam_ugc_queries.clear();
}
