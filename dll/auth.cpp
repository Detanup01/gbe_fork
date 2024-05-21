#include "dll/auth.h"

#define STEAM_ID_OFFSET_TICKET (4 + 8)
#define STEAM_TICKET_MIN_SIZE  (4 + 8 + 8)
#define STEAM_TICKET_MIN_SIZE_NEW 170

#define STEAM_TICKET_PROCESS_TIME 0.03

//Conan Exiles doesn't work with 512 or 128, 256 seems to be the good size
// Usually steam send as 1024 (or recommend sending as that)
//Steam returns 234
#define STEAM_AUTH_TICKET_SIZE 256 //234


static inline int generate_random_int() {
    int a;
    randombytes((char *)&a, sizeof(a));
    return a;
}

static uint32 generate_steam_ticket_id() {
    /* not random starts with 2? */
    static uint32 a = 1;
    ++a;
    // this must never return 0, it is reserved for "k_HAuthTicketInvalid" when the auth APIs fail
    if (a == 0) ++a;
    return a;
}

static uint32_t get_ticket_count() {
    static uint32_t a = 0;
    ++a;
    // this must never return 0, on overflow just go back to 1 again
    if (a == 0) a = 1;
    return a;
}


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


static std::vector<uint8_t> sign_auth_data(const std::string &private_key_content, const std::vector<uint8_t> &data, size_t effective_data_len)
{
    std::vector<uint8_t> signature{};

    // Hash the data using SHA-1
    constexpr static int SHA1_DIGEST_LENGTH = 20;
    uint8_t hash[SHA1_DIGEST_LENGTH]{};
    int result = mbedtls_sha1(data.data(), effective_data_len, hash);
    if (result != 0) {
#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("failed to hash the data via SHA1: %s", err_msg.c_str());
#endif

        return signature;
    }

    mbedtls_entropy_context entropy_ctx; // entropy context for random number generation
    mbedtls_entropy_init(&entropy_ctx);

    mbedtls_ctr_drbg_context ctr_drbg_ctx; // CTR-DRBG context for deterministic random number generation
    mbedtls_ctr_drbg_init(&ctr_drbg_ctx);

    // seed the CTR-DRBG context with random numbers
    result = mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, mbedtls_entropy_func, &entropy_ctx, nullptr, 0);
    if (mbedtls_ctr_drbg_seed(&ctr_drbg_ctx, mbedtls_entropy_func, &entropy_ctx, nullptr, 0) != 0) {
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("failed to seed the CTR-DRBG context: %s", err_msg.c_str());
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

    if (result != 0) {
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("failed to parse private key: %s", err_msg.c_str());
#endif

        return signature;
    }

    // private key must be valid RSA key
    if (mbedtls_pk_get_type(&private_key_ctx) != MBEDTLS_PK_RSA ||  // invalid type
        mbedtls_pk_can_do(&private_key_ctx, MBEDTLS_PK_RSA) == 0) { // or initialized but not properly setup (maybe freed?)
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

        PRINT_DEBUG("parsed key is not a valid RSA private key");
        return signature;
    }

    // get the underlying RSA context from the parsed private key
    mbedtls_rsa_context* rsa_ctx = mbedtls_pk_rsa(private_key_ctx);

    // resize the output buffer to accomodate the size of the private key
    const size_t private_key_len = mbedtls_pk_get_len(&private_key_ctx);
    if (private_key_len == 0) { // TODO must be 128 siglen
        mbedtls_pk_free(&private_key_ctx);
        mbedtls_ctr_drbg_free(&ctr_drbg_ctx);
        mbedtls_entropy_free(&entropy_ctx);

        PRINT_DEBUG("failed to get private key (final buffer) length");
        return signature;
    }

    PRINT_DEBUG("computed private key (final buffer) length = %zu", private_key_len);
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

    if (result != 0) {
        signature.clear();

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        std::string err_msg(256, 0);
        mbedtls_strerror(result, &err_msg[0], err_msg.size());
        PRINT_DEBUG("RSA signing failed: %s", err_msg.c_str());
#endif
    }

#ifndef EMU_RELEASE_BUILD
        // we nedd a live object until the printf does its job, hence this special handling
        auto str = common_helpers::uint8_vector_to_hex_string(signature);
        PRINT_DEBUG("final signature [%zu bytes]:\n  %s", signature.size(), str.c_str());
#endif

    return signature;
}




