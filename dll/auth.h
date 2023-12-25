// source: https://github.com/Detanup01/stmsrv/blob/main/Steam3Server/Others/AppTickets.cs
// thanks Detanup01

#ifndef AUTH_INCLUDE
#define AUTH_INCLUDE

#include "base.h"
#include "mbedtls/pk.h"
#include "mbedtls/x509.h"
#include "mbedtls/error.h"
#include "mbedtls/sha1.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


// the data type is important, we depend on sizeof() for each one of them
constexpr uint32_t STEAM_APPTICKET_SIGLEN = 128;
constexpr uint32_t STEAM_APPTICKET_GCLen = 20;
constexpr uint32_t STEAM_APPTICKET_SESSIONLEN = 24;

// source: https://github.com/Detanup01/stmsrv/blob/main/Cert/AppTicket.key
// thanks Detanup01
const static std::string app_ticket_key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBAMITHOY6pfsvaGTI\n"
    "llmilPa1+ev4BsUV0IW3+F/3pQlZ+o57CO1HbepSh2a37cbGUSehOVQ7lREPVXP3\n"
    "UdyF5tU5IMytJef5N7euM5z2IG9IszeOReO87h2AmtlwGqnRj7qd0MeVxSAuUq7P\n"
    "C/Ir1VyOg58+wAKxaPL18upylnGJAgMBAAECgYEAnKQQj0KG9VYuTCoaL/6pfPcj\n"
    "4PEvhaM1yrfSIKMg8YtOT/G+IsWkUZyK7L1HjUhD+FiIjRQKHNrjfdYAnJz20Xom\n"
    "k6iVt7ugihIne1Q3pGYG8TY9P1DPdN7zEnAVY1Bh2PAlqJWrif3v8v1dUGE/dYr2\n"
    "U3M0JhvzO7VL1B/chIECQQDqW9G5azGMA/cL4jOg0pbj9GfxjJZeT7M2rBoIaRWP\n"
    "C3ROndyb+BNahlKk6tbvqillvvMQQiSFGw/PbmCwtLL3AkEA0/79W0q9d3YCXQGW\n"
    "k3hQvR8HEbxLmRaRF2gU4MOa5C0JqwsmxzdK4mKoJCpVAiu1gmFonLjn2hm8i+vK\n"
    "b7hffwJAEiMpCACTxRJJfFH1TOz/YIT5xmfq+0GPzRtkqGH5mSh5x9vPxwJb/RWI\n"
    "L9s85y90JLuyc/+qc+K0Rol0Ujip4QJAGLXVJEn+8ajAt8SSn5fbmV+/fDK9gRef\n"
    "S+Im5NgH+ubBBL3lBD2Orfqf7K8+f2VG3+6oufPXmpV7Y7fVPdZ40wJALDujJXgi\n"
    "XiCBSht1YScYjfmJh2/xZWh8/w+vs5ZTtrnW2FQvfvVDG9c1hrChhpvmA0QxdgWB\n"
    "zSsAno/utcuB9w==\n"
    "-----END PRIVATE KEY-----\n";


