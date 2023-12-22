#ifndef AUTH_INCLUDE
#define AUTH_INCLUDE

#include "base.h"
//#include "common_includes.h"
#include "../sha/sha1.hpp"
#include <ctime>
#include <sstream>
#include <string>
#include <iostream>

// the data type is important, we depend on sizeof() for each one of them
constexpr uint32_t STEAM_APPTICKET_SIGLEN = 128;
constexpr uint32_t STEAM_APPTICKET_GCLen = 20;
constexpr uint32_t STEAM_APPTICKET_SESSIONLEN = 24;

struct DLC {
    uint32_t AppId;
    std::vector<uint32_t> Licenses;

    std::vector<uint8_t> Serialize()
    {
        PRINT_DEBUG("AUTH::DLC::SER AppId = %u, Licenses count = %zu\n", AppId, Licenses.size());

        // we need this variable because we depend on the sizeof, must be 2 bytes
        const uint16_t dlcs_licenses_count = (uint16_t)Licenses.size();
        const size_t dlcs_licenses_total_size =
            Licenses.size() * sizeof(Licenses[0]); // count * element size

        const size_t total_size =
            sizeof(AppId) +
            sizeof(dlcs_licenses_count) +
            dlcs_licenses_total_size;

        std::vector<uint8_t> buffer;
        buffer.resize(total_size);
        
        uint8_t* pBuffer = buffer.data();

#define SER_VAR(v) \
    *reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
    pBuffer += sizeof(v)
    
        SER_VAR(AppId);
        SER_VAR(dlcs_licenses_count);
        for(uint32_t dlc_license : Licenses)
        {
            SER_VAR(dlc_license);
        }

#undef SER_VAR

        PRINT_DEBUG("AUTH::DLC::SER final size = %zu\n", buffer.size());
        return buffer;
    }
};

