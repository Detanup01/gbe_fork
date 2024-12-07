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

#include "dll/steam_inventory.h"


bool Steam_Inventory_Requests::result_done() const
{
    return done;
}

uint32 Steam_Inventory_Requests::timestamp() const
{
    return std::chrono::duration_cast<std::chrono::duration<uint32>>(time_created.time_since_epoch()).count();
}



struct Steam_Inventory_Requests* Steam_Inventory::new_inventory_result(bool full_query, const SteamItemInstanceID_t* pInstanceIDs, uint32 unCountInstanceIDs)
{
    static SteamInventoryResult_t result;
    ++result;

    struct Steam_Inventory_Requests request;
    request.inventory_result = result;
    request.full_query = full_query;
    if (pInstanceIDs && unCountInstanceIDs) {
        request.instance_ids.reserve(unCountInstanceIDs);
        std::copy(pInstanceIDs, pInstanceIDs + unCountInstanceIDs, std::back_inserter(request.instance_ids));
    }

    request.time_created = std::chrono::system_clock::now();
    inventory_requests.push_back(request);

    return &(inventory_requests.back());
}

struct Steam_Inventory_Requests* Steam_Inventory::get_inventory_result(SteamInventoryResult_t resultHandle)
{
    auto request = std::find_if(inventory_requests.begin(), inventory_requests.end(), [&resultHandle](struct Steam_Inventory_Requests const& item) { return item.inventory_result == resultHandle; });
    if (inventory_requests.end() == request)
        return NULL;

    return &(*request);
}

void Steam_Inventory::read_items_db()
{
    std::string items_db_path = Local_Storage::get_game_settings_path() + items_user_file;
    PRINT_DEBUG("file path: %s", items_db_path.c_str());
    local_storage->load_json(items_db_path, defined_items);
}

void Steam_Inventory::read_inventory_db()
{
    // If we havn't got any inventory
    if (!local_storage->load_json_file("", items_user_file, user_items))
    {
        // Try to load a default one
        std::string items_db_path = Local_Storage::get_game_settings_path() + items_default_file;
        PRINT_DEBUG("items file path: %s", items_db_path.c_str());
        local_storage->load_json(items_db_path, user_items);
    }
}


void Steam_Inventory::run_every_runcb_cb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Inventory *obj = (Steam_Inventory *)object;
    obj->RunCallbacks();
}


Steam_Inventory::Steam_Inventory(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, class Local_Storage *local_storage):
    settings(settings),
    callback_results(callback_results),
    callbacks(callbacks),
    run_every_runcb(run_every_runcb),
    local_storage(local_storage),

    defined_items(nlohmann::json::object()),
    user_items(nlohmann::json::object()),

    inventory_loaded(false),
    call_definition_update(false),
    item_definitions_loaded(false)
{
    this->run_every_runcb->add(&Steam_Inventory::run_every_runcb_cb, this);
}

Steam_Inventory::~Steam_Inventory()
{
    this->run_every_runcb->remove(&Steam_Inventory::run_every_runcb_cb, this);
}


// INVENTORY ASYNC RESULT MANAGEMENT
//
// Asynchronous inventory queries always output a result handle which can be used with
// GetResultStatus, GetResultItems, etc. A SteamInventoryResultReady_t callback will
// be triggered when the asynchronous result becomes ready (or fails).
//

// Find out the status of an asynchronous inventory result handle. Possible values:
//  k_EResultPending - still in progress
//  k_EResultOK - done, result ready
//  k_EResultExpired - done, result ready, maybe out of date (see DeserializeResult)
//  k_EResultInvalidParam - ERROR: invalid API call parameters
//  k_EResultServiceUnavailable - ERROR: service temporarily down, you may retry later
//  k_EResultLimitExceeded - ERROR: operation would exceed per-user inventory limits
//  k_EResultFail - ERROR: unknown / generic error
STEAM_METHOD_DESC(Find out the status of an asynchronous inventory result handle.)
EResult Steam_Inventory::GetResultStatus( SteamInventoryResult_t resultHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests *request = get_inventory_result(resultHandle);
    if (!request) return k_EResultInvalidParam;
    if (!request->result_done()) return k_EResultPending;
    return k_EResultOK;
}


