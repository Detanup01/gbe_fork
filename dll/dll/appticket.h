
struct AppTicketV1
{
    // Total ticket size - 16
    uint32_t TicketSize;
    uint32_t TicketVersion;
    uint32_t Unk2;
    std::vector<uint8_t> UserData;

    void Reset()
    {
        TicketSize = 0;
        TicketVersion = 0;
        Unk2 = 0;
        UserData.clear();
    }

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;

        buffer.resize(16 + UserData.size());
        pBuffer = buffer.data();
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketSize;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketVersion;   pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = UserData.size(); pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = Unk2;            pBuffer += 4;
        memcpy(pBuffer, UserData.data(), UserData.size());

        return buffer;
    }

    bool Deserialize(const uint8_t* pBuffer, size_t size)
    {
        if (size < 16)
            return false;

        uint32_t user_data_size;

        TicketSize     = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        TicketVersion  = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        user_data_size = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;

        if (size < (user_data_size + 16))
            return false;

        Unk2 = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        UserData.resize(user_data_size);
        memcpy(UserData.data(), pBuffer, user_data_size);

        return true;
    }

    //inline uint32_t TicketSize()   { return *reinterpret_cast<uint32_t*>(_Buffer); }
    //inline uint32_t TicketVersion(){ return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 4); }
    //inline uint32_t UserDataSize() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 8); }
    //inline uint32_t Unk2()         { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 12); }
    //inline uint8_t* UserData()     { return  reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 16); }
};

struct AppTicketV2
{
    // Totals ticket size - 16 - TicketV1::UserData.size()
    uint32_t TicketSize;
    uint32_t TicketVersion;
    uint64_t SteamID;
    uint32_t AppID;
    uint32_t Unk1;
    uint32_t Unk2;
    uint32_t TicketFlags;
    uint32_t TicketIssueTime;
    uint32_t TicketValidityEnd;

    static constexpr uint32_t LicenseBorrowed = 0x00000002; // Bit 1: IsLicenseBorrowed
    static constexpr uint32_t LicenseTemporary = 0x00000004; // Bit 2: IsLicenseTemporary

    void Reset()
    {
        TicketSize = 0;
        TicketVersion = 0;
        SteamID = 0;
        AppID = 0;
        Unk1 = 0;
        Unk2 = 0;
        TicketFlags = 0;
        TicketIssueTime = 0;
        TicketValidityEnd = 0;
    }

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;

        buffer.resize(40);
        pBuffer = buffer.data();
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketSize;      pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketVersion;   pBuffer += 4;
        *reinterpret_cast<uint64_t*>(pBuffer) = SteamID;         pBuffer += 8;
        *reinterpret_cast<uint32_t*>(pBuffer) = AppID;           pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = Unk1;            pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = Unk2;            pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketFlags;     pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketIssueTime; pBuffer += 4;
        *reinterpret_cast<uint32_t*>(pBuffer) = TicketValidityEnd;

        return buffer;
    }

    bool Deserialize(const uint8_t* pBuffer, size_t size)
    {
        if (size < 40)
            return false;

        TicketSize        = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        TicketVersion     = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        SteamID           = *reinterpret_cast<const uint64_t*>(pBuffer); pBuffer += 8;
        AppID             = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        Unk1              = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        Unk2              = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        TicketFlags       = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        TicketIssueTime   = *reinterpret_cast<const uint32_t*>(pBuffer); pBuffer += 4;
        TicketValidityEnd = *reinterpret_cast<const uint32_t*>(pBuffer);

        return true;
    }

    //inline uint32_t TicketSize() { return *reinterpret_cast<uint32_t*>(_Buffer); }
    //inline uint32_t TicketVersion() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 4); }
    //inline uint64_t SteamID() { return *reinterpret_cast<uint64_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 8); };
    //inline uint32_t AppID() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 16); }
    //inline uint32_t Unk1() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 20); }
    //inline uint32_t Unk2() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 24); }
    //inline uint32_t TicketFlags() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 28); }
    //inline uint32_t TicketIssueTime() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 32); }
    //inline uint32_t TicketValidityEnd() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 36); }
};

struct AppTicketV4
{
    std::vector<uint32_t> AppIDs;

    bool HasVACStatus = false;
    uint32_t VACStatus;
    bool HasAppValue = false;
    uint32_t AppValue;

    void Reset()
    {
        AppIDs.clear();
        HasVACStatus = false;
        HasAppValue = false;
    }

