#include "dll/auth.h"

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


static void steam_auth_manager_ticket_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_auth_manager_ticket_callback\n");

    Auth_Manager *auth_manager = (Auth_Manager *)object;
    auth_manager->Callback(msg);
}

Auth_Manager::Auth_Manager(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks) {
    this->network = network;
    this->settings = settings;
    this->callbacks = callbacks;

    this->network->setCallback(CALLBACK_ID_AUTH_TICKET, settings->get_local_steam_id(), &steam_auth_manager_ticket_callback, this);
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &steam_auth_manager_ticket_callback, this);
}

#define STEAM_TICKET_PROCESS_TIME 0.03

void Auth_Manager::launch_callback(CSteamID id, EAuthSessionResponse resp, double delay)
{
    ValidateAuthTicketResponse_t data;
    data.m_SteamID = id;
    data.m_eAuthSessionResponse = resp;
    data.m_OwnerSteamID = id;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), delay);
}

void Auth_Manager::launch_callback_gs(CSteamID id, bool approved)
{
    if (approved) {
        GSClientApprove_t data;
        data.m_SteamID = data.m_OwnerSteamID = id;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    } else {
        GSClientDeny_t data;
        data.m_SteamID = id;
        data.m_eDenyReason = k_EDenyNotLoggedOn; //TODO: other reasons?
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }
}

#define STEAM_ID_OFFSET_TICKET (4 + 8)
#define STEAM_TICKET_MIN_SIZE  (4 + 8 + 8)
#define STEAM_TICKET_MIN_SIZE_NEW 170

Auth_Data Auth_Manager::getTicketData( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{

#define IP4_AS_DWORD_LITTLE_ENDIAN(a,b,c,d) (((uint32_t)d)<<24 | ((uint32_t)c)<<16 | ((uint32_t)b)<<8 | (uint32_t)a)

    Auth_Data ticket_data;
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
        for (int i = 0; i < dlcCount; ++i)
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
        *pcbTicket = ser.size();
        memcpy(pTicket, ser.data(), ser.size());
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

//Conan Exiles doesn't work with 512 or 128, 256 seems to be the good size
//Steam returns 234
#define STEAM_AUTH_TICKET_SIZE 256 //234

uint32 Auth_Manager::getTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    if (settings->enable_new_app_ticket)
    {
        if (cbMaxTicket < STEAM_TICKET_MIN_SIZE_NEW) return 0;
    }
    else
    {
        if (cbMaxTicket < STEAM_TICKET_MIN_SIZE) return 0;
        if (cbMaxTicket > STEAM_AUTH_TICKET_SIZE) cbMaxTicket = STEAM_AUTH_TICKET_SIZE;
    }
    
    Auth_Data ticket_data = getTicketData(pTicket, cbMaxTicket, pcbTicket );
    uint32 ttt = ticket_data.number;
    GetAuthSessionTicketResponse_t data;
    data.m_hAuthTicket = ttt;
    data.m_eResult = k_EResultOK;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), STEAM_TICKET_PROCESS_TIME);

    outbound.push_back(ticket_data);

    return ttt;
}

uint32 Auth_Manager::getWebApiTicket( const char* pchIdentity )
{
    // https://docs.unity.com/ugs/en-us/manual/authentication/manual/platform-signin-steam
    GetTicketForWebApiResponse_t data{};
    uint32 cbTicket = 0;
    Auth_Data ticket_data = getTicketData(data.m_rgubTicket, STEAM_AUTH_TICKET_SIZE, &cbTicket);
    data.m_cubTicket = (int)cbTicket;
    uint32 ttt = ticket_data.number;
    data.m_hAuthTicket = ttt;
    data.m_eResult = k_EResultOK;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), STEAM_TICKET_PROCESS_TIME);

    outbound.push_back(ticket_data);

    return ttt;
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
    return inbound.size();
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
            PRINT_DEBUG("TICKET DISCONNECT\n");
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
            PRINT_DEBUG("TICKET CANCEL " "%" PRIu64 "\n", msg->source_id());
            uint32 number = msg->auth_ticket().number();
            auto t = std::begin(inbound);
            while (t != std::end(inbound)) {
                if (t->id.ConvertToUint64() == msg->source_id() && t->number == number) {
                    PRINT_DEBUG("TICKET CANCELED\n");
                    launch_callback(t->id, k_EAuthSessionResponseAuthTicketCanceled);
                    t = inbound.erase(t);
                } else {
                    ++t;
                }
            }
        }
    }
}