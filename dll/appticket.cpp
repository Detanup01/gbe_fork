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

#include "dll/appticket.h"


void AppTicketV1::Reset()
{
    TicketSize = 0;
    TicketVersion = 0;
    Unk2 = 0;
    UserData.clear();
}

std::vector<uint8_t> AppTicketV1::Serialize() const
{
    std::vector<uint8_t> buffer{};
    uint8_t* pBuffer{};

    buffer.resize(16 + UserData.size());
    pBuffer = buffer.data();
    *reinterpret_cast<uint32_t*>(pBuffer) = TicketSize;      pBuffer += 4;
    *reinterpret_cast<uint32_t*>(pBuffer) = TicketVersion;   pBuffer += 4;
    *reinterpret_cast<uint32_t*>(pBuffer) = static_cast<uint32_t>(UserData.size()); pBuffer += 4;
    *reinterpret_cast<uint32_t*>(pBuffer) = Unk2;            pBuffer += 4;
    memcpy(pBuffer, UserData.data(), UserData.size());

    return buffer;
}

bool AppTicketV1::Deserialize(const uint8_t* pBuffer, size_t size)
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



void AppTicketV2::Reset()
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

std::vector<uint8_t> AppTicketV2::Serialize() const
{
    std::vector<uint8_t> buffer{};
    uint8_t* pBuffer{};

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

bool AppTicketV2::Deserialize(const uint8_t* pBuffer, size_t size)
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



void AppTicketV4::Reset()
{
    AppIDs.clear();
    HasVACStatus = false;
    HasAppValue = false;
}

std::vector<uint8_t> AppTicketV4::Serialize()
{
    std::vector<uint32_t> appids = AppIDs;
    if (appids.size() == 0) {
        appids.emplace_back(0);
    }

    uint16_t appid_count = static_cast<uint16_t>(static_cast<uint16_t>(appids.size()) > 140 ? 140 : static_cast<uint16_t>(appids.size()));
    size_t buffer_size = static_cast<uint32_t>(appid_count) * 4ul + 2ul;
    std::vector<uint8_t> buffer{};
    uint8_t* pBuffer{};

    if (HasAppValue) {// VACStatus + AppValue
        buffer_size += 4;
        if (!HasVACStatus) {
            HasVACStatus = true;
            VACStatus = 0;
        }
    }

    if (HasVACStatus) {// VACStatus only
        buffer_size += 4;
    }

    buffer.resize(buffer_size);
    pBuffer = buffer.data();
    *reinterpret_cast<uint16_t*>(pBuffer) = appid_count;
    pBuffer += 2;

    for (int i = 0; i < appid_count && i < 140; ++i) {
        *reinterpret_cast<uint32_t*>(pBuffer) = appids[i];
        pBuffer += 4;
    }

    if (HasVACStatus) {
        *reinterpret_cast<uint32_t*>(pBuffer) = VACStatus;
        pBuffer += 4;
    }

    if (HasAppValue) {
        *reinterpret_cast<uint32_t*>(pBuffer) = AppValue;
    }

    return buffer;
}

bool AppTicketV4::Deserialize(const uint8_t* pBuffer, size_t size)
{
    if (size < 2)
        return false;

    uint16_t appid_count = *reinterpret_cast<const uint16_t*>(pBuffer);
    if (size < (appid_count * 4 + 2) || appid_count >= 140)
        return false;

    AppIDs.resize(appid_count);
    pBuffer += 2;
    size -= 2;
    for (int i = 0; i < appid_count; ++i) {
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



bool DecryptedAppTicket::DeserializeTicket(const uint8_t* pBuffer, size_t buf_size)
{
    if (!TicketV1.Deserialize(pBuffer, buf_size))
        return false;

    pBuffer += 16 + TicketV1.UserData.size();
    buf_size -= 16 + TicketV1.UserData.size();
    if (!TicketV2.Deserialize(pBuffer, buf_size))
        return false;

    if (TicketV2.TicketVersion > 2) {
        pBuffer += 40;
        buf_size -= 40;
        if (!TicketV4.Deserialize(pBuffer, buf_size))
            return false;
    }

    return true;
}

std::vector<uint8_t> DecryptedAppTicket::SerializeTicket()
{
    std::vector<uint8_t> buffer{};

    TicketV1.TicketSize = static_cast<uint32_t>(TicketV1.UserData.size()) + 40 + 2 + ((static_cast<uint32_t>(TicketV4.AppIDs.size()) == 0 ? 1 : static_cast<uint32_t>(TicketV4.AppIDs.size())) * 4) + (TicketV4.HasVACStatus ? 4 : 0) + (TicketV4.HasAppValue ? 4 : 0);
    TicketV2.TicketSize = TicketV1.TicketSize - static_cast<uint32_t>(TicketV1.UserData.size());

    buffer = TicketV1.Serialize();

    auto v = TicketV2.Serialize();

    buffer.insert(buffer.end(), v.begin(), v.end());
    v = TicketV4.Serialize();
    buffer.insert(buffer.end(), v.begin(), v.end());

    return buffer;
}



Steam_AppTicket::Steam_AppTicket(class Settings *settings) :
    settings(settings)
{

}

uint32 Steam_AppTicket::GetAppOwnershipTicketData( uint32 nAppID, void *pvBuffer, uint32 cbBufferLength, uint32 *piAppId, uint32 *piSteamId, uint32 *piSignature, uint32 *pcbSignature )
{
    PRINT_DEBUG("TODO %u, %p, %u, %p, %p, %p, %p", nAppID, pvBuffer, cbBufferLength, piAppId, piSteamId, piSignature, pcbSignature);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return 0;
}