struct AppTicketGC {
    uint64_t GCToken;
    CSteamID id;
    uint32_t ticketGenDate; //epoch
    uint32_t ExternalIP;
    uint32_t InternalIP;
    uint32_t TimeSinceStartup;
    uint32_t TicketGeneratedCount;

private:
    uint32_t one = 1;
    uint32_t two = 2;

public:
    std::vector<uint8_t> Serialize()
    {
        const uint64_t steam_id = id.ConvertToUint64();

        // must be 52
        constexpr size_t total_size = 
            sizeof(STEAM_APPTICKET_GCLen) +
            sizeof(GCToken) +
            sizeof(steam_id) +
            sizeof(ticketGenDate) +
            sizeof(STEAM_APPTICKET_SESSIONLEN) +
            sizeof(one) +
            sizeof(two) +
            sizeof(ExternalIP) +
            sizeof(InternalIP) +
            sizeof(TimeSinceStartup) +
            sizeof(TicketGeneratedCount);

        // check the size at compile time, we must ensure the correct size
#ifndef EMU_RELEASE_BUILD
            static_assert(
                total_size == 52, 
                "AUTH::AppTicketGC::SER calculated size of serialized data != 52 bytes, your compiler has some incorrect sizes"
            );
#endif

        PRINT_DEBUG(
            "AUTH::AppTicketGC::SER Token:\n"
            "  GCToken: %I64u\n"
            "  user steam_id: %I64u\n"
            "  ticketGenDate: %u\n"
            "  ExternalIP: 0x%08X, InternalIP: 0x%08X\n"
            "  TimeSinceStartup: %u, TicketGeneratedCount: %u\n"
            "  SER size = %zu\n",

            GCToken,
            steam_id,
            ticketGenDate,
            ExternalIP, InternalIP,
            TimeSinceStartup, TicketGeneratedCount,
            total_size
        );
        
        std::vector<uint8_t> buffer;
        buffer.resize(total_size);
        
        uint8_t* pBuffer = buffer.data();

#define SER_VAR(v) \
    *reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
    pBuffer += sizeof(v)
    
        SER_VAR(STEAM_APPTICKET_GCLen);
        SER_VAR(GCToken);
        SER_VAR(steam_id);
        SER_VAR(ticketGenDate);
        SER_VAR(STEAM_APPTICKET_SESSIONLEN);
        SER_VAR(one);
        SER_VAR(two);
        SER_VAR(ExternalIP);
        SER_VAR(InternalIP);
        SER_VAR(TimeSinceStartup);
        SER_VAR(TicketGeneratedCount);

#undef SER_VAR

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        auto str = uint8_vector_to_hex_string(buffer);
        PRINT_DEBUG("AUTH::AppTicketGC::SER final data [%zu bytes]: %s\n", buffer.size(), str.c_str());
#endif

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
        const uint64_t steam_id = id.ConvertToUint64();

        PRINT_DEBUG(
            "AUTH::AppTicket::SER:\n"
            "  Version: %u\n"
            "  user steam_id: %I64u\n"
            "  AppId: %u\n"
            "  ExternalIP: 0x%08X, InternalIP: 0x%08X\n"
            "  TicketGeneratedDate: %u, TicketGeneratedExpireDate: %u\n"
            "  Licenses count: %zu, DLCs count: %zu\n",

            Version,
            steam_id,
            AppId,
            ExternalIP, InternalIP,
            TicketGeneratedDate, TicketGeneratedExpireDate,
            Licenses.size(), DLCs.size()
        );

        // we need this variable because we depend on the sizeof, must be 2 bytes
        const uint16_t licenses_count = (uint16_t)Licenses.size();
        const size_t licenses_total_size =
            Licenses.size() * sizeof(Licenses[0]); // total count * element size

        // we need this variable because we depend on the sizeof, must be 2 bytes
        const uint16_t dlcs_count = (uint16_t)DLCs.size();
        size_t dlcs_total_size = 0;
        std::vector<std::vector<uint8_t>> serialized_dlcs;
        for (DLC &dlc : DLCs)
        {
            auto dlc_ser = dlc.Serialize();
            dlcs_total_size += dlc_ser.size();
            serialized_dlcs.push_back(dlc_ser);
        }

        //padding
        constexpr uint16_t padding = (uint16_t)0;

        // must be 42
        constexpr size_t static_fields_size =
            sizeof(Version) +
            sizeof(steam_id) +
            sizeof(AppId) +
            sizeof(ExternalIP) +
            sizeof(InternalIP) +
            sizeof(AlwaysZero) +
            sizeof(TicketGeneratedDate) +
            sizeof(TicketGeneratedExpireDate) +

            sizeof(licenses_count) +
            sizeof(dlcs_count) +

            sizeof(padding);

        // check the size at compile time, we must ensure the correct size
#ifndef EMU_RELEASE_BUILD
            static_assert(
                static_fields_size == 42, 
                "AUTH::AppTicket::SER calculated size of serialized data != 42 bytes, your compiler has some incorrect sizes"
            );
#endif

        const size_t total_size =
            static_fields_size +
            licenses_total_size +
            dlcs_total_size;

        PRINT_DEBUG("AUTH::AppTicket::SER final size = %zu\n", total_size);

        std::vector<uint8_t> buffer;
        buffer.resize(total_size);
        uint8_t* pBuffer = buffer.data();

#define SER_VAR(v) \
    *reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
    pBuffer += sizeof(v)
    
        SER_VAR(Version);
        SER_VAR(steam_id);
        SER_VAR(AppId);
        SER_VAR(ExternalIP);
        SER_VAR(InternalIP);
        SER_VAR(AlwaysZero);
        SER_VAR(TicketGeneratedDate);
        SER_VAR(TicketGeneratedExpireDate);

#ifndef EMU_RELEASE_BUILD
        {
            // we nedd a live object until the printf does its job, hence this special handling
            auto str = uint8_vector_to_hex_string(buffer);
            PRINT_DEBUG("AUTH::AppTicket::SER (before licenses + DLCs): %s\n", str.c_str());
        }
#endif

        /*
         * layout of licenses:
         * ------------------------
         * 2 bytes: count of licenses
         * ------------------------
         * [
         *   ------------------------
         *   | 4 bytes: license element
         *   ------------------------
         * ]
        */
        SER_VAR(licenses_count);
        for(uint32_t license : Licenses)
        {
            SER_VAR(license);
        }        

        /*
         * layout of DLCs:
         * ------------------------
         * | 2 bytes: count of DLCs
         * ------------------------
         * [
         *   ------------------------
         *   | 4 bytes: app id
         *   ------------------------
         *   | 2 bytes: DLC licenses count
         *   ------------------------
         *   [
         *     4 bytes: DLC license element
         *   ]
         * ]
         */
        SER_VAR(dlcs_count);
        for (std::vector<uint8_t> &dlc_ser : serialized_dlcs)
        {
            memcpy(pBuffer, dlc_ser.data(), dlc_ser.size());
            pBuffer += dlc_ser.size();
        }

        //padding
        SER_VAR(padding);

#undef SER_VAR

#ifndef EMU_RELEASE_BUILD
        {
            // we nedd a live object until the printf does its job, hence this special handling
            auto str = uint8_vector_to_hex_string(buffer);
            PRINT_DEBUG("AUTH::AppTicket::SER final data [%zu bytes]: %s\n", buffer.size(), str.c_str());
        }
#endif

        return buffer;
    }
};

struct Auth_Data {
    bool HasGC;
    AppTicketGC GC;
    AppTicket Ticket;
    //old data
    CSteamID id;
    uint64_t number;
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
        
#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        auto str = uint8_vector_to_hex_string(buffer);
        PRINT_DEBUG("AUTH::Auth_Data::SER final data [%zu bytes]: %s\n", buffer.size(), str.c_str());
#endif

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
    std::vector<struct Auth_Data> inbound;
    std::vector<struct Auth_Data> outbound;
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

#endif // AUTH_INCLUDE