// Copies the contents of a result set into a flat array. The specific
// contents of the result set depend on which query which was used.
STEAM_METHOD_DESC(Copies the contents of a result set into a flat array. The specific contents of the result set depend on which query which was used.)
bool Steam_Inventory::GetResultItems( SteamInventoryResult_t resultHandle,
                            STEAM_OUT_ARRAY_COUNT( punOutItemsArraySize,Output array) SteamItemDetails_t *pOutItemsArray,
                            uint32 *punOutItemsArraySize )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests *request = get_inventory_result(resultHandle);
    if (!request) return false;
    if (!request->result_done()) return false;
    if (!inventory_loaded) return false;

    if (pOutItemsArray != nullptr)
    {
        SteamItemDetails_t *items_array_base = pOutItemsArray;
        uint32 max_items = *punOutItemsArraySize;

        if (request->full_query) {
            // We end if we reached the end of items or the end of buffer
            for( auto i = user_items.begin(); i != user_items.end() && max_items; ++i, --max_items )
            {
                pOutItemsArray->m_iDefinition = std::stoi(i.key());
                pOutItemsArray->m_itemId = pOutItemsArray->m_iDefinition;
                try
                {
                    pOutItemsArray->m_unQuantity = i->value("quantity", static_cast<uint16>(0));
                }
                catch (...)
                {
                    pOutItemsArray->m_unQuantity = 0;
                }
                pOutItemsArray->m_unFlags = k_ESteamItemNoTrade;
                ++pOutItemsArray;
            }
        } else {
            for (auto &itemid : request->instance_ids) {
                if (!max_items) break;
                auto it = user_items.find(std::to_string(itemid));
                if (it != user_items.end()) {
                    pOutItemsArray->m_iDefinition = static_cast<int32>(itemid);
                    pOutItemsArray->m_itemId = itemid;

                    try
                    {
                        pOutItemsArray->m_unQuantity = it->value("quantity", static_cast<uint16>(0));
                    }
                    catch (...)
                    {
                        pOutItemsArray->m_unQuantity = 0;
                    }
                    pOutItemsArray->m_unFlags = k_ESteamItemNoTrade;
                    ++pOutItemsArray;
                    --max_items;
                }
            }
        }

        *punOutItemsArraySize = static_cast<uint32>(pOutItemsArray - items_array_base);
    }
    else if (punOutItemsArraySize != nullptr)
    {
        if (request->full_query) {
            *punOutItemsArraySize = static_cast<uint32>(user_items.size());
        } else {
            *punOutItemsArraySize = static_cast<uint32>(std::count_if(request->instance_ids.begin(), request->instance_ids.end(), [this](SteamItemInstanceID_t item_id){ return user_items.find(std::to_string(item_id)) != user_items.end();}));
        }
    }

    PRINT_DEBUG("good");
    return true;
}


// In combination with GetResultItems, you can use GetResultItemProperty to retrieve
// dynamic string properties for a given item returned in the result set.
// 
// Property names are always composed of ASCII letters, numbers, and/or underscores.
//
// Pass a NULL pointer for pchPropertyName to get a comma - separated list of available
// property names.
//
// If pchValueBuffer is NULL, *punValueBufferSize will contain the 
// suggested buffer size. Otherwise it will be the number of bytes actually copied
// to pchValueBuffer. If the results do not fit in the given buffer, partial 
// results may be copied.
bool Steam_Inventory::GetResultItemProperty( SteamInventoryResult_t resultHandle, 
                                    uint32 unItemIndex, 
                                    const char *pchPropertyName,
                                    STEAM_OUT_STRING_COUNT( punValueBufferSizeOut ) char *pchValueBuffer, uint32 *punValueBufferSizeOut )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    return false;
}


