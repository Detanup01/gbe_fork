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


// this interface is not found in public SDK archives, it is based on reversing the returned vftable from steamclient64.dll
// real client returns 0/false in all these functions, the TODO notes are just for the names


#include "dll/steam_billing.h"


void Steam_Billing::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_Billing *)object;
    inst->steam_run_callback();
}

void Steam_Billing::steam_network_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_Billing *)object;
    inst->network_callback(msg);
}



Steam_Billing::Steam_Billing(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Billing::steam_network_callback, this);
    this->run_every_runcb->add(&Steam_Billing::steam_run_every_runcb, this);
}

Steam_Billing::~Steam_Billing()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Billing::steam_network_callback, this);
    this->run_every_runcb->remove(&Steam_Billing::steam_run_every_runcb, this);
}


bool Steam_Billing::_unknown_fn_1( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_2( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_3( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_4( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_5( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_6( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_7( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_8( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_9( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_10( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_11( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_12( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_13( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_14( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_15( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_16( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_17( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_18( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_19( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

int Steam_Billing::_unknown_fn_20( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_21( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_22( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_23( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_24( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_25( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_26( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

const char* Steam_Billing::_unknown_fn_27( ) // returns null string (str address is inside .rdata so it can't change at runtime)
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return "";
}

int Steam_Billing::_unknown_fn_28( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_29( ) // mov eax, 2
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_30( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_31( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_32( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_33( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_34( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_35( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_36( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_37( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

const char* Steam_Billing::_unknown_fn_38( ) // returns null string (str address is inside .rdata so it can't change at runtime)
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return "";
}

int Steam_Billing::_unknown_fn_39( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

int Steam_Billing::_unknown_fn_40( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return 0;
}

bool Steam_Billing::_unknown_fn_41( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_42( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}

bool Steam_Billing::_unknown_fn_43( )
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

    return false;
}



void Steam_Billing::steam_run_callback()
{

}



void Steam_Billing::network_callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}
