#ifndef ISTEAMNETWORKINGSOCKETSSERIALIZED_H
#define ISTEAMNETWORKINGSOCKETSSERIALIZED_H

#include "steamtypes.h"
#include "steamclientpublic.h"

class ISteamNetworkingSocketsSerialized002
{
public:
    virtual void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous ) = 0;
    virtual void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason ) = 0;
    //SteamNetworkingSocketsCert_t
    virtual SteamAPICall_t GetCertAsync() = 0;
    virtual int GetNetworkConfigJSON( void *buf, uint32 cbBuf ) = 0;
    virtual void CacheRelayTicket( const void *pTicket, uint32 cbTicket ) = 0;
    virtual uint32 GetCachedRelayTicketCount() = 0;
    virtual int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf ) = 0;
    virtual void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg ) = 0;
};

class ISteamNetworkingSocketsSerialized003
{
public:
	virtual void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous ) = 0;
	virtual void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason ) = 0;
	virtual SteamAPICall_t GetCertAsync() = 0;
	virtual int GetNetworkConfigJSON( void *buf, uint32 cbBuf, const char *pszLauncherPartner ) = 0;
	virtual void CacheRelayTicket( const void *pTicket, uint32 cbTicket ) = 0;
	virtual uint32 GetCachedRelayTicketCount() = 0;
	virtual int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf ) = 0;
	virtual void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg ) = 0;
};

class ISteamNetworkingSocketsSerialized004
{
public:
	virtual void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous ) = 0;
	virtual void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason ) = 0;
	virtual SteamAPICall_t GetCertAsync() = 0;
	virtual int GetNetworkConfigJSON( void *buf, uint32 cbBuf, const char *pszLauncherPartner ) = 0;
	virtual void CacheRelayTicket( const void *pTicket, uint32 cbTicket ) = 0;
	virtual uint32 GetCachedRelayTicketCount() = 0;
	virtual int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf ) = 0;
	virtual void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg ) = 0;
    virtual bool GetSTUNServer(int dont_know, char *buf, unsigned int len) = 0;
    virtual bool BAllowDirectConnectToPeer(SteamNetworkingIdentity const &identity) = 0;
};

class ISteamNetworkingSocketsSerialized005
{
public:
	virtual void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous ) = 0;
	virtual void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason ) = 0;
	virtual SteamAPICall_t GetCertAsync() = 0;
	virtual int GetNetworkConfigJSON( void *buf, uint32 cbBuf, const char *pszLauncherPartner ) = 0;
	virtual void CacheRelayTicket( const void *pTicket, uint32 cbTicket ) = 0;
	virtual uint32 GetCachedRelayTicketCount() = 0;
	virtual int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf ) = 0;
	virtual void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg ) = 0;
    virtual bool GetSTUNServer(int dont_know, char *buf, unsigned int len) = 0;
    virtual bool BAllowDirectConnectToPeer(SteamNetworkingIdentity const &identity) = 0;
    virtual int BeginAsyncRequestFakeIP(int a) = 0; //not sure if the right return type
};

struct SteamNetworkingSocketsConfigUpdated_t
{
    enum { k_iCallback = k_iSteamNetworkingCallbacks + 95 };
    //TODO total size: 4
    char placeholder[4];
};

struct SteamNetworkingSocketsCert_t
{
    enum { k_iCallback = k_iSteamNetworkingCallbacks + 96 };

    EResult m_eResult;
    uint32 m_cbCert;
    char m_certOrMsg[0x200]; //CMsgSteamDatagramCertificate protobuf
    uint64 m_caKeyID;
    uint32 m_cbSignature;
    char m_signature[0x80];
    uint32 m_cbPrivKey;
    char m_privKey[0x80];
    //total size: 792
    //0
    //4
    //8 []
    //0x208
    //0x20C
    //0x210
    //0x214 []
    //0x294
    //0x298 []
};

struct SteamNetworkingSocketsRecvP2PFailure_t
{
    enum { k_iCallback = k_iSteamNetworkingCallbacks + 97 };
    uint64 steamIDRemote;
    uint32 unConnectionIDDest;
    uint32 placeholder; //not sure what this is supposed to be
    uint32 nReason;
    char pszReason[128];
    //total size: 148
};

struct SteamNetworkingSocketsRecvP2PRendezvous_t
{
    enum { k_iCallback = k_iSteamNetworkingCallbacks + 98 };
    uint64 steamIDRemote;
    uint32 unConnectionIDSrc;
    uint32 m_cbRendezvous;
    char m_MsgRendezvous[512];
    //total size: 528
};


#define STEAMNETWORKINGSOCKETSSERIALIZED_INTERFACE_VERSION "SteamNetworkingSocketsSerialized002"

#endif