// Returns the server time at which the result was generated. Compare against
// the value of IClientUtils::GetServerRealTime() to determine age.
STEAM_METHOD_DESC(Returns Steam_Inventory::the server time at which the result was generated. Compare against the value of IClientUtils::GetServerRealTime() to determine age.)
uint32 Steam_Inventory::GetResultTimestamp( SteamInventoryResult_t resultHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests *request = get_inventory_result(resultHandle);
    if (!request || !request->result_done()) return 0;
    return request->timestamp();
}


// Returns true if the result belongs to the target steam ID, false if the
// result does not. This is important when using DeserializeResult, to verify
// that a remote player is not pretending to have a different user's inventory.
STEAM_METHOD_DESC(Returns true if the result belongs to the target steam ID or false if the result does not. This is important when using DeserializeResult to verify that a remote player is not pretending to have a different users inventory.)
bool Steam_Inventory::CheckResultSteamID( SteamInventoryResult_t resultHandle, CSteamID steamIDExpected )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    return true;
}


// Destroys a result handle and frees all associated memory.
STEAM_METHOD_DESC(Destroys a result handle and frees all associated memory.)
void Steam_Inventory::DestroyResult( SteamInventoryResult_t resultHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(inventory_requests.begin(), inventory_requests.end(), [&resultHandle](struct Steam_Inventory_Requests const& item) { return item.inventory_result == resultHandle; });
    if (inventory_requests.end() == request)
        return;

    inventory_requests.erase(request);
}



// INVENTORY ASYNC QUERY
//

// Captures the entire state of the current user's Steam inventory.
// You must call DestroyResult on this handle when you are done with it.
// Returns false and sets *pResultHandle to zero if inventory is unavailable.
// Note: calls to this function are subject to rate limits and may return
// cached results if called too frequently. It is suggested that you call
// this function only when you are about to display the user's full inventory,
// or if you expect that the inventory may have changed.
STEAM_METHOD_DESC(Captures the entire state of the current users Steam inventory.)
bool Steam_Inventory::GetAllItems( SteamInventoryResult_t *pResultHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests* request = new_inventory_result();

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;

    return true;
}



// Captures the state of a subset of the current user's Steam inventory,
// identified by an array of item instance IDs. The results from this call
// can be serialized and passed to other players to "prove" that the current
// user owns specific items, without exposing the user's entire inventory.
// For example, you could call GetItemsByID with the IDs of the user's
// currently equipped cosmetic items and serialize this to a buffer, and
// then transmit this buffer to other players upon joining a game.
STEAM_METHOD_DESC(Captures the state of a subset of the current users Steam inventory identified by an array of item instance IDs.)
bool Steam_Inventory::GetItemsByID( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT( unCountInstanceIDs ) const SteamItemInstanceID_t *pInstanceIDs, uint32 unCountInstanceIDs )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pResultHandle) {
        struct Steam_Inventory_Requests *request = new_inventory_result(false, pInstanceIDs, unCountInstanceIDs);
        *pResultHandle = request->inventory_result;
        return true;
    }

    return false;
}



// RESULT SERIALIZATION AND AUTHENTICATION
//
// Serialized result sets contain a short signature which can't be forged
// or replayed across different game sessions. A result set can be serialized
// on the local client, transmitted to other players via your game networking,
// and deserialized by the remote players. This is a secure way of preventing
// hackers from lying about posessing rare/high-value items.

