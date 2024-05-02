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

#ifndef __INCLUDED_STEAM_INVENTORY_H__
#define __INCLUDED_STEAM_INVENTORY_H__

#include "base.h" // For SteamItemDef_t

struct Steam_Inventory_Requests {
    double timeout = 0.1;
    bool done = false;
    bool full_query{};

    SteamInventoryResult_t inventory_result{};
    std::chrono::system_clock::time_point time_created{};

    std::vector<SteamItemInstanceID_t> instance_ids{};

    bool result_done() const;

    // in seconds
    uint32 timestamp() const;
};

class Steam_Inventory :
    public ISteamInventory001,
    public ISteamInventory002,
    public ISteamInventory
{
public:
    static constexpr const char items_user_file[]    = "items.json";
    static constexpr const char items_default_file[] = "default_items.json";

private:
    class Settings *settings{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    class Local_Storage* local_storage{};

    nlohmann::json defined_items{};
    nlohmann::json user_items{};

    std::vector<struct Steam_Inventory_Requests> inventory_requests{};

    bool inventory_loaded{};
    bool call_definition_update{};
    bool item_definitions_loaded{};

    struct Steam_Inventory_Requests* new_inventory_result(bool full_query=true, const SteamItemInstanceID_t* pInstanceIDs = NULL, uint32 unCountInstanceIDs = 0);
    struct Steam_Inventory_Requests *get_inventory_result(SteamInventoryResult_t resultHandle);

    void read_items_db();
    void read_inventory_db();

    static void run_every_runcb_cb(void *object);

public:
    Steam_Inventory(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, class Local_Storage *local_storage);
    ~Steam_Inventory();

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
    EResult GetResultStatus( SteamInventoryResult_t resultHandle );


    // Copies the contents of a result set into a flat array. The specific
    // contents of the result set depend on which query which was used.
    STEAM_METHOD_DESC(Copies the contents of a result set into a flat array. The specific contents of the result set depend on which query which was used.)
    bool GetResultItems( SteamInventoryResult_t resultHandle,
                                STEAM_OUT_ARRAY_COUNT( punOutItemsArraySize,Output array) SteamItemDetails_t *pOutItemsArray,
                                uint32 *punOutItemsArraySize );


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
    bool GetResultItemProperty( SteamInventoryResult_t resultHandle, 
                                        uint32 unItemIndex, 
                                        const char *pchPropertyName,
                                        STEAM_OUT_STRING_COUNT( punValueBufferSizeOut ) char *pchValueBuffer, uint32 *punValueBufferSizeOut );


    // Returns the server time at which the result was generated. Compare against
    // the value of IClientUtils::GetServerRealTime() to determine age.
    STEAM_METHOD_DESC(Returns the server time at which the result was generated. Compare against the value of IClientUtils::GetServerRealTime() to determine age.)
    uint32 GetResultTimestamp( SteamInventoryResult_t resultHandle );


    // Returns true if the result belongs to the target steam ID, false if the
    // result does not. This is important when using DeserializeResult, to verify
    // that a remote player is not pretending to have a different user's inventory.
    STEAM_METHOD_DESC(Returns true if the result belongs to the target steam ID or false if the result does not. This is important when using DeserializeResult to verify that a remote player is not pretending to have a different users inventory.)
    bool CheckResultSteamID( SteamInventoryResult_t resultHandle, CSteamID steamIDExpected );


    // Destroys a result handle and frees all associated memory.
    STEAM_METHOD_DESC(Destroys a result handle and frees all associated memory.)
    void DestroyResult( SteamInventoryResult_t resultHandle );



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
    bool GetAllItems( SteamInventoryResult_t *pResultHandle );



    // Captures the state of a subset of the current user's Steam inventory,
    // identified by an array of item instance IDs. The results from this call
    // can be serialized and passed to other players to "prove" that the current
    // user owns specific items, without exposing the user's entire inventory.
    // For example, you could call GetItemsByID with the IDs of the user's
    // currently equipped cosmetic items and serialize this to a buffer, and
    // then transmit this buffer to other players upon joining a game.
    STEAM_METHOD_DESC(Captures the state of a subset of the current users Steam inventory identified by an array of item instance IDs.)
    bool GetItemsByID( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT( unCountInstanceIDs ) const SteamItemInstanceID_t *pInstanceIDs, uint32 unCountInstanceIDs );



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
    bool SerializeResult( SteamInventoryResult_t resultHandle, STEAM_OUT_BUFFER_COUNT(punOutBufferSize) void *pOutBuffer, uint32 *punOutBufferSize );


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
    bool DeserializeResult( SteamInventoryResult_t *pOutResultHandle, STEAM_BUFFER_COUNT(punOutBufferSize) const void *pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE);



    // INVENTORY ASYNC MODIFICATION
    //

    // GenerateItems() creates one or more items and then generates a SteamInventoryCallback_t
    // notification with a matching nCallbackContext parameter. This API is only intended
    // for prototyping - it is only usable by Steam accounts that belong to the publisher group 
    // for your game.
    // If punArrayQuantity is not NULL, it should be the same length as pArrayItems and should
    // describe the quantity of each item to generate.
    bool GenerateItems( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, STEAM_ARRAY_COUNT(unArrayLength) const uint32 *punArrayQuantity, uint32 unArrayLength );


    // GrantPromoItems() checks the list of promotional items for which the user may be eligible
    // and grants the items (one time only).  On success, the result set will include items which
    // were granted, if any. If no items were granted because the user isn't eligible for any
    // promotions, this is still considered a success.
    STEAM_METHOD_DESC(GrantPromoItems() checks the list of promotional items for which the user may be eligible and grants the items (one time only).)
    bool GrantPromoItems( SteamInventoryResult_t *pResultHandle );


    // AddPromoItem() / AddPromoItems() are restricted versions of GrantPromoItems(). Instead of
    // scanning for all eligible promotional items, the check is restricted to a single item
    // definition or set of item definitions. This can be useful if your game has custom UI for
    // showing a specific promo item to the user.
    bool AddPromoItem( SteamInventoryResult_t *pResultHandle, SteamItemDef_t itemDef );

    bool AddPromoItems( SteamInventoryResult_t *pResultHandle, STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, uint32 unArrayLength );


    // ConsumeItem() removes items from the inventory, permanently. They cannot be recovered.
    // Not for the faint of heart - if your game implements item removal at all, a high-friction
    // UI confirmation process is highly recommended.
    STEAM_METHOD_DESC(ConsumeItem() removes items from the inventory permanently.)
    bool ConsumeItem( SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity );


    // ExchangeItems() is an atomic combination of item generation and consumption. 
    // It can be used to implement crafting recipes or transmutations, or items which unpack 
    // themselves into other items (e.g., a chest). 
    // Exchange recipes are defined in the ItemDef, and explicitly list the required item 
    // types and resulting generated type. 
    // Exchange recipes are evaluated atomically by the Inventory Service; if the supplied
    // components do not match the recipe, or do not contain sufficient quantity, the 
    // exchange will fail.
    bool ExchangeItems( SteamInventoryResult_t *pResultHandle,
                                STEAM_ARRAY_COUNT(unArrayGenerateLength) const SteamItemDef_t *pArrayGenerate, STEAM_ARRAY_COUNT(unArrayGenerateLength) const uint32 *punArrayGenerateQuantity, uint32 unArrayGenerateLength,
                                STEAM_ARRAY_COUNT(unArrayDestroyLength) const SteamItemInstanceID_t *pArrayDestroy, STEAM_ARRAY_COUNT(unArrayDestroyLength) const uint32 *punArrayDestroyQuantity, uint32 unArrayDestroyLength );



    // TransferItemQuantity() is intended for use with items which are "stackable" (can have
    // quantity greater than one). It can be used to split a stack into two, or to transfer
    // quantity from one stack into another stack of identical items. To split one stack into
    // two, pass k_SteamItemInstanceIDInvalid for itemIdDest and a new item will be generated.
    bool TransferItemQuantity( SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest );



    // TIMED DROPS AND PLAYTIME CREDIT
    //

    // Deprecated. Calling this method is not required for proper playtime accounting.
    STEAM_METHOD_DESC( Deprecated method. Playtime accounting is performed on the Steam servers. )
    void SendItemDropHeartbeat();


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
    bool TriggerItemDrop( SteamInventoryResult_t *pResultHandle, SteamItemDef_t dropListDefinition );



    // Deprecated. This method is not supported.
    bool TradeItems( SteamInventoryResult_t *pResultHandle, CSteamID steamIDTradePartner,
                                STEAM_ARRAY_COUNT(nArrayGiveLength) const SteamItemInstanceID_t *pArrayGive, STEAM_ARRAY_COUNT(nArrayGiveLength) const uint32 *pArrayGiveQuantity, uint32 nArrayGiveLength,
                                STEAM_ARRAY_COUNT(nArrayGetLength) const SteamItemInstanceID_t *pArrayGet, STEAM_ARRAY_COUNT(nArrayGetLength) const uint32 *pArrayGetQuantity, uint32 nArrayGetLength );



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
    bool LoadItemDefinitions();


    // GetItemDefinitionIDs returns the set of all defined item definition IDs (which are
    // defined via Steamworks configuration, and not necessarily contiguous integers).
    // If pItemDefIDs is null, the call will return true and *punItemDefIDsArraySize will
    // contain the total size necessary for a subsequent call. Otherwise, the call will
    // return false if and only if there is not enough space in the output array.
    bool GetItemDefinitionIDs(
                STEAM_OUT_ARRAY_COUNT(punItemDefIDsArraySize,List of item definition IDs) SteamItemDef_t *pItemDefIDs,
                STEAM_DESC(Size of array is passed in and actual size used is returned in this param) uint32 *punItemDefIDsArraySize );


    // GetItemDefinitionProperty returns a string property from a given item definition.
    // Note that some properties (for example, "name") may be localized and will depend
    // on the current Steam language settings (see ISteamApps::GetCurrentGameLanguage).
    // Property names are always composed of ASCII letters, numbers, and/or underscores.
    // Pass a NULL pointer for pchPropertyName to get a comma - separated list of available
    // property names. If pchValueBuffer is NULL, *punValueBufferSize will contain the 
    // suggested buffer size. Otherwise it will be the number of bytes actually copied
    // to pchValueBuffer. If the results do not fit in the given buffer, partial 
    // results may be copied.
    bool GetItemDefinitionProperty( SteamItemDef_t iDefinition, const char *pchPropertyName,
        STEAM_OUT_STRING_COUNT(punValueBufferSizeOut) char *pchValueBuffer, uint32 *punValueBufferSizeOut );


    // Request the list of "eligible" promo items that can be manually granted to the given
    // user.  These are promo items of type "manual" that won't be granted automatically.
    // An example usage of this is an item that becomes available every week.
    STEAM_CALL_RESULT( SteamInventoryEligiblePromoItemDefIDs_t )
    SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs( CSteamID steamID );


    // After handling a SteamInventoryEligiblePromoItemDefIDs_t call result, use this
    // function to pull out the list of item definition ids that the user can be
    // manually granted via the AddPromoItems() call.
    bool GetEligiblePromoItemDefinitionIDs(
        CSteamID steamID,
        STEAM_OUT_ARRAY_COUNT(punItemDefIDsArraySize,List of item definition IDs) SteamItemDef_t *pItemDefIDs,
        STEAM_DESC(Size of array is passed in and actual size used is returned in this param) uint32 *punItemDefIDsArraySize );


    // Starts the purchase process for the given item definitions.  The callback SteamInventoryStartPurchaseResult_t
    // will be posted if Steam was able to initialize the transaction.
    // 
    // Once the purchase has been authorized and completed by the user, the callback SteamInventoryResultReady_t 
    // will be posted.
    STEAM_CALL_RESULT( SteamInventoryStartPurchaseResult_t )
    SteamAPICall_t StartPurchase( STEAM_ARRAY_COUNT(unArrayLength) const SteamItemDef_t *pArrayItemDefs, STEAM_ARRAY_COUNT(unArrayLength) const uint32 *punArrayQuantity, uint32 unArrayLength );


    // Request current prices for all applicable item definitions
    STEAM_CALL_RESULT( SteamInventoryRequestPricesResult_t )
    SteamAPICall_t RequestPrices();


    // Returns the number of items with prices.  Need to call RequestPrices() first.
    uint32 GetNumItemsWithPrices();

    bool GetItemsWithPrices( STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pArrayItemDefs, Items with prices) SteamItemDef_t *pArrayItemDefs,
                                        STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pCurrentPrices,
                                        STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pBasePrices,
                                        uint32 unArrayLength );

    // Returns item definition ids and their prices in the user's local currency.
    // Need to call RequestPrices() first.
    bool GetItemsWithPrices( STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pArrayItemDefs, Items with prices) SteamItemDef_t *pArrayItemDefs,
                                        STEAM_ARRAY_COUNT(unArrayLength) STEAM_OUT_ARRAY_COUNT(pPrices, List of prices for the given item defs) uint64 *pPrices,
                                        uint32 unArrayLength );

    bool GetItemPrice( SteamItemDef_t iDefinition, uint64 *pCurrentPrice, uint64 *pBasePrice );

    // Retrieves the price for the item definition id
    // Returns false if there is no price stored for the item definition.
    bool GetItemPrice( SteamItemDef_t iDefinition, uint64 *pPrice );


    // Create a request to update properties on items
    SteamInventoryUpdateHandle_t StartUpdateProperties();

    // Remove the property on the item
    bool RemoveProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName );

    // Accessor methods to set properties on items
    bool SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, const char *pchPropertyValue );

    bool SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, bool bValue );

    bool SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, int64 nValue );

    bool SetProperty( SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char *pchPropertyName, float flValue );

    // Submit the update request by handle
    bool SubmitUpdateProperties( SteamInventoryUpdateHandle_t handle, SteamInventoryResult_t * pResultHandle );

    bool InspectItem( SteamInventoryResult_t *pResultHandle, const char *pchItemToken );

    void RunCallbacks();

};

#endif // __INCLUDED_STEAM_INVENTORY_H__