std::vector<uint8_t> DLC::Serialize() const
{
    PRINT_DEBUG("AppId = %u, Licenses count = %zu", AppId, Licenses.size());

    // we need this variable because we depend on the sizeof, must be 2 bytes
    const uint16_t dlcs_licenses_count = (uint16_t)Licenses.size();
    const size_t dlcs_licenses_total_size =
        Licenses.size() * sizeof(Licenses[0]); // count * element size

    const size_t total_size =
        sizeof(AppId) +
        sizeof(dlcs_licenses_count) +
        dlcs_licenses_total_size;

    std::vector<uint8_t> buffer{};
    buffer.resize(total_size);
    
    uint8_t* pBuffer = &buffer[0];

#define SER_VAR(v) \
*reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
pBuffer += sizeof(v)

    SER_VAR(AppId);
    SER_VAR(dlcs_licenses_count);
    for(uint32_t dlc_license : Licenses) {
        SER_VAR(dlc_license);
    }

#undef SER_VAR

    PRINT_DEBUG("final size = %zu", buffer.size());
    return buffer;
}

std::vector<uint8_t> AppTicketGC::Serialize() const
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
        "\n"
        "  GCToken: " "%" PRIu64 "\n"
        "  user steam_id: " "%" PRIu64 "\n"
        "  ticketGenDate: %u\n"
        "  ExternalIP: 0x%08X, InternalIP: 0x%08X\n"
        "  TimeSinceStartup: %u, TicketGeneratedCount: %u\n"
        "  SER size = %zu",

        GCToken,
        steam_id,
        ticketGenDate,
        ExternalIP, InternalIP,
        TimeSinceStartup, TicketGeneratedCount,
        total_size
    );
    
    std::vector<uint8_t> buffer{};
    buffer.resize(total_size);
    
    uint8_t* pBuffer = &buffer[0];

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
    auto str = common_helpers::uint8_vector_to_hex_string(buffer);
    PRINT_DEBUG("final data [%zu bytes]:\n  %s", buffer.size(), str.c_str());
#endif

    return buffer;
}

std::vector<uint8_t> AppTicket::Serialize() const
{
    const uint64_t steam_id = id.ConvertToUint64();

    PRINT_DEBUG(
        "\n"
        "  Version: %u\n"
        "  user steam_id: " "%" PRIu64 "\n"
        "  AppId: %u\n"
        "  ExternalIP: 0x%08X, InternalIP: 0x%08X\n"
        "  TicketGeneratedDate: %u, TicketGeneratedExpireDate: %u\n"
        "  Licenses count: %zu, DLCs count: %zu",

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
    std::vector<std::vector<uint8_t>> serialized_dlcs{};
    for (const DLC &dlc : DLCs) {
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

    PRINT_DEBUG("final size = %zu", total_size);

    std::vector<uint8_t> buffer{};
    buffer.resize(total_size);
    uint8_t* pBuffer = &buffer[0];

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
        auto str = common_helpers::uint8_vector_to_hex_string(buffer);
        PRINT_DEBUG("(before licenses + DLCs):\n  %s", str.c_str());
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
    for(uint32_t license : Licenses) {
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
    for (const auto &dlc_ser : serialized_dlcs){
        memcpy(pBuffer, dlc_ser.data(), dlc_ser.size());
        pBuffer += dlc_ser.size();
    }

    //padding
    SER_VAR(padding);

#undef SER_VAR

#ifndef EMU_RELEASE_BUILD
    {
        // we nedd a live object until the printf does its job, hence this special handling
        auto str = common_helpers::uint8_vector_to_hex_string(buffer);
        PRINT_DEBUG("final data [%zu bytes]:\n  %s", buffer.size(), str.c_str());
    }
#endif

    return buffer;
}

std::vector<uint8_t> Auth_Data::Serialize() const
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
        "\n"
        "  HasGC: %u\n"
        "  user steam_id: " "%" PRIu64 "\n"
        "  number: " "%" PRIu64 ,

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

    std::vector<uint8_t> GCData{};
    size_t gc_data_layout_length = 0;
    if (HasGC) {
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
    PRINT_DEBUG("size without sig len = %zu, size with sig len (final size) = %zu",
        total_size_without_siglen,
        final_buffer_size
    );
    
    std::vector<uint8_t> buffer;
    buffer.resize(final_buffer_size);

    uint8_t* pBuffer = &buffer[0];
    
#define SER_VAR(v) \
*reinterpret_cast<std::remove_const<decltype(v)>::type *>(pBuffer) = v; \
pBuffer += sizeof(v)

    // serialize the GC data first
    if (HasGC) {
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
        auto str = common_helpers::uint8_vector_to_hex_string(buffer);
        PRINT_DEBUG("final data (before signature) [%zu bytes]:\n  %s", buffer.size(), str.c_str());
    }
#endif

    //Todo make a signature
    std::vector<uint8_t> signature = sign_auth_data(app_ticket_key, tickedData, total_size_without_siglen);
    if (signature.size() == STEAM_APPTICKET_SIGLEN) {
        memcpy(buffer.data() + total_size_without_siglen, signature.data(), signature.size());

#ifndef EMU_RELEASE_BUILD
        {
            // we nedd a live object until the printf does its job, hence this special handling
            auto str = common_helpers::uint8_vector_to_hex_string(buffer);
            PRINT_DEBUG("final data (after signature) [%zu bytes]:\n  %s", buffer.size(), str.c_str());
        }
#endif

    } else {
        PRINT_DEBUG("signature size [%zu] is invalid", signature.size());
    }

#undef SER_VAR

    return buffer;
}



void Auth_Manager::ticket_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Auth_Manager *auth_manager = (Auth_Manager *)object;
    auth_manager->Callback(msg);
}

Auth_Manager::Auth_Manager(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks)
{
    this->network = network;
    this->settings = settings;
    this->callbacks = callbacks;

    this->network->setCallback(CALLBACK_ID_AUTH_TICKET, settings->get_local_steam_id(), &Auth_Manager::ticket_callback, this);
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Auth_Manager::ticket_callback, this);
}

Auth_Manager::~Auth_Manager()
{
    this->network->rmCallback(CALLBACK_ID_AUTH_TICKET, settings->get_local_steam_id(), &Auth_Manager::ticket_callback, this);
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Auth_Manager::ticket_callback, this);
}