// Serializes a result set with signature bytes to an output buffer. Pass
// NULL as an output buffer to get the required size via punOutBufferSize.
// The size of a serialized result depends on the number items which are being
// serialized. When securely transmitting items to other players, it is
// recommended to use "GetItemsByID" first to create a minimal result set.
// Results have a built-in timestamp which will be considered "expired" after
// an hour has elapsed. See DeserializeResult for expiration handling.
bool Steam_Inventory::SerializeResult( SteamInventoryResult_t resultHandle, STEAM_OUT_BUFFER_COUNT(punOutBufferSize) void *pOutBuffer, uint32 *punOutBufferSize )
{
    PRINT_DEBUG("%i", resultHandle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    struct Steam_Inventory_Requests *request = get_inventory_result(resultHandle);
    if (!request) return false;
    if (!request->result_done()) return false;

    uint8 buffer[8 + 128] = {};
    memset(buffer, 0x5F, sizeof(buffer));

    if (!punOutBufferSize) return false;
    PRINT_DEBUG("  Size %u", *punOutBufferSize);
    if (!pOutBuffer) {
        *punOutBufferSize = sizeof(buffer);
        return true;
    }

    if (*punOutBufferSize < sizeof(buffer)) {
        *punOutBufferSize = sizeof(buffer);
        return false; //??
    }

    memcpy(pOutBuffer, buffer, sizeof(buffer));
    *punOutBufferSize = sizeof(buffer);
    return true;
}


// Deserializes a result set and verifies the signature bytes. Returns false
// if bRequireFullOnlineVerify is set but Steam is running in Offline mode.
// Otherwise returns true and then delivers error codes via GetResultStatus.
//
// The bRESERVED_MUST_BE_FALSE flag is reserved for future use and should not
// be set to true by your game at this time.
//
// DeserializeResult has a potential soft-failure mode where the handle status
// is set to k_EResultExpired. GetResultItems() still succeeds in this mode.
// The "expired" result could indicate that the data may be out of date - not
// just due to timed expiration (one hour), but also because one of the items
// in the result set may have been traded or consumed since the result set was
// generated. You could compare the timestamp from GetResultTimestamp() to
// ISteamUtils::GetServerRealTime() to determine how old the data is. You could
// simply ignore the "expired" result code and continue as normal, or you
// could challenge the player with expired data to send an updated result set.
bool Steam_Inventory::DeserializeResult( SteamInventoryResult_t *pOutResultHandle, STEAM_BUFFER_COUNT(punOutBufferSize) const void *pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE)
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    if (pOutResultHandle) {
        struct Steam_Inventory_Requests *request = new_inventory_result(false);
        *pOutResultHandle = request->inventory_result;
        return true;
    }

    return false;
}



// INVENTORY ASYNC MODIFICATION
//

// GenerateItems() creates one or more items and then generates a SteamInventoryCallback_t
// notification with a matching nCallbackContext parameter. This API is only intended
// for prototyping - it is only usable by Steam accounts that belong to the publisher group 
// for your game.
// If punArrayQuantity is not NULL, it should be the same length as pArrayItems and should
// describe the quantity of each item to generate.
bool Steam_Inventory::GenerateItems( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, STEAM_ARRAY_COUNT(unArrayLength) const uint32 *punArrayQuantity, uint32 unArrayLength )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// GrantPromoItems() checks the list of promotional items for which the user may be eligible
// and grants the items (one time only).  On success, the result set will include items which
// were granted, if any. If no items were granted because the user isn't eligible for any
// promotions, this is still considered a success.
STEAM_METHOD_DESC(GrantPromoItems() Steam_Inventory::checks the list of promotional items for which the user may be eligible and grants the items (one time only).)
bool Steam_Inventory::GrantPromoItems( SteamInventoryResult_t *pResultHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests* request = new_inventory_result(false);

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;
    return true;
}


// AddPromoItem() / AddPromoItems() are restricted versions of GrantPromoItems(). Instead of
// scanning for all eligible promotional items, the check is restricted to a single item
// definition or set of item definitions. This can be useful if your game has custom UI for
// showing a specific promo item to the user.
bool Steam_Inventory::AddPromoItem( SteamInventoryResult_t *pResultHandle, SteamItemDef_t itemDef )
{
    PRINT_DEBUG_ENTRY();
    //TODO
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests* request = new_inventory_result(false);

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;
    return true;
}

bool Steam_Inventory::AddPromoItems( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, uint32 unArrayLength )
{
    PRINT_DEBUG_ENTRY();
    //TODO
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests* request = new_inventory_result(false);

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;
    return true;
}


