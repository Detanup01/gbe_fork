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

#ifndef __INCLUDED_STEAM_BILLING_H__
#define __INCLUDED_STEAM_BILLING_H__

#include "base.h"

class Steam_Billing:
public ISteamBilling
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    

    static void steam_network_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

    void steam_run_callback();
    void network_callback(Common_Message *msg);

public:
    Steam_Billing(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Billing();

    bool _unknown_fn_1( );
    bool _unknown_fn_2( );
    bool _unknown_fn_3( );
    bool _unknown_fn_4( );
    bool _unknown_fn_5( );
    bool _unknown_fn_6( );
    bool _unknown_fn_7( );
    bool _unknown_fn_8( );
    bool _unknown_fn_9( );
    bool _unknown_fn_10( );
    bool _unknown_fn_11( );
    bool _unknown_fn_12( );
    bool _unknown_fn_13( );
    bool _unknown_fn_14( );
    bool _unknown_fn_15( );
    bool _unknown_fn_16( );
    bool _unknown_fn_17( );
    bool _unknown_fn_18( );
    bool _unknown_fn_19( );

    int _unknown_fn_20( );
    int _unknown_fn_21( );
    int _unknown_fn_22( );
    int _unknown_fn_23( );
    int _unknown_fn_24( );
    int _unknown_fn_25( );
    int _unknown_fn_26( );

    const char* _unknown_fn_27( ); // returns null string (str address is inside .rdata so it can't change at runtime)

    int _unknown_fn_28( );

    int _unknown_fn_29( ); // mov eax, 2

    int _unknown_fn_30( );
    int _unknown_fn_31( );
    int _unknown_fn_32( );
    int _unknown_fn_33( );
    int _unknown_fn_34( );
    int _unknown_fn_35( );
    int _unknown_fn_36( );
    int _unknown_fn_37( );

    const char* _unknown_fn_38( ); // returns null string (str address is inside .rdata so it can't change at runtime)

    int _unknown_fn_39( );
    int _unknown_fn_40( );

    bool _unknown_fn_41( );
    bool _unknown_fn_42( );
    bool _unknown_fn_43( );

};

#endif // __INCLUDED_STEAM_BILLING_H__