void Auth_Manager::launch_callback(CSteamID id, EAuthSessionResponse resp, double delay)
{
    ValidateAuthTicketResponse_t data{};
    data.m_SteamID = id;
    data.m_eAuthSessionResponse = resp;
    data.m_OwnerSteamID = id;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), delay);
}

void Auth_Manager::launch_callback_gs(CSteamID id, bool approved)
{
    if (approved) {
        GSClientApprove_t data{};
        data.m_SteamID = data.m_OwnerSteamID = id;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    } else {
        GSClientDeny_t data{};
        data.m_SteamID = id;
        data.m_eDenyReason = k_EDenyNotLoggedOn; //TODO: other reasons?
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }
}

Auth_Data Auth_Manager::getTicketData( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{

#define IP4_AS_DWORD_LITTLE_ENDIAN(a,b,c,d) (((uint32_t)d)<<24 | ((uint32_t)c)<<16 | ((uint32_t)b)<<8 | (uint32_t)a)

    Auth_Data ticket_data{};
    CSteamID steam_id = settings->get_local_steam_id();
    if (settings->enable_new_app_ticket)
    {
        ticket_data.id = steam_id;
        ticket_data.number = generate_steam_ticket_id();
        ticket_data.Ticket.Version = 4;
        ticket_data.Ticket.id = steam_id;
        ticket_data.Ticket.AppId = settings->get_local_game_id().AppID();
        ticket_data.Ticket.ExternalIP = IP4_AS_DWORD_LITTLE_ENDIAN(127, 0, 0, 1); //TODO
        ticket_data.Ticket.InternalIP = IP4_AS_DWORD_LITTLE_ENDIAN(127, 0, 0, 1);
        ticket_data.Ticket.AlwaysZero = 0;
        const auto curTime = std::chrono::system_clock::now();
        const auto GenDate = std::chrono::duration_cast<std::chrono::seconds>(curTime.time_since_epoch());
        ticket_data.Ticket.TicketGeneratedDate = (uint32_t)GenDate.count();
        uint32_t expTime = (uint32_t)(GenDate + std::chrono::hours(24)).count();
        ticket_data.Ticket.TicketGeneratedExpireDate = expTime;
        ticket_data.Ticket.Licenses.resize(0);
        ticket_data.Ticket.Licenses.push_back(0); //TODO
        unsigned int dlcCount = settings->DLCCount();
        ticket_data.Ticket.DLCs.resize(0);  //currently set to 0
        for (unsigned i = 0; i < dlcCount; ++i)
        {
            DLC dlc;
            AppId_t appid;
            bool available;
            std::string name;
            if (!settings->getDLC(i, appid, available, name)) break;
            dlc.AppId = (uint32_t)appid;
            dlc.Licenses.resize(0); //TODO
            ticket_data.Ticket.DLCs.push_back(dlc);
        }
        ticket_data.HasGC = false;
        if (settings->use_gc_token)
        {
            ticket_data.HasGC = true;
            ticket_data.GC.GCToken = generate_random_int();
            ticket_data.GC.id = steam_id;
            ticket_data.GC.ticketGenDate = (uint32_t)GenDate.count();
            ticket_data.GC.ExternalIP = IP4_AS_DWORD_LITTLE_ENDIAN(127, 0, 0, 1);
            ticket_data.GC.InternalIP = IP4_AS_DWORD_LITTLE_ENDIAN(127, 0, 0, 1);
            ticket_data.GC.TimeSinceStartup = (uint32_t)std::chrono::duration_cast<std::chrono::seconds>(curTime - startup_time).count();
            ticket_data.GC.TicketGeneratedCount = get_ticket_count();
        }
        std::vector<uint8_t> ser = ticket_data.Serialize();
        uint32_t ser_size = static_cast<uint32_t>(ser.size());
        *pcbTicket = ser_size;
        if (cbMaxTicket >= ser_size)
            memcpy(pTicket, ser.data(), ser_size);
    }
    else
    {
        memset(pTicket, 123, cbMaxTicket);
        ((char *)pTicket)[0] = 0x14;
        ((char *)pTicket)[1] = 0;
        ((char *)pTicket)[2] = 0;
        ((char *)pTicket)[3] = 0;
        uint64 steam_id_buff = steam_id.ConvertToUint64();
        memcpy((char *)pTicket + STEAM_ID_OFFSET_TICKET, &steam_id_buff, sizeof(steam_id_buff));
        *pcbTicket = cbMaxTicket;

        ticket_data.id = steam_id;
        ticket_data.number = generate_steam_ticket_id();
        uint32 ttt = ticket_data.number;

        memcpy(((char *)pTicket) + sizeof(uint64), &ttt, sizeof(ttt));
    }

#undef IP4_AS_DWORD_LITTLE_ENDIAN

    return ticket_data;
}

HAuthTicket Auth_Manager::getTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    if (settings->enable_new_app_ticket)
    {
        if (cbMaxTicket < STEAM_TICKET_MIN_SIZE_NEW) return k_HAuthTicketInvalid;
    }
    else
    {
        if (cbMaxTicket < STEAM_TICKET_MIN_SIZE) return k_HAuthTicketInvalid;
        if (cbMaxTicket > STEAM_AUTH_TICKET_SIZE) cbMaxTicket = STEAM_AUTH_TICKET_SIZE;
    }
    
    Auth_Data ticket_data = getTicketData(pTicket, cbMaxTicket, pcbTicket );

    if (*pcbTicket > cbMaxTicket)
        return k_HAuthTicketInvalid;

    GetAuthSessionTicketResponse_t data{};
    data.m_hAuthTicket = (HAuthTicket)ticket_data.number;
    data.m_eResult = EResult::k_EResultOK;
    
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), STEAM_TICKET_PROCESS_TIME);
    outbound.push_back(ticket_data);

    return data.m_hAuthTicket;
}

