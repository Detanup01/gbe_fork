
#ifndef ISTEAMINVENTORY001_H
#define ISTEAMINVENTORY001_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamInventory001
{
public:										
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
		virtual EResult GetResultStatus(SteamInventoryResult_t resultHandle) = 0;

	// Copies the contents of a result set into a flat array. The specific
	// contents of the result set depend on which query which was used.
		virtual bool GetResultItems(SteamInventoryResult_t resultHandle,
		SteamItemDetails_t *pOutItemsArray,
		uint32 *punOutItemsArraySize) = 0;

	// Returns the server time at which the result was generated. Compare against
	// the value of IClientUtils::GetServerRealTime() to determine age.
		virtual uint32 GetResultTimestamp(SteamInventoryResult_t resultHandle) = 0;

	// Returns true if the result belongs to the target steam ID, false if the
	// result does not. This is important when using DeserializeResult, to verify
	// that a remote player is not pretending to have a different user's inventory.
		virtual bool CheckResultSteamID(SteamInventoryResult_t resultHandle, CSteamID steamIDExpected) = 0;

	// Destroys a result handle and frees all associated memory.
		virtual void DestroyResult(SteamInventoryResult_t resultHandle) = 0;


	// INVENTORY ASYNC QUERY
	//

	// Captures the entire state of the current user's Steam inventory.
	// You must call DestroyResult on this handle when you are done with it.
	// Returns false and sets *pResultHandle to zero if inventory is unavailable.
	// Note: calls to this function are subject to rate limits and may return
	// cached results if called too frequently. It is suggested that you call
	// this function only when you are about to display the user's full inventory,
	// or if you expect that the inventory may have changed.
		virtual bool GetAllItems(SteamInventoryResult_t *pResultHandle) = 0;


	// Captures the state of a subset of the current user's Steam inventory,
	// identified by an array of item instance IDs. The results from this call
	// can be serialized and passed to other players to "prove" that the current
	// user owns specific items, without exposing the user's entire inventory.
	// For example, you could call GetItemsByID with the IDs of the user's
	// currently equipped cosmetic items and serialize this to a buffer, and
	// then transmit this buffer to other players upon joining a game.
		virtual bool GetItemsByID(SteamInventoryResult_t *pResultHandle, const SteamItemInstanceID_t *pInstanceIDs, uint32 unCountInstanceIDs) = 0;


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
	virtual bool SerializeResult(SteamInventoryResult_t resultHandle, void *pOutBuffer, uint32 *punOutBufferSize) = 0;

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
	virtual bool DeserializeResult(SteamInventoryResult_t *pOutResultHandle, const void *pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE = false) = 0;


	// INVENTORY ASYNC MODIFICATION
	//

	// GenerateItems() creates one or more items and then generates a SteamInventoryCallback_t
	// notification with a matching nCallbackContext parameter. This API is insecure, and could
	// be abused by hacked clients. It is, however, very useful as a development cheat or as
	// a means of prototyping item-related features for your game. The use of GenerateItems can
	// be restricted to certain item definitions or fully blocked via the Steamworks website.
	// If punArrayQuantity is not NULL, it should be the same length as pArrayItems and should
	// describe the quantity of each item to generate.
	virtual bool GenerateItems(SteamInventoryResult_t *pResultHandle, const SteamItemDef_t *pArrayItemDefs, const uint32 *punArrayQuantity, uint32 unArrayLength) = 0;

	// GrantPromoItems() checks the list of promotional items for which the user may be eligible
	// and grants the items (one time only).  On success, the result set will include items which
	// were granted, if any. If no items were granted because the user isn't eligible for any
	// promotions, this is still considered a success.
		virtual bool GrantPromoItems(SteamInventoryResult_t *pResultHandle) = 0;

	// AddPromoItem() / AddPromoItems() are restricted versions of GrantPromoItems(). Instead of
	// scanning for all eligible promotional items, the check is restricted to a single item
	// definition or set of item definitions. This can be useful if your game has custom UI for
	// showing a specific promo item to the user.
	virtual bool AddPromoItem(SteamInventoryResult_t *pResultHandle, SteamItemDef_t itemDef) = 0;
	virtual bool AddPromoItems(SteamInventoryResult_t *pResultHandle, const SteamItemDef_t *pArrayItemDefs, uint32 unArrayLength) = 0;

	// ConsumeItem() removes items from the inventory, permenantly. They cannot be recovered.
	// Not for the faint of heart - if your game implements item removal at all, a high-friction
	// UI confirmation process is highly recommended. Similar to GenerateItems, punArrayQuantity
	// can be NULL or else an array of the same length as pArrayItems which describe the quantity
	// of each item to destroy. ConsumeItem can be restricted to certain item definitions or
	// fully blocked via the Steamworks website to minimize support/abuse issues such as the
	// clasic "my brother borrowed my laptop and deleted all of my rare items".
		virtual bool ConsumeItem(SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity) = 0;

	// ExchangeItems() is an atomic combination of GenerateItems and DestroyItems. It can be
	// used to implement crafting recipes or transmutations, or items which unpack themselves
	// into other items. Like GenerateItems, this is a flexible and dangerous API which is
	// meant for rapid prototyping. You can configure restrictions on ExchangeItems via the
	// Steamworks website, such as limiting it to a whitelist of input/output combinations
	// corresponding to recipes.
	// (Note: although GenerateItems may be hard or impossible to use securely in your game,
	// ExchangeItems is perfectly reasonable to use once the whitelists are set accordingly.)
	virtual bool ExchangeItems(SteamInventoryResult_t *pResultHandle,
		const SteamItemDef_t *pArrayGenerate, const uint32 *punArrayGenerateQuantity, uint32 unArrayGenerateLength,
		const SteamItemInstanceID_t *pArrayDestroy, const uint32 *punArrayDestroyQuantity, uint32 unArrayDestroyLength) = 0;


	// TransferItemQuantity() is intended for use with items which are "stackable" (can have
	// quantity greater than one). It can be used to split a stack into two, or to transfer
	// quantity from one stack into another stack of identical items. To split one stack into
	// two, pass k_SteamItemInstanceIDInvalid for itemIdDest and a new item will be generated.
	virtual bool TransferItemQuantity(SteamInventoryResult_t *pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest) = 0;


	// TIMED DROPS AND PLAYTIME CREDIT
	//

	// Applications which use timed-drop mechanics should call SendItemDropHeartbeat() when
	// active gameplay begins, and at least once every two minutes afterwards. The backend
	// performs its own time calculations, so the precise timing of the heartbeat is not
	// critical as long as you send at least one heartbeat every two minutes. Calling the
	// function more often than that is not harmful, it will simply have no effect. Note:
	// players may be able to spoof this message by hacking their client, so you should not
	// attempt to use this as a mechanism to restrict playtime credits. It is simply meant
	// to distinguish between being in any kind of gameplay situation vs the main menu or
	// a pre-game launcher window. (If you are stingy with handing out playtime credit, it
	// will only encourage players to run bots or use mouse/kb event simulators.)
	//
	// Playtime credit accumulation can be capped on a daily or weekly basis through your
	// Steamworks configuration.
	//
		virtual void SendItemDropHeartbeat() = 0;

	// Playtime credit must be consumed and turned into item drops by your game. Only item
	// definitions which are marked as "playtime item generators" can be spawned. The call
	// will return an empty result set if there is not enough playtime credit for a drop.
	// Your game should call TriggerItemDrop at an appropriate time for the user to receive
	// new items, such as between rounds or while the player is dead. Note that players who
	// hack their clients could modify the value of "dropListDefinition", so do not use it
	// to directly control rarity. It is primarily useful during testing and development,
	// where you may wish to perform experiments with different types of drops.
		virtual bool TriggerItemDrop(SteamInventoryResult_t *pResultHandle, SteamItemDef_t dropListDefinition) = 0;


	// IN-GAME TRADING
	//
	// TradeItems() implements limited in-game trading of items, if you prefer not to use
	// the overlay or an in-game web browser to perform Steam Trading through the website.
	// You should implement a UI where both players can see and agree to a trade, and then
	// each client should call TradeItems simultaneously (+/- 5 seconds) with matching
	// (but reversed) parameters. The result is the same as if both players performed a
	// Steam Trading transaction through the web. Each player will get an inventory result
	// confirming the removal or quantity changes of the items given away, and the new
	// item instance id numbers and quantities of the received items.
	// (Note: new item instance IDs are generated whenever an item changes ownership.)
	virtual bool TradeItems(SteamInventoryResult_t *pResultHandle, CSteamID steamIDTradePartner,
		const SteamItemInstanceID_t *pArrayGive, const uint32 *pArrayGiveQuantity, uint32 nArrayGiveLength,
		const SteamItemInstanceID_t *pArrayGet, const uint32 *pArrayGetQuantity, uint32 nArrayGetLength) = 0;


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
		virtual bool LoadItemDefinitions() = 0;

	// GetItemDefinitionIDs returns the set of all defined item definition IDs (which are
	// defined via Steamworks configuration, and not necessarily contiguous integers).
	// If pItemDefIDs is null, the call will return true and *punItemDefIDsArraySize will
	// contain the total size necessary for a subsequent call. Otherwise, the call will
	// return false if and only if there is not enough space in the output array.
	virtual bool GetItemDefinitionIDs(
		SteamItemDef_t *pItemDefIDs,
		uint32 *punItemDefIDsArraySize) = 0;

	// GetItemDefinitionProperty returns a string property from a given item definition.
	// Note that some properties (for example, "name") may be localized and will depend
	// on the current Steam language settings (see ISteamApps::GetCurrentGameLanguage).
	// Property names are always composed of ASCII letters, numbers, and/or underscores.
	// Pass a NULL pointer for pchPropertyName to get a comma - separated list of available
	// property names. 
	virtual bool GetItemDefinitionProperty(SteamItemDef_t iDefinition, const char *pchPropertyName,
		char *pchValueBuffer, uint32 *punValueBufferSize) = 0;

	virtual SteamAPICall_t RequestEligiblePromoItemDefinitionsIDs( CSteamID steamID ) = 0;
	virtual bool GetEligiblePromoItemDefinitionIDs( CSteamID steamID, int32 *ItemDefIDs, uint32 *ItemDefIDsArraySize ) = 0;

};

#endif // ISTEAMINVENTORY001_H
