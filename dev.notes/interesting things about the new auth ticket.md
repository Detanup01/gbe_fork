# Interesting things about the new auth ticket
Firstly, why do you need to use the new auth ticket?

Well, thats because some Emulators, or servers checking inside the tickets. (Example is Nemirtingas Epic Emu)\
Old version of the ticket not gonna work with them.

## How does the old ticket look vs the new?

Old Ticket looks like this:
```
4 byte (header) | 4 byte | 8 byte
0x14 (AKA 20) 00 00 00 | [TicketNumber] | [SteamId]
```
As you see the ticket doesnt contains any information when its made, what DLC you have and appid you started.

### Before seeing how the new ticket looks, what does the "HasGC" means?

GC means Game Coordinator.\
It helps with IP address, better matchmake, and other things.

Why do we use it?\
Well simple because I researched for it and everything usually sending that data back.\
You can write a simple Application and edit steam_api.txt for any appid you own and gather the ticket from it.

GC contains these infromation:
```c++
uint32_t STEAM_APPTICKET_GCLen = 20; // Magic header 20
uint64_t GCToken{}; // A unique token for this, can be random or sequential
CSteamID id{}; // our steamId
uint32_t ticketGenDate{}; //epoch time when generated
uint32_t STEAM_APPTICKET_SESSIONLEN = 24; // Magic Header 24
uint32_t one = 1; // dont know yet
uint32_t two = 2; // dont know yet
uint32_t ExternalIP{}; // External ip (Steam usually encrypting these)
uint32_t InternalIP{}; // Internal ip (Steam usually encrypting these)
uint32_t TimeSinceStartup{}; // Seconds since Steam Startup
uint32_t TicketGeneratedCount{}; // how many ticket did you generated since startup
uint32_t FullSizeOfGC = 56; // GC size (52) + 4
```

If you add those together you get 52

```
8 = uint64_t
4 = uint32_t
4 + 8 + 8 + 4 = 24 (4 without the header is 20 so the lenght of the Next section)

4 + 4 + 4 + 4 + 4 + 4 + 4 = 28 (4 without the header is 24 so the lenght of the Next section)
```

Yes, we could separate these but since only GC doing this, that is not much

### The rest of the Ticket
As you see in the auth.h file the ticket is contains these infromation:

```c++
uint32_t TheTicketLenght; // Full lenght of the ticket exluding the padding and the Singature
uint32_t Version{}; // Latest version is 4 so we keep that way
CSteamID id{}; // our steamId
uint32_t AppId{}; // Current AppId that we playing
uint32_t ExternalIP{}; // External ip (Steam usually encrypting these)
uint32_t InternalIP{}; // Internal ip (Steam usually encrypting these)
uint32_t AlwaysZero = 0; //OwnershipFlags? or Might be VAC Banned?
uint32_t TicketGeneratedDate{}; // Epoch Seconds when the Ticket generated
uint32_t TicketGeneratedExpireDate{}; // Epoch Seconds when the Ticket will expire
std::vector<uint32_t> Licenses{}; // our licenses (Usually is 0 or if you own a locked beta that will be it)
std::vector<DLC> DLCs{}; // what DLC we own
```

The DLC data inside:
```c++
struct DLC {
    uint32_t AppId{}; // AppId of the DLC
    std::vector<uint32_t> Licenses{}; // Again license what you own, usually 0 or nothing inside
};
```

The Licenses:\
All app if not relesed to public is behind a license, steam usually set (or returns) 0 as if you own it or doesnt have any license to it.\
IT DOES not mean the app is free, even if you bought it still shown as 0!

### Signature and padding.

I dont know why steam has a 2 byte for a padding but that could be something or a random value.\
OR that could be if we got banend by VAC? I dont know yet.

Steam has a signature, as I seen its a 128 lenght one. I choosen RSA1 and PKCS1 since it giving me that one.\
I generated a key (You can get yourself here: https://github.com/Detanup01/stmsrv/blob/main/Cert/AppTicket.key) or from Auth.cpp/h file.

It is just we get the ticket data as bytes and we sign it with our key, and vola we have a ticket!

Thats why the NEW size is Minimum 170 because 128 + 42 (Minimum Ticket Data without any DLC, License, and GC)

## Interesting things

The Ticket can exceed 1024 byte if user own soo many DLC. Steam recommend setting as 1024 but I recommend everyone using 2048 if you have a Game that has many DLC. (PayDay 2)

Old ticket is similar to the start of our GC ticket. 

Currently SendUserConnectAndAuthenticate, beginAuth "does not" have code for supporting NEW AuthTicket. But because the Old ticket header is similar to GC which we do send data with my steamId and a random Id. It doesnt need to Deserialize anything from the ticket.