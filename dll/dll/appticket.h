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

#ifndef __INCLUDED_STEAM_APP_TICKET_H__
#define __INCLUDED_STEAM_APP_TICKET_H__

#include "base.h"
#include "steam/isteamappticket.h"

struct AppTicketV1
{
    // Total ticket size - 16
    uint32_t TicketSize{};
    uint32_t TicketVersion{};
    uint32_t Unk2{};
    std::vector<uint8_t> UserData{};

    void Reset();

    std::vector<uint8_t> Serialize() const;

    bool Deserialize(const uint8_t* pBuffer, size_t size);

    //inline uint32_t TicketSize()   { return *reinterpret_cast<uint32_t*>(_Buffer); }
    //inline uint32_t TicketVersion(){ return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 4); }
    //inline uint32_t UserDataSize() { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 8); }
    //inline uint32_t Unk2()         { return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 12); }
    //inline uint8_t* UserData()     { return  reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(_Buffer) + 16); }
};

struct AppTicketV2
{
    // Totals ticket size - 16 - TicketV1::UserData.size()
    uint32_t TicketSize{};
    uint32_t TicketVersion{};
    uint64_t SteamID{};
    uint32_t AppID{};
    uint32_t Unk1{};
    uint32_t Unk2{};
    uint32_t TicketFlags{};
    uint32_t TicketIssueTime{};
    uint32_t TicketValidityEnd{};

    static constexpr const uint32_t LicenseBorrowed = 0x00000002; // Bit 1: IsLicenseBorrowed
    static constexpr const uint32_t LicenseTemporary = 0x00000004; // Bit 2: IsLicenseTemporary

    void Reset();

    std::vector<uint8_t> Serialize() const;

    bool Deserialize(const uint8_t* pBuffer, size_t size);

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
    std::vector<uint32_t> AppIDs{};

    bool HasVACStatus = false;
    uint32_t VACStatus{};
    bool HasAppValue = false;
    uint32_t AppValue{};

    void Reset();

    std::vector<uint8_t> Serialize();

    bool Deserialize(const uint8_t* pBuffer, size_t size);

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
    AppTicketV1 TicketV1{};
    AppTicketV2 TicketV2{};
    AppTicketV4 TicketV4{};

    bool DeserializeTicket(const uint8_t* pBuffer, size_t buf_size);

    std::vector<uint8_t> SerializeTicket();
};


class Steam_AppTicket :
public ISteamAppTicket
{
private:
    class Settings *settings{};

public:
    Steam_AppTicket(class Settings *settings);

    virtual uint32 GetAppOwnershipTicketData( uint32 nAppID, void *pvBuffer, uint32 cbBufferLength, uint32 *piAppId, uint32 *piSteamId, uint32 *piSignature, uint32 *pcbSignature );

};

#endif // __INCLUDED_STEAM_APP_TICKET_H__