HAuthTicket Auth_Manager::getWebApiTicket( const char* pchIdentity )
{
    // https://docs.unity.com/ugs/en-us/manual/authentication/manual/platform-signin-steam
    GetTicketForWebApiResponse_t data{};

    uint32 cbTicket = 0;
    Auth_Data ticket_data = getTicketData(data.m_rgubTicket, sizeof(data.m_rgubTicket), &cbTicket);

    if (cbTicket > sizeof(data.m_rgubTicket))
        return k_HAuthTicketInvalid;

    data.m_cubTicket = (int)cbTicket;
    data.m_hAuthTicket = (HAuthTicket)ticket_data.number;
    data.m_eResult = EResult::k_EResultOK;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), STEAM_TICKET_PROCESS_TIME);
    outbound.push_back(ticket_data);

    return data.m_hAuthTicket;
}

CSteamID Auth_Manager::fakeUser()
{
    Auth_Data data = {};
    data.id = generate_steam_anon_user();
    inbound.push_back(data);
    return data.id;
}

void Auth_Manager::cancelTicket(uint32 number)
{
    auto ticket = std::find_if(outbound.begin(), outbound.end(), [&number](Auth_Data const& item) { return item.number == number; });
    if (outbound.end() == ticket)
        return;

    Auth_Ticket *auth_ticket = new Auth_Ticket();
    auth_ticket->set_number(number);
    auth_ticket->set_type(Auth_Ticket::CANCEL);
    Common_Message msg;
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    msg.set_allocated_auth_ticket(auth_ticket);
    network->sendToAll(&msg, true);

    outbound.erase(ticket);
}

