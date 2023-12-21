#ifndef AUTH_INCLUDE
#define AUTH_INCLUDE

#include "base.h"
//#include "common_includes.h"
#include "../sha/sha1.hpp"
#include <ctime>
#include <sstream>
#include <string>
#include <iostream>

#define STEAM_APPTICKET_SIGLEN 128
#define STEAM_APPTICKET_GCLen 20
#define STEAM_APPTICKET_SESSIONLEN 24

struct DLC {
    uint32_t AppId;
    std::vector<uint32_t> Licenses;
};

struct AppTicketGC {
    uint64 GCToken;
    CSteamID id;
    uint32_t ticketGenDate; //epoch
    uint32_t one = 1;
    uint32_t two = 2;
    uint32_t ExternalIP;
    uint32_t InternalIP;
    uint32_t TimeSinceStartup;
    uint32_t TicketGeneratedCount;

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;
        buffer.resize(52);
        pBuffer = buffer.data();
        PRINT_DEBUG("AppTicketGC: Token: %I64u Startup: %u count: %u", GCToken, TimeSinceStartup, TicketGeneratedCount);
        *reinterpret_cast<uint32_t*>(pBuffer) = STEAM_APPTICKET_GCLen;      pBuffer += 4;
        *reinterpret_cast<uint64_t*>(pBuffer) = GCToken;      pBuffer += 8;
        *reinterpret_cast<uint64_t*>(pBuffer) = id.ConvertToUint64();      pBuffer += 8;
        *reinterpret_cast<uint32_t*>(pBuffer) = ticketGenDate;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = STEAM_APPTICKET_SESSIONLEN;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = one;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = two;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = ExternalIP;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = InternalIP;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TimeSinceStartup;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketGeneratedCount;      pBuffer += 4;
        PRINT_DEBUG("AppTicketGC SER: %s\n",uint8_vector_to_hex_string(buffer).c_str());
        return buffer;
    }
};

struct AppTicket {
    uint32_t Version;
    CSteamID id;
    uint32_t AppId;
    uint32_t ExternalIP;
    uint32_t InternalIP;
    uint32_t AlwaysZero = 0; //OwnershipFlags?
    uint32_t TicketGeneratedDate;
    uint32_t TicketGeneratedExpireDate;
    std::vector<uint32_t> Licenses;
    std::vector<DLC> DLCs;

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;
        PRINT_DEBUG("AppTicket Licenses Size : %u, DLCs: %u\n",(uint16_t)Licenses.size(), (uint16_t)DLCs.size());
        uint32_t licSize = Licenses.size() * 4;
        uint32_t dlcSize = 0;
        for(DLC dlc_s : DLCs)
        {
            dlcSize += 4;
            dlcSize += 2;
            dlcSize += (uint32_t)dlc_s.Licenses.size() * 4;
        }
        PRINT_DEBUG("AppTicket Size: %i \n" + (42 + licSize + dlcSize));
        buffer.resize(42 + licSize+ dlcSize);
        pBuffer = buffer.data();
        *reinterpret_cast<uint32_t*>(pBuffer) = Version;      pBuffer += 4;
        *reinterpret_cast<uint64_t*>(pBuffer) = id.ConvertToUint64();      pBuffer += 8;
        *reinterpret_cast<uint32_t*>(pBuffer) = AppId;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = ExternalIP;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = InternalIP;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = AlwaysZero;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketGeneratedDate;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketGeneratedExpireDate;      pBuffer += 4;
        PRINT_DEBUG("AppTicket SER (before): %s\n",uint8_vector_to_hex_string(buffer).c_str());
        *reinterpret_cast<uint16_t*>(pBuffer) = (uint16_t)Licenses.size();      pBuffer += 2;

        for(uint32_t license : Licenses)
        {
            *reinterpret_cast<uint32_t*>(pBuffer) = license;      pBuffer += 4;
        }       

        *reinterpret_cast<uint16_t*>(pBuffer) = (uint16_t)DLCs.size();      pBuffer += 2;

        for(DLC dlc : DLCs)
        {
            *reinterpret_cast<uint32_t*>(pBuffer) = dlc.AppId;      pBuffer += 4;
            *reinterpret_cast<uint16_t*>(pBuffer) = (uint16_t)dlc.Licenses.size();      pBuffer += 2;

            for(uint32_t dlc_license : dlc.Licenses)
            {
                *reinterpret_cast<uint32_t*>(pBuffer) = dlc_license;      pBuffer += 4;
            }     
        }

        *reinterpret_cast<uint16_t*>(pBuffer) = (uint16_t)0;      pBuffer += 2;   //padding
        PRINT_DEBUG("AppTicket SER : %s\n",uint8_vector_to_hex_string(buffer).c_str());
        return buffer;
    }
};
struct Auth_Data {
    bool HasGC;
    AppTicketGC GC;
    AppTicket Ticket;
    //old data
    CSteamID id;
    uint64 number;
    std::chrono::high_resolution_clock::time_point created;

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;
        
        std::vector<uint8_t> tickedData = Ticket.Serialize();
        uint32_t size = tickedData.size() + 4;
        std::vector<uint8_t> GCData;    
        if (HasGC)
        {
            GCData = GC.Serialize();
            size += GCData.size() + 4;
        }
        PRINT_DEBUG("Ticket Ser Size: %u\n", size);
        buffer.resize(size+STEAM_APPTICKET_SIGLEN);
        pBuffer = buffer.data();
        if (HasGC)
        {
            memcpy(pBuffer, GCData.data(), GCData.size());
            pBuffer+= GCData.size();
            *reinterpret_cast<uint32_t*>(pBuffer) = (128+tickedData.size()+4);      pBuffer += 4;
        }
        
        *reinterpret_cast<uint32_t*>(pBuffer) = (tickedData.size()+4);      pBuffer += 4;      
        memcpy(pBuffer, tickedData.data(), tickedData.size());
        PRINT_DEBUG("Auth_Data SER : %s\n",uint8_vector_to_hex_string(buffer).c_str());
        //Todo make a signature
        return buffer;
    }
};



class Auth_Manager {
    class Settings *settings;
    class Networking *network;
    class SteamCallBacks *callbacks;

    void launch_callback(CSteamID id, EAuthSessionResponse resp, double delay=0);
    void launch_callback_gs(CSteamID id, bool approved);
    std::vector<struct Auth_Data> inbound, outbound;
public:
    Auth_Manager(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks);

    void Callback(Common_Message *msg);
    uint32 getTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
    uint32 getWebApiTicket( const char *pchIdentity );
    void cancelTicket(uint32 number);
    EBeginAuthSessionResult beginAuth(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID);
    bool endAuth(CSteamID id);
    uint32 countInboundAuth();
    bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser );
    CSteamID fakeUser();
    Auth_Data getTicketData( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
};

#endif