// ConsumeItem() removes items from the inventory, permanently. They cannot be recovered.
// Not for the faint of heart - if your game implements item removal at all, a high-friction
// UI confirmation process is highly recommended.
STEAM_METHOD_DESC(ConsumeItem() removes items from the inventory permanently.)
bool Steam_Inventory::ConsumeItem( SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity )
{
    PRINT_DEBUG("%llu %u", itemConsume, unQuantity);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto it = user_items.find(std::to_string(itemConsume));
    if (it != user_items.end()) {
        try
        {
            uint32 current = it->get<int>();
            PRINT_DEBUG("previous %u", current);
            if (current < unQuantity) unQuantity = current;
            uint32 result = current - unQuantity;
            if (result == 0) {
                user_items.erase(it);
            } else {
                *it = result;
            }
        }
        catch (...) {}

    } else {
        return false;
    }

    struct Steam_Inventory_Requests* request = new_inventory_result(false, &itemConsume, 1);

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;

    return true;
}


// ExchangeItems() is an atomic combination of item generation and consumption. 
// It can be used to implement crafting recipes or transmutations, or items which unpack 
// themselves into other items (e.g., a chest). 
// Exchange recipes are defined in the ItemDef, and explicitly list the required item 
// types and resulting generated type. 
// Exchange recipes are evaluated atomically by the Inventory Service; if the supplied
// components do not match the recipe, or do not contain sufficient quantity, the 
// exchange will fail.
bool Steam_Inventory::ExchangeItems( SteamInventoryResult_t *pResultHandle,
                            STEAM_ARRAY_COUNT(unArrayGenerateLength) const SteamItemDef_t *pArrayGenerate, STEAM_ARRAY_COUNT(unArrayGenerateLength) const uint32 *punArrayGenerateQuantity, uint32 unArrayGenerateLength,
                            STEAM_ARRAY_COUNT(unArrayDestroyLength) const SteamItemInstanceID_t *pArrayDestroy, STEAM_ARRAY_COUNT(unArrayDestroyLength) const uint32 *punArrayDestroyQuantity, uint32 unArrayDestroyLength )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}



// TransferItemQuantity() is intended for use with items which are "stackable" (can have
// quantity greater than one). It can be used to split a stack into two, or to transfer
// quantity from one stack into another stack of identical items. To split one stack into
// two, pass k_SteamItemInstanceIDInvalid for itemIdDest and a new item will be generated.
bool Steam_Inventory::TransferItemQuantity( SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}



// TIMED DROPS AND PLAYTIME CREDIT
//

// Deprecated. Calling this method is not required for proper playtime accounting.
STEAM_METHOD_DESC( Deprecated method. Playtime accounting is performed on the Steam servers. )
void Steam_Inventory::SendItemDropHeartbeat()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Playtime credit must be consumed and turned into item drops by your game. Only item
// definitions which are marked as "playtime item generators" can be spawned. The call
// will return an empty result set if there is not enough playtime credit for a drop.
// Your game should call TriggerItemDrop at an appropriate time for the user to receive
// new items, such as between rounds or while the player is dead. Note that players who
// hack their clients could modify the value of "dropListDefinition", so do not use it
// to directly control rarity.
// See your Steamworks configuration to set playtime drop rates for individual itemdefs.
// The client library will suppress too-frequent calls to this method.
STEAM_METHOD_DESC(Playtime credit must be consumed and turned into item drops by your game.)
bool Steam_Inventory::TriggerItemDrop( SteamInventoryResult_t *pResultHandle, SteamItemDef_t dropListDefinition )
{
    PRINT_DEBUG("%p %i", pResultHandle, dropListDefinition);
    //TODO: if gameserver return false
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Steam_Inventory_Requests* request = new_inventory_result(false);

    if (pResultHandle != nullptr)
        *pResultHandle = request->inventory_result;
    return true;
}