bool Auth_Manager::SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser )
{
    if (cubAuthBlobSize < STEAM_TICKET_MIN_SIZE) return false;

    Auth_Data data;
    uint64 id;
    memcpy(&id, (char *)pvAuthBlob + STEAM_ID_OFFSET_TICKET, sizeof(id));
    uint32 number;
    memcpy(&number, ((char *)pvAuthBlob) + sizeof(uint64), sizeof(number));
    data.id = CSteamID(id);
    data.number = number;
    if (pSteamIDUser) *pSteamIDUser = data.id;

    for (auto & t : inbound) {
        if (t.id == data.id) {
            //Should this return false?
            launch_callback_gs(id, true);
            return true;
        }
    }

    inbound.push_back(data);
    launch_callback_gs(id, true);
    return true;
}

EBeginAuthSessionResult Auth_Manager::beginAuth(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    if (cbAuthTicket < STEAM_TICKET_MIN_SIZE) return k_EBeginAuthSessionResultInvalidTicket;

    Auth_Data data;
    uint64 id;
    memcpy(&id, (char *)pAuthTicket + STEAM_ID_OFFSET_TICKET, sizeof(id));
    uint32 number;
    memcpy(&number, ((char *)pAuthTicket) + sizeof(uint64), sizeof(number));
    data.id = CSteamID(id);
    data.number = number;
    data.created = std::chrono::high_resolution_clock::now();

    for (auto & t : inbound) {
        if (t.id == data.id && !check_timedout(t.created, STEAM_TICKET_PROCESS_TIME)) {
            return k_EBeginAuthSessionResultDuplicateRequest;
        }
    }

    inbound.push_back(data);
    launch_callback(steamID, k_EAuthSessionResponseOK, STEAM_TICKET_PROCESS_TIME);
    return k_EBeginAuthSessionResultOK;
}

uint32 Auth_Manager::countInboundAuth()
{
    return static_cast<uint32>(inbound.size());
}

bool Auth_Manager::endAuth(CSteamID id)
{
    bool erased = false;
    auto t = std::begin(inbound);
    while (t != std::end(inbound)) {
        if (t->id == id) {
            erased = true;
            t = inbound.erase(t);
        } else {
            ++t;
        }
    }

    return erased;
}



void Auth_Manager::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {
            PRINT_DEBUG("TICKET DISCONNECT");
            auto t = std::begin(inbound);
            while (t != std::end(inbound)) {
                if (t->id.ConvertToUint64() == msg->source_id()) {
                    launch_callback(t->id, k_EAuthSessionResponseUserNotConnectedToSteam);
                    t = inbound.erase(t);
                } else {
                    ++t;
                }
            }
        }
    }

    if (msg->has_auth_ticket()) {
        if (msg->auth_ticket().type() == Auth_Ticket::CANCEL) {
            PRINT_DEBUG("TICKET CANCEL " "%" PRIu64, msg->source_id());
            uint32 number = msg->auth_ticket().number();
            auto t = std::begin(inbound);
            while (t != std::end(inbound)) {
                if (t->id.ConvertToUint64() == msg->source_id() && t->number == number) {
                    PRINT_DEBUG("TICKET CANCELED");
                    launch_callback(t->id, k_EAuthSessionResponseAuthTicketCanceled);
                    t = inbound.erase(t);
                } else {
                    ++t;
                }
            }
        }
    }
}
