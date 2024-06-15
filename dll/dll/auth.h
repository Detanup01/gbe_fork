// source: https://github.com/Detanup01/stmsrv/blob/main/Steam3Server/Others/AppTickets.cs
// thanks Detanup01

#ifndef AUTH_INCLUDE_H
#define AUTH_INCLUDE_H

#include "base.h"
#include "include.wrap.mbedtls.h"


// the data type is important, we depend on sizeof() for each one of them
constexpr const static uint32_t STEAM_APPTICKET_SIGLEN = 128;
constexpr const static uint32_t STEAM_APPTICKET_GCLen = 20;
constexpr const static uint32_t STEAM_APPTICKET_SESSIONLEN = 24;


struct DLC {
    uint32_t AppId{};
    std::vector<uint32_t> Licenses{};

    std::vector<uint8_t> Serialize() const;
};

struct AppTicketGC {
    uint64_t GCToken{};
    CSteamID id{};
    uint32_t ticketGenDate{}; //epoch
    uint32_t ExternalIP{};
    uint32_t InternalIP{};
    uint32_t TimeSinceStartup{};
    uint32_t TicketGeneratedCount{};

private:
    uint32_t one = 1;
    uint32_t two = 2;

public:
    std::vector<uint8_t> Serialize() const;
};

struct AppTicket {
    uint32_t Version{};
    CSteamID id{};
    uint32_t AppId{};
    uint32_t ExternalIP{};
    uint32_t InternalIP{};
    uint32_t AlwaysZero = 0; //OwnershipFlags?
    uint32_t TicketGeneratedDate{};
    uint32_t TicketGeneratedExpireDate{};
    std::vector<uint32_t> Licenses{};
    std::vector<DLC> DLCs{};

    std::vector<uint8_t> Serialize() const;
};

struct Auth_Data {
    bool HasGC{};
    AppTicketGC GC{};
    AppTicket Ticket{};
    //old data
    CSteamID id{};
    uint64_t number{};
    std::chrono::high_resolution_clock::time_point created{};

    std::vector<uint8_t> Serialize() const;
};



class Auth_Manager {
    class Settings *settings{};
    class Networking *network{};
    class SteamCallBacks *callbacks{};

    std::vector<struct Auth_Data> inbound{};
    std::vector<struct Auth_Data> outbound{};

    void launch_callback(CSteamID id, EAuthSessionResponse resp, double delay=0);
    void launch_callback_gs(CSteamID id, bool approved);

    static void ticket_callback(void *object, Common_Message *msg);
    void Callback(Common_Message *msg);

public:
    Auth_Manager(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks);
    ~Auth_Manager();

    HAuthTicket getTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
    HAuthTicket getWebApiTicket( const char *pchIdentity );
    void cancelTicket(uint32 number);

    EBeginAuthSessionResult beginAuth(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID);
    bool endAuth(CSteamID id);

    uint32 countInboundAuth();

    bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser );

    CSteamID fakeUser();
    
    Auth_Data getTicketData( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
};

#endif // AUTH_INCLUDE_H