// Deprecated. This method is not supported.
bool Steam_Inventory::TradeItems( SteamInventoryResult_t *pResultHandle, CSteamID steamIDTradePartner,
                            STEAM_ARRAY_COUNT(nArrayGiveLength) const SteamItemInstanceID_t *pArrayGive, STEAM_ARRAY_COUNT(nArrayGiveLength) const uint32 *pArrayGiveQuantity, uint32 nArrayGiveLength,
                            STEAM_ARRAY_COUNT(nArrayGetLength) const SteamItemInstanceID_t *pArrayGet, STEAM_ARRAY_COUNT(nArrayGetLength) const uint32 *pArrayGetQuantity, uint32 nArrayGetLength )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}



// ITEM DEFINITIONS
//
// Item definitions are a mapping of "definition IDs" (integers between 1 and 1000000)
// to a set of string properties. Some of these properties are required to display items
// on the Steam community web site. Other properties can be defined by applications.
// Use of these functions is optional; there is no reason to call LoadItemDefinitions
// if your game hardcodes the numeric definition IDs (eg, purple face mask = 20, blue
// weapon mod = 55) and does not allow for adding new item types without a client patch.
//

// LoadItemDefinitions triggers the automatic load and refresh of item definitions.
// Every time new item definitions are available (eg, from the dynamic addition of new
// item types while players are still in-game), a SteamInventoryDefinitionUpdate_t
// callback will be fired.
STEAM_METHOD_DESC(LoadItemDefinitions triggers the automatic load and refresh of item definitions.)
bool Steam_Inventory::LoadItemDefinitions()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!item_definitions_loaded)  {
        call_definition_update = true;
    }

    //real steam launches a SteamInventoryResultReady_t which is why I create a new inventory result
    new_inventory_result(false);
    return true;
}


// GetItemDefinitionIDs returns the set of all defined item definition IDs (which are
// defined via Steamworks configuration, and not necessarily contiguous integers).
// If pItemDefIDs is null, the call will return true and *punItemDefIDsArraySize will
// contain the total size necessary for a subsequent call. Otherwise, the call will
// return false if and only if there is not enough space in the output array.
bool Steam_Inventory::GetItemDefinitionIDs(
            STEAM_OUT_ARRAY_COUNT(punItemDefIDsArraySize,List of item definition IDs) SteamItemDef_t *pItemDefIDs,
            STEAM_DESC(Size of array is passed in and actual size used is returned in this param) uint32 *punItemDefIDsArraySize )
{
    PRINT_DEBUG("%p", pItemDefIDs);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!punItemDefIDsArraySize)
        return false;

    PRINT_DEBUG("  array_size %u", *punItemDefIDsArraySize);

    if (!item_definitions_loaded)
        return false;

    if (pItemDefIDs == nullptr || *punItemDefIDsArraySize == 0)
    {
        *punItemDefIDsArraySize = static_cast<uint32>(defined_items.size());
        return true;
    }

    if (*punItemDefIDsArraySize < static_cast<uint32>(defined_items.size()))
        return false;

    for (auto i = defined_items.begin(); i != defined_items.end(); ++i)
        *pItemDefIDs++ = std::stoi(i.key());

    return true;
}