    std::vector<uint8_t> Serialize()
    {
        std::vector<uint32_t> appids = AppIDs;
        if (appids.size() == 0)
        {
            appids.emplace_back(0);
        }

        uint16_t appid_count = static_cast<uint16_t>(appids.size() > 140 ? 140 : appids.size());
        size_t buffer_size = static_cast<uint32_t>(appid_count) * 4ul + 2ul;
        std::vector<uint8_t> buffer;
        uint8_t* pBuffer;

        if (HasAppValue)
        {// VACStatus + AppValue
            buffer_size += 4;
            if (!HasVACStatus)
            {
                HasVACStatus = true;
                VACStatus = 0;
            }
        }
        if (HasVACStatus)
        {// VACStatus only
            buffer_size += 4;
        }

        buffer.resize(buffer_size);
        pBuffer = buffer.data();
        *reinterpret_cast<uint16_t*>(pBuffer) = appid_count;
        pBuffer += 2;

        for (int i = 0; i < appid_count && i < 140; ++i)
        {
            *reinterpret_cast<uint32_t*>(pBuffer) = appids[i];
            pBuffer += 4;
        }

        if (HasVACStatus)
        {
            *reinterpret_cast<uint32_t*>(pBuffer) = VACStatus;
            pBuffer += 4;
        }
        if (HasAppValue)
        {
            *reinterpret_cast<uint32_t*>(pBuffer) = AppValue;
        }

        return buffer;
    }

    bool Deserialize(const uint8_t* pBuffer, size_t size)
    {
        if (size < 2)
            return false;

        uint16_t appid_count = *reinterpret_cast<const uint16_t*>(pBuffer);
        if (size < (appid_count * 4 + 2) || appid_count >= 140)
            return false;

        AppIDs.resize(appid_count);
        pBuffer += 2;
        size -= 2;
        for (int i = 0; i < appid_count; ++i)
        {
            AppIDs[i] = *reinterpret_cast<const uint32_t*>(pBuffer);
            pBuffer += 4;
            size -= 4;
        }

        HasVACStatus = false;
        HasAppValue = false;

        if (size < 4)
            return true;

        HasVACStatus = true;
        VACStatus = *reinterpret_cast<const uint32_t*>(pBuffer);
        pBuffer += 4;
        size -= 4;

        if (size < 4)
            return true;

        HasAppValue = true;
        AppValue = *reinterpret_cast<const uint32_t*>(pBuffer);

        return true;
    }

    // Often 1 with empty appid
    //inline uint16_t  AppIDCount() { return *reinterpret_cast<uint16_t*>(_Buffer); }
    //inline uint32_t* AppIDs() { return reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 2); }
    // Optional
    //inline uint32_t  VACStatus() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 2 + AppIDCount() * 4); }
    // Optional
    //inline uint32_t  AppValue() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 2 + AppIDCount() * 4 + 4); }
};

class DecryptedAppTicket
{
public:
    AppTicketV1 TicketV1;
    AppTicketV2 TicketV2;
    AppTicketV4 TicketV4;

    bool DeserializeTicket(const uint8_t* pBuffer, size_t buf_size)
    {
        if (!TicketV1.Deserialize(pBuffer, buf_size))
            return false;

        pBuffer += 16 + TicketV1.UserData.size();
        buf_size -= 16 + TicketV1.UserData.size();
        if (!TicketV2.Deserialize(pBuffer, buf_size))
            return false;

        if (TicketV2.TicketVersion > 2)
        {
            pBuffer += 40;
            buf_size -= 40;
            if (!TicketV4.Deserialize(pBuffer, buf_size))
                return false;
        }

        return true;
    }

    std::vector<uint8_t> SerializeTicket()
    {
        std::vector<uint8_t> buffer;

        TicketV1.TicketSize = TicketV1.UserData.size() + 40 + 2 + ((TicketV4.AppIDs.size() == 0 ? 1 : TicketV4.AppIDs.size()) * 4) + (TicketV4.HasVACStatus ? 4 : 0) + (TicketV4.HasAppValue ? 4 : 0);
        TicketV2.TicketSize = TicketV1.TicketSize - TicketV1.UserData.size();

        buffer = TicketV1.Serialize();

        auto v = TicketV2.Serialize();

        buffer.insert(buffer.end(), v.begin(), v.end());
        v = TicketV4.Serialize();
        buffer.insert(buffer.end(), v.begin(), v.end());

        return buffer;
    }
};