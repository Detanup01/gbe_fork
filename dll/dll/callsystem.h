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

#ifndef __INCLUDED_CALLSYSTEM_H__
#define __INCLUDED_CALLSYSTEM_H__

#include "common_includes.h"

#define DEFAULT_CB_TIMEOUT 0.002
#define STEAM_CALLRESULT_TIMEOUT 120.0
#define STEAM_CALLRESULT_WAIT_FOR_CB 0.01


class CCallbackMgr
{
public:
    static void SetRegister(class CCallbackBase *pCallback, int iCallback);
    static void SetUnregister(class CCallbackBase *pCallback);

    static bool isServer(class CCallbackBase *pCallback);
};

struct Steam_Call_Result {
    SteamAPICall_t api_call{};
    std::vector<class CCallbackBase *> callbacks{};
    std::vector<char> result{};
    bool to_delete = false;
    bool reserved = false;
    std::chrono::high_resolution_clock::time_point created{};
    double run_in{};
    bool run_call_completed_cb{};
    int iCallback{};

    Steam_Call_Result(SteamAPICall_t a, int icb, void *r, unsigned int s, double r_in, bool run_cc_cb);

    bool operator==(const struct Steam_Call_Result& other) const;

    bool timed_out() const;

    bool call_completed() const;

    bool can_execute() const;

    bool has_cb() const;

};

class SteamCallResults {
    std::vector<struct Steam_Call_Result> callresults{};
    std::vector<class CCallbackBase *> completed_callbacks{};
    void (*cb_all)(std::vector<char> result, int callback) = nullptr;

public:
    void addCallCompleted(class CCallbackBase *cb);

    void rmCallCompleted(class CCallbackBase *cb);

    void addCallBack(SteamAPICall_t api_call, class CCallbackBase *cb);

    bool exists(SteamAPICall_t api_call) const;

    bool callback_result(SteamAPICall_t api_call, void *copy_to, unsigned int size);

    void rmCallBack(SteamAPICall_t api_call, class CCallbackBase *cb);

    void rmCallBack(class CCallbackBase *cb);

    SteamAPICall_t addCallResult(SteamAPICall_t api_call, int iCallback, void *result, unsigned int size, double timeout=DEFAULT_CB_TIMEOUT, bool run_call_completed_cb=true);

    SteamAPICall_t reserveCallResult();

    SteamAPICall_t addCallResult(int iCallback, void *result, unsigned int size, double timeout=DEFAULT_CB_TIMEOUT, bool run_call_completed_cb=true);

    void setCbAll(void (*cb_all)(std::vector<char> result, int callback));

    void runCallResults();
};

struct Steam_Call_Back {
    std::vector<class CCallbackBase *> callbacks{};
    std::vector<std::vector<char>> results{};
};

class SteamCallBacks {
    std::map<int, struct Steam_Call_Back> callbacks{};
    SteamCallResults *results{};

public:
    SteamCallBacks(SteamCallResults *results);

    void addCallBack(int iCallback, class CCallbackBase *cb);
    void addCBResult(int iCallback, void *result, unsigned int size, double timeout, bool dont_post_if_already);
    void addCBResult(int iCallback, void *result, unsigned int size);
    void addCBResult(int iCallback, void *result, unsigned int size, bool dont_post_if_already);
    void addCBResult(int iCallback, void *result, unsigned int size, double timeout);

    void rmCallBack(int iCallback, class CCallbackBase *cb);

    void runCallBacks();
};

struct RunCBs {
    void (*function)(void *object) = nullptr;
    void *object{};
};

class RunEveryRunCB {
    std::vector<struct RunCBs> cbs{};

public:
    void add(void (*cb)(void *object), void *object);

    void remove(void (*cb)(void *object), void *object);

    void run() const;
};

#endif // __INCLUDED_CALLSYSTEM_H__