// GetItemDefinitionProperty returns a string property from a given item definition.
// Note that some properties (for example, "name") may be localized and will depend
// on the current Steam language settings (see ISteamApps::GetCurrentGameLanguage).
// Property names are always composed of ASCII letters, numbers, and/or underscores.
// Pass a NULL pointer for pchPropertyName to get a comma - separated list of available
// property names. If pchValueBuffer is NULL, *punValueBufferSize will contain the 
// suggested buffer size. Otherwise it will be the number of bytes actually copied
// to pchValueBuffer. If the results do not fit in the given buffer, partial 
// results may be copied.
bool Steam_Inventory::GetItemDefinitionProperty( SteamItemDef_t iDefinition, const char *pchPropertyName,
    STEAM_OUT_STRING_COUNT(punValueBufferSizeOut) char *pchValueBuffer, uint32 *punValueBufferSizeOut )
{
    PRINT_DEBUG("%i %s", iDefinition, pchPropertyName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto item = defined_items.find(std::to_string(iDefinition));
    if (item != defined_items.end())
    {
        if (pchPropertyName != nullptr)
        {
            // Should I check for punValueBufferSizeOut == nullptr ?
            // Try to get the property
            auto attr = item.value().find(pchPropertyName);
            if (attr != item.value().end())
            {
                std::string val;
                try
                {
                    val = attr.value().get<std::string>();
                }
                catch (...)
                {
                    pchPropertyName = "";
                    *punValueBufferSizeOut = 0;
                    PRINT_DEBUG("  Error, item: %d, attr: %s is not a string!", iDefinition, pchPropertyName);
                    return true;
                }
                if (pchValueBuffer != nullptr)
                {
                    // copy what we can
                    strncpy(pchValueBuffer, val.c_str(), *punValueBufferSizeOut);
                    *punValueBufferSizeOut = std::min(static_cast<uint32>(val.length() + 1), *punValueBufferSizeOut);
                }
                else
                {
                    // Set punValueBufferSizeOut to the property size
                    *punValueBufferSizeOut = static_cast<uint32>(val.length()) + 1;
                }

                if (pchValueBuffer != nullptr)
                {
                    // Make sure we have a null terminator
                    pchValueBuffer[*punValueBufferSizeOut-1] = '\0';
                }
            }
            // Property not found
            else
            {
                *punValueBufferSizeOut = 0;
                PRINT_DEBUG("  Attr %s not found for item %d", pchPropertyName, iDefinition);
                return false;
            }
        }
        else // Pass a NULL pointer for pchPropertyName to get a comma - separated list of available property names.
        {
            // If pchValueBuffer is NULL, *punValueBufferSize will contain the suggested buffer size
            if (pchValueBuffer == nullptr)
            {
                // Should I check for punValueBufferSizeOut == nullptr ?
                *punValueBufferSizeOut = 0;
                for (auto i = item.value().begin(); i != item.value().end(); ++i)
                    *punValueBufferSizeOut += static_cast<uint32>(i.key().length()) + 1; // Size of key + comma, and the last is not a comma but null char
            }
            else
            {
                // strncat always add the null terminator, so remove 1 to the string length
                uint32_t len = *punValueBufferSizeOut-1;
                *punValueBufferSizeOut = 0;
                memset(pchValueBuffer, 0, len);
                for( auto i = item.value().begin(); i != item.value().end() && len > 0; ++i )
                {
                    strncat(pchValueBuffer, i.key().c_str(), len);
                    // Count how many chars we copied
                    // Either the string length or the buffer size if its too small
                    uint32 x = std::min(len, static_cast<uint32>(i.key().length()));
                    *punValueBufferSizeOut += x;
                    len -= x;

                    if (len && std::distance(i, item.value().end()) != 1) // If this is not the last item, add a comma
                        strncat(pchValueBuffer, ",", len--);

                    // Always add 1, its a comma or the null terminator
                    ++*punValueBufferSizeOut;
                }
            }
        }

        return true;
    }

    return false;
}


// Request the list of "eligible" promo items that can be manually granted to the given
// user.  These are promo items of type "manual" that won't be granted automatically.
// An example usage of this is an item that becomes available every week.
STEAM_CALL_RESULT( SteamInventoryEligiblePromoItemDefIDs_t )
SteamAPICall_t Steam_Inventory::RequestEligiblePromoItemDefinitionsIDs( CSteamID steamID )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}


// After handling a SteamInventoryEligiblePromoItemDefIDs_t call result, use this
// function to pull out the list of item definition ids that the user can be
// manually granted via the AddPromoItems() call.
bool Steam_Inventory::GetEligiblePromoItemDefinitionIDs(
    CSteamID steamID,
    STEAM_OUT_ARRAY_COUNT(punItemDefIDsArraySize,List of item definition IDs) SteamItemDef_t *pItemDefIDs,
    STEAM_DESC(Size of array is passed in and actual size used is returned in this param) uint32 *punItemDefIDsArraySize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Starts the purchase process for the given item definitions.  The callback SteamInventoryStartPurchaseResult_t
// will be posted if Steam was able to initialize the transaction.
// 
// Once the purchase has been authorized and completed by the user, the callback SteamInventoryResultReady_t 
// will be posted.
STEAM_CALL_RESULT( SteamInventoryStartPurchaseResult_t )
SteamAPICall_t Steam_Inventory::StartPurchase( STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, STEAM_ARRAY_COUNT(unArrayLength) const uint32 *punArrayQuantity, uint32 unArrayLength )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}