static std::vector<uint8_t> sign_auth_data(const std::string &private_key_content, const std::vector<uint8_t> &data, size_t effective_data_len) {
    std::vector<uint8_t> signature{};

    // Hash the data using SHA-1
    constexpr static int SHA1_DIGEST_LENGTH = 20;
    uint8_t hash[SHA1_DIGEST_LENGTH]{};
    int result = mbedtls_sha1(data.data(), effective_data_len, hash);
    if (result != 0)
    {
#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("sign_auth_data failed to hash the data via SHA1: %s\n", err_msg.c_str());
#endif

        return signature;
    }

    mbedtls_entropy_context entropy_ctx; // entropy context for random number generation
    mbedtls_entropy_init(&entropy_ctx);

    mbedtls_ctr_drbg_context ctr_drbg_ctx; // CTR-DRBG context for deterministic random number generation
    mbedtls_ctr_drbg_init(&ctr_drbg_ctx);

    // seed the CTR-DRBG context with random numbers
    result = mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, mbedtls_entropy_func, &entropy_ctx, nullptr, 0);
    if (mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, mbedtls_entropy_func, &entropy_ctx, nullptr, 0) != 0)
    {
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("sign_auth_data failed to seed the CTR-DRBG context: %s\n", err_msg.c_str());
#endif

        return signature;
    }

    mbedtls_pk_context private_key_ctx; // holds the parsed private key
    mbedtls_pk_init(&private_key_ctx);

    result = mbedtls_pk_parse_key(
        &private_key_ctx,                                      // will hold the parsed private key
        (const unsigned char *)private_key_content.c_str(),
        private_key_content.size() + 1,                        // we MUST include the null terminator, otherwise this API returns an error!
        nullptr, 0,                                            // no password stuff, private key isn't protected
        mbedtls_ctr_drbg_random, &ctr_drbg_ctx                 // random number generation function + the CTR-DRBG context it requires as an input
    );

    if (result != 0)
    {
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("sign_auth_data failed to parse private key: %s\n", err_msg.c_str());
#endif

        return signature;
    }

    // private key must be valid RSA key
    if (mbedtls_pk_get_type(&private_key_ctx) != MBEDTLS_PK_RSA || // invalid type
        mbedtls_pk_can_do(&private_key_ctx, MBEDTLS_PK_RSA) == 0)  // or initialized but not properly setup (maybe freed?)
    {
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

        PRINT_DEBUG("sign_auth_data parsed key is not a valid RSA private key\n");
        return signature;
    }

    // get the underlying RSA context from the parsed private key
    mbedtls_rsa_context* rsa_ctx = mbedtls_pk_rsa(private_key_ctx);

    // resize the output buffer to accomodate the size of the private key
    const size_t private_key_len = mbedtls_pk_get_len(&private_key_ctx);
    if (private_key_len == 0) // TODO must be 128 siglen
    {
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

        PRINT_DEBUG("sign_auth_data failed to get private key (final buffer) length\n");
        return signature;
    }

    PRINT_DEBUG("sign_auth_data computed private key (final buffer) length = %zu\n", private_key_len);
    signature.resize(private_key_len);

    // finally sign the computed hash using RSA and PKCS#1 padding
    result = mbedtls_rsa_pkcs1_sign(
        rsa_ctx,
        mbedtls_ctr_drbg_random, &ctr_drbg_ctx,
        MBEDTLS_MD_SHA1, // we used SHA1 to hash the data
        sizeof(hash), hash,
        signature.data() // output
    );

    mbedtls_pk_free(&private_key_ctx);
    mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
    mbedtls_entropy_free(&entropy_ctx);

    if (result != 0)
    {
        signature.clear();

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("sign_auth_data RSA signing failed: %s\n", err_msg.c_str());
#endif
    }

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        auto str = uint8_vector_to_hex_string(signature);
        PRINT_DEBUG("sign_auth_data final signature [%zu bytes]:\n  %s\n", signature.size(), str.c_str());
#endif

    return signature;
}


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
            "  GCToken: " "%" PRIu64 "\n"
            "  user steam_id: " "%" PRIu64 "\n"
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
        PRINT_DEBUG("AUTH::AppTicketGC::SER final data [%zu bytes]:\n  %s\n", buffer.size(), str.c_str());
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
            "  user steam_id: " "%" PRIu64 "\n"
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
            PRINT_DEBUG("AUTH::AppTicket::SER (before licenses + DLCs):\n  %s\n", str.c_str());
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
            PRINT_DEBUG("AUTH::AppTicket::SER final data [%zu bytes]:\n  %s\n", buffer.size(), str.c_str());
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
        /*
         * layout of Auth_Data with GC:
         * ------------------------
         * X bytes: GC data blob (currently 52 bytes)
         * ------------------------
         * 4 bytes: remaining Auth_Data blob size (4 + Y + Z)
         * ------------------------
         * 4 bytes: size of ticket data layout (not blob!, hence blob + 4)
         * ------------------------
         * Y bytes: ticket data blob
         * ------------------------
         * Z bytes: App Ticket signature
         * ------------------------
         * 
         * total layout length = X + 4 + 4 + Y + Z
         */
        
        /*
         * layout of Auth_Data without GC:
         * ------------------------
         * 4 bytes: size of ticket data layout (not blob!, hence blob + 4)
         * ------------------------
         * Y bytes: ticket data blob
         * ------------------------
         * Z bytes: App Ticket signature
         * ------------------------
         * 
         * total layout length = 4 + Y + Z
         */
        const uint64_t steam_id = id.ConvertToUint64();

        PRINT_DEBUG(
            "AUTH::Auth_Data::SER:\n"
            "  HasGC: %u\n"
            "  user steam_id: " "%" PRIu64 "\n"
            "  number: " "%" PRIu64 "\n",

            (int)HasGC,
            steam_id,
            number
        );

        /*
         * layout of ticket data:
         * ------------------------
         * 4 bytes: size of ticket data layout (not blob!, hence blob + 4)
         * ------------------------
         * Y bytes: ticket data blob
         * ------------------------
         * 
         * total layout length = 4 + Y
         */
        std::vector<uint8_t> tickedData = Ticket.Serialize();
        // we need this variable because we depend on the sizeof, must be 4 bytes
        const uint32_t ticket_data_layout_length =
            sizeof(uint32_t) + // size of this uint32_t because it is included!
            (uint32_t)tickedData.size();

        size_t total_size_without_siglen = ticket_data_layout_length;

        std::vector<uint8_t> GCData;
        size_t gc_data_layout_length = 0;
        if (HasGC)
        {
            /*
             * layout of GC data:
             * ------------------------
             * X bytes: GC data blob (currently 52 bytes)
             * ------------------------
             * 4 bytes: remaining Auth_Data blob size
             * ------------------------
             * 
             * total layout length = X + 4
             */
            GCData = GC.Serialize();
            gc_data_layout_length +=
                GCData.size() +
                sizeof(uint32_t);
            
            total_size_without_siglen += gc_data_layout_length;
        }

        const size_t final_buffer_size = total_size_without_siglen + STEAM_APPTICKET_SIGLEN;
        PRINT_DEBUG(
            "AUTH::Auth_Data::SER size without sig len = %zu, size with sig len (final size) = %zu\n",
            total_size_without_siglen,
            final_buffer_size
        );
        
        std::vector<uint8_t> buffer;
        buffer.resize(final_buffer_size);

        uint8_t* pBuffer = buffer.data();
        
#define SER_VAR(v) \
    *reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
    pBuffer += sizeof(v)
    
        // serialize the GC data first
        if (HasGC)
        {
            memcpy(pBuffer, GCData.data(), GCData.size());
            pBuffer += GCData.size();

            // when GC data is written (HasGC),
            // the next 4 bytes after the GCData will be the length of the remaining data in the final buffer
            // i.e. final buffer size - length of GCData layout
            // i.e. ticket data length + STEAM_APPTICKET_SIGLEN
            //
            // notice that we subtract the entire layout length, not just GCData.size(),
            // otherwise these next 4 bytes will include themselves!
            uint32_t remaining_length = (uint32_t)(final_buffer_size - gc_data_layout_length);
            SER_VAR(remaining_length);
        }
        
        // serialize the ticket data
        SER_VAR(ticket_data_layout_length);
        memcpy(pBuffer, tickedData.data(), tickedData.size());
        
#ifndef EMU_RELEASE_BUILD
        {
            // we nedd a live object until the printf does its job, hence this special handling
            auto str = uint8_vector_to_hex_string(buffer);
            PRINT_DEBUG("AUTH::Auth_Data::SER final data (before signature) [%zu bytes]:\n  %s\n", buffer.size(), str.c_str());
        }
#endif

        //Todo make a signature
        std::vector<uint8_t> signature = sign_auth_data(app_ticket_key, tickedData, total_size_without_siglen);
        if (signature.size() == STEAM_APPTICKET_SIGLEN) {
            memcpy(buffer.data() + total_size_without_siglen, signature.data(), signature.size());

#ifndef EMU_RELEASE_BUILD
        {
            // we nedd a live object until the printf does its job, hence this special handling
            auto str = uint8_vector_to_hex_string(buffer);
            PRINT_DEBUG("AUTH::Auth_Data::SER final data (after signature) [%zu bytes]:\n  %s\n", buffer.size(), str.c_str());
        }
#endif

        } else {
            PRINT_DEBUG("AUTH::Auth_Data::SER signature size [%zu] is invalid\n", signature.size());
        }

#undef SER_VAR

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