// Request current prices for all applicable item definitions
STEAM_CALL_RESULT( SteamInventoryRequestPricesResult_t )
SteamAPICall_t Steam_Inventory::RequestPrices()
{
    PRINT_DEBUG_ENTRY();
    SteamInventoryRequestPricesResult_t data{};
    data.m_result = k_EResultOK;
    memcpy(data.m_rgchCurrency, "USD", 4);
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.2);
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.2);
    return ret;
}


// Returns the number of items with prices.  Need to call RequestPrices() first.
uint32 Steam_Inventory::GetNumItemsWithPrices()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

bool Steam_Inventory::GetItemsWithPrices( STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pArrayItemDefs, Items with prices) SteamItemDef_t *pArrayItemDefs,
									 STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pCurrentPrices,
									 STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pBasePrices,
									 uint32 unArrayLength )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Returns item definition ids and their prices in the user's local currency.
// Need to call RequestPrices() first.
bool Steam_Inventory::GetItemsWithPrices( STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pArrayItemDefs, Items with prices) SteamItemDef_t *pArrayItemDefs,
                                    STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pPrices,
                                    uint32 unArrayLength )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return GetItemsWithPrices(pArrayItemDefs, pPrices, NULL, unArrayLength);
}

bool Steam_Inventory::GetItemPrice( SteamItemDef_t iDefinition, uint64 *pCurrentPrice, uint64 *pBasePrice )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Retrieves the price for the item definition id
// Returns false if there is no price stored for the item definition.
bool Steam_Inventory::GetItemPrice( SteamItemDef_t iDefinition, uint64 *pPrice )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return GetItemPrice(iDefinition, pPrice, NULL);
}


// Create a request to update properties on items
SteamInventoryUpdateHandle_t Steam_Inventory::StartUpdateProperties()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

// Remove the property on the item
bool Steam_Inventory::RemoveProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Accessor methods to set properties on items
bool Steam_Inventory::SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, const char *pchPropertyValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_Inventory::SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, bool bValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_Inventory::SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, int64 nValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_Inventory::SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, float flValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Submit the update request by handle
bool Steam_Inventory::SubmitUpdateProperties( SteamInventoryUpdateHandle_t handle, SteamInventoryResult_t * pResultHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_Inventory::InspectItem( SteamInventoryResult_t *pResultHandle, const char *pchItemToken )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


void Steam_Inventory::RunCallbacks()
{
    if (call_definition_update || !inventory_requests.empty()) {
        if (!item_definitions_loaded) {
            read_items_db();
            item_definitions_loaded = true;

            //only gets called once
            //also gets called when getting items
            SteamInventoryDefinitionUpdate_t data = {};
            callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.05);
        }

        call_definition_update = false;
    }

    if (!inventory_requests.empty() && !inventory_loaded) {
        read_inventory_db();
        inventory_loaded = true;
    }

    if (inventory_loaded)
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        for (auto& r : inventory_requests) {
            if (!r.done && std::chrono::duration_cast<std::chrono::duration<double>>(now - r.time_created).count() > r.timeout) {
                if (r.full_query) {
                    // SteamInventoryFullUpdate_t callbacks are triggered when GetAllItems
                    // successfully returns a result which is newer / fresher than the last
                    // known result.
                    struct SteamInventoryFullUpdate_t data;
                    data.m_handle = r.inventory_result;
                    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                }

                {
                    struct SteamInventoryResultReady_t data;
                    data.m_handle = r.inventory_result;
                    data.m_result = k_EResultOK;
                    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                }

                r.done = true;
            }
        }
    }
}
