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

#ifndef BASE_INCLUDE
#define BASE_INCLUDE

#include "common_includes.h"

#define PUSH_BACK_IF_NOT_IN(vector, element) { if(std::find(vector.begin(), vector.end(), element) == vector.end()) vector.push_back(element); }

extern std::recursive_mutex global_mutex;
extern const std::chrono::time_point<std::chrono::high_resolution_clock> startup_counter;
extern const std::chrono::time_point<std::chrono::system_clock> startup_time;

void randombytes(char *buf, size_t size);
std::string get_env_variable(std::string name);
bool set_env_variable(std::string name, std::string value);
bool check_timedout(std::chrono::high_resolution_clock::time_point old, double timeout);

class CCallbackMgr
{
public:
    static void SetRegister(class CCallbackBase *pCallback, int iCallback) {
        pCallback->m_nCallbackFlags |= CCallbackBase::k_ECallbackFlagsRegistered;
        pCallback->m_iCallback = iCallback;
    };

    static void SetUnregister(class CCallbackBase *pCallback) {
        if (pCallback)
            pCallback->m_nCallbackFlags &= !CCallbackBase::k_ECallbackFlagsRegistered;
    };

    static bool isServer(class CCallbackBase *pCallback) {
        return (pCallback->m_nCallbackFlags & CCallbackBase::k_ECallbackFlagsGameServer) != 0;
    };
};

#define STEAM_CALLRESULT_TIMEOUT 120.0
#define STEAM_CALLRESULT_WAIT_FOR_CB 0.01
struct Steam_Call_Result {
    Steam_Call_Result(SteamAPICall_t a, int icb, void *r, unsigned int s, double r_in, bool run_cc_cb) {
        api_call = a;
        result.resize(s);
        if (s > 0 && r != NULL)
            memcpy(&(result[0]), r, s);
        created = std::chrono::high_resolution_clock::now();
        run_in = r_in;
        run_call_completed_cb = run_cc_cb;
        iCallback = icb;
    }

    bool operator==(const struct Steam_Call_Result& a)
    {
        return a.api_call == api_call && a.callbacks == callbacks;
    }

    bool timed_out() {
        return check_timedout(created, STEAM_CALLRESULT_TIMEOUT);
    }

    bool call_completed() {
        return (!reserved) && check_timedout(created, run_in);
    }

    bool can_execute() {
        return (!to_delete) && call_completed() && (has_cb() || check_timedout(created, STEAM_CALLRESULT_WAIT_FOR_CB));
    }

    bool has_cb() {
        return callbacks.size() > 0;
    }

    SteamAPICall_t api_call;
    std::vector<class CCallbackBase *> callbacks;
    std::vector<char> result;
    bool to_delete = false;
    bool reserved = false;
    std::chrono::high_resolution_clock::time_point created;
    double run_in;
    bool run_call_completed_cb;
    int iCallback;
};

unsigned generate_account_id();
CSteamID generate_steam_anon_user();
SteamAPICall_t generate_steam_api_call_id();
CSteamID generate_steam_id_user();
CSteamID generate_steam_id_server();
CSteamID generate_steam_id_anonserver();
CSteamID generate_steam_id_lobby();
std::string get_full_lib_path();
std::string get_full_program_path();
std::string get_current_path();
std::string canonical_path(std::string path);
bool file_exists_(std::string full_path);
unsigned int file_size_(std::string full_path);

#define DEFAULT_CB_TIMEOUT 0.002

class SteamCallResults {
    std::vector<struct Steam_Call_Result> callresults;
    std::vector<class CCallbackBase *> completed_callbacks;
    void (*cb_all)(std::vector<char> result, int callback) = nullptr;

public:
    void addCallCompleted(class CCallbackBase *cb) {
        if (std::find(completed_callbacks.begin(), completed_callbacks.end(), cb) == completed_callbacks.end()) {
            completed_callbacks.push_back(cb);
        }
    }

    void rmCallCompleted(class CCallbackBase *cb) {
        auto c = std::find(completed_callbacks.begin(), completed_callbacks.end(), cb);
        if (c != completed_callbacks.end()) {
            completed_callbacks.erase(c);
        }
    }

    void addCallBack(SteamAPICall_t api_call, class CCallbackBase *cb) {
        auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct Steam_Call_Result const& item) { return item.api_call == api_call; });
        if (cb_result != callresults.end()) {
            cb_result->callbacks.push_back(cb);
            CCallbackMgr::SetRegister(cb, cb->GetICallback());
        }
    }

    bool exists(SteamAPICall_t api_call) {
        auto cr = std::find_if(callresults.begin(), callresults.end(), [api_call](struct Steam_Call_Result const& item) { return item.api_call == api_call; });
        if (cr == callresults.end()) return false;
        if (!cr->call_completed()) return false;
        return true;
    }

    bool callback_result(SteamAPICall_t api_call, void *copy_to, unsigned int size) {
        auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct Steam_Call_Result const& item) { return item.api_call == api_call; });
        if (cb_result != callresults.end()) {
            if (!cb_result->call_completed()) return false;
            if (cb_result->result.size() > size) return false;

            memcpy(copy_to, &(cb_result->result[0]), cb_result->result.size());
            cb_result->to_delete = true;
            return true;
        } else {
            return false;
        }
    }

    void rmCallBack(SteamAPICall_t api_call, class CCallbackBase *cb) {
        auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct Steam_Call_Result const& item) { return item.api_call == api_call; });
        if (cb_result != callresults.end()) {
            auto it = std::find(cb_result->callbacks.begin(), cb_result->callbacks.end(), cb);
            if (it != cb_result->callbacks.end()) {
                cb_result->callbacks.erase(it);
                CCallbackMgr::SetUnregister(cb);
            }
        }
    }

    void rmCallBack(class CCallbackBase *cb) {
        //TODO: check if callback is callback or call result?
        for (auto & cr: callresults) {
            auto it = std::find(cr.callbacks.begin(), cr.callbacks.end(), cb);
            if (it != cr.callbacks.end()) {
                cr.callbacks.erase(it);
            }

            if (cr.callbacks.size() == 0) {
                cr.to_delete = true;
            }
        }
    }

    SteamAPICall_t addCallResult(SteamAPICall_t api_call, int iCallback, void *result, unsigned int size, double timeout=DEFAULT_CB_TIMEOUT, bool run_call_completed_cb=true) {
        auto cb_result = std::find_if(callresults.begin(), callresults.end(), [api_call](struct Steam_Call_Result const& item) { return item.api_call == api_call; });
        if (cb_result != callresults.end()) {
            if (cb_result->reserved) {
                std::chrono::high_resolution_clock::time_point created = cb_result->created;
                std::vector<class CCallbackBase *> temp_cbs = cb_result->callbacks;
                *cb_result = Steam_Call_Result(api_call, iCallback, result, size, timeout, run_call_completed_cb);
                cb_result->callbacks = temp_cbs;
                cb_result->created = created;
                return cb_result->api_call;
            }
        } else {
            struct Steam_Call_Result res = Steam_Call_Result(api_call, iCallback, result, size, timeout, run_call_completed_cb);
            callresults.push_back(res);
            return callresults.back().api_call;
        }

        PRINT_DEBUG("addCallResult ERROR\n");
        return 0;
    }

    SteamAPICall_t reserveCallResult() {
        struct Steam_Call_Result res = Steam_Call_Result(generate_steam_api_call_id(), 0, NULL, 0, 0.0, true);
        res.reserved = true;
        callresults.push_back(res);
        return callresults.back().api_call;
    }

    SteamAPICall_t addCallResult(int iCallback, void *result, unsigned int size, double timeout=DEFAULT_CB_TIMEOUT, bool run_call_completed_cb=true) {
        return addCallResult(generate_steam_api_call_id(), iCallback, result, size, timeout, run_call_completed_cb);
    }

    void setCbAll(void (*cb_all)(std::vector<char> result, int callback)) {
        this->cb_all = cb_all;
    }

    void runCallResults() {
        unsigned long current_size = callresults.size();
        for (unsigned i = 0; i < current_size; ++i) {
            unsigned index = i;

            if (!callresults[index].to_delete) {
                if (callresults[index].can_execute()) {
                    std::vector<char> result = callresults[index].result;
                    SteamAPICall_t api_call = callresults[index].api_call;
                    bool run_call_completed_cb = callresults[index].run_call_completed_cb;
                    int iCallback = callresults[index].iCallback;
                    if (run_call_completed_cb) {
                        callresults[index].run_call_completed_cb = false;
                    }

                    callresults[index].to_delete = true;
                    if (callresults[index].has_cb()) {
                        std::vector<class CCallbackBase *> temp_cbs = callresults[index].callbacks;
                        for (auto & cb : temp_cbs) {
                            PRINT_DEBUG("Calling callresult %p %i\n", cb, cb->GetICallback());
                            global_mutex.unlock();
                            //TODO: unlock relock doesn't work if mutex was locked more than once.
                            if (run_call_completed_cb) { //run the right function depending on if it's a callback or a call result.
                                cb->Run(&(result[0]), false, api_call);
                            } else {
                                cb->Run(&(result[0]));
                            }
                            //COULD BE DELETED SO DON'T TOUCH CB
                            global_mutex.lock();
                            PRINT_DEBUG("callresult done\n");
                        }
                    }

                    if (run_call_completed_cb) {
                        //can it happen that one is removed during the callback?
                        std::vector<class CCallbackBase *> callbacks = completed_callbacks;
                        SteamAPICallCompleted_t data;
                        data.m_hAsyncCall = api_call;
                        data.m_iCallback = iCallback;
                        data.m_cubParam = result.size();

                        for (auto & cb: callbacks) {
                            PRINT_DEBUG("Call complete cb %i %p %llu\n", iCallback, cb, api_call);
                            //TODO: check if this is a problem or not.
                            SteamAPICallCompleted_t temp = data;
                            global_mutex.unlock();
                            cb->Run(&temp);
                            global_mutex.lock();
                        }

                        if (cb_all) {
                            std::vector<char> res;
                            res.resize(sizeof(data));
                            memcpy(&(res[0]), &data, sizeof(data));
                            cb_all(res, data.k_iCallback);
                        }
                    } else {
                        if (cb_all) {
                            cb_all(result, iCallback);
                        }
                    }
                } else {
                    if (callresults[index].timed_out()) {
                        callresults[index].to_delete = true;
                    }
                }
            }
        }

        PRINT_DEBUG("runCallResults erase to_delete\n");
        auto c = std::begin(callresults);
        while (c != std::end(callresults)) {
            if (c->to_delete) {
                if (c->timed_out()) {
                    c = callresults.erase(c);
                } else {
                    ++c;
                }
            } else {
                ++c;
            }
        }
    }
};

struct Steam_Call_Back {
    std::vector<class CCallbackBase *> callbacks;
    std::vector<std::vector<char>> results;
};

class SteamCallBacks {
    std::map<int, struct Steam_Call_Back> callbacks;
    SteamCallResults *results;
public:
    SteamCallBacks(SteamCallResults *results) {
        this->results = results;
    }

    void addCallBack(int iCallback, class CCallbackBase *cb) {
        PRINT_DEBUG("addCallBack %i\n", iCallback);
        if (iCallback == SteamAPICallCompleted_t::k_iCallback) {
            results->addCallCompleted(cb);
            CCallbackMgr::SetRegister(cb, iCallback);
            return;
        }

        if (std::find(callbacks[iCallback].callbacks.begin(), callbacks[iCallback].callbacks.end(), cb) == callbacks[iCallback].callbacks.end()) {
            callbacks[iCallback].callbacks.push_back(cb);
            CCallbackMgr::SetRegister(cb, iCallback);
            for (auto & res: callbacks[iCallback].results) {
                //TODO: timeout?
                SteamAPICall_t api_id = results->addCallResult(iCallback, &(res[0]), res.size(), 0.0, false);
                results->addCallBack(api_id, cb);
            }
        }
    }

    void addCBResult(int iCallback, void *result, unsigned int size, double timeout, bool dont_post_if_already) {
        if (dont_post_if_already) {
            for (auto & r : callbacks[iCallback].results) {
                if (r.size() == size) {
                    if (memcmp(&(r[0]), result, size) == 0) {
                        //cb already posted
                        return;
                    }
                }
            }
        }

        std::vector<char> temp;
        temp.resize(size);
        memcpy(&(temp[0]), result, size);
        callbacks[iCallback].results.push_back(temp);
        for (auto cb: callbacks[iCallback].callbacks) {
            SteamAPICall_t api_id = results->addCallResult(iCallback, result, size, timeout, false);
            results->addCallBack(api_id, cb);
        }

        if (callbacks[iCallback].callbacks.empty()) {
            results->addCallResult(iCallback, result, size, timeout, false);
        }
    }

    void addCBResult(int iCallback, void *result, unsigned int size) {
        addCBResult(iCallback, result, size, DEFAULT_CB_TIMEOUT, false);
    }

    void addCBResult(int iCallback, void *result, unsigned int size, bool dont_post_if_already) {
        addCBResult(iCallback, result, size, DEFAULT_CB_TIMEOUT, dont_post_if_already);
    }

    void addCBResult(int iCallback, void *result, unsigned int size, double timeout) {
        addCBResult(iCallback, result, size, timeout, false);
    }

    void rmCallBack(int iCallback, class CCallbackBase *cb) {
        if (iCallback == SteamAPICallCompleted_t::k_iCallback) {
            results->rmCallCompleted(cb);
            CCallbackMgr::SetUnregister(cb);
            return;
        }

        auto c = std::find(callbacks[iCallback].callbacks.begin(), callbacks[iCallback].callbacks.end(), cb);
        if (c != callbacks[iCallback].callbacks.end()) {
            callbacks[iCallback].callbacks.erase(c);
            CCallbackMgr::SetUnregister(cb);
            results->rmCallBack(cb);
        }
    }

    void runCallBacks() {
        for (auto & c : callbacks) {
            c.second.results.clear();
        }
    }
};

struct RunCBs {
    void (*function)(void *object);
    void *object;
};

class RunEveryRunCB {
    std::vector<struct RunCBs> cbs;
public:
    void add(void (*cb)(void *object), void *object) {
        remove(cb, object);
        RunCBs rcb;
        rcb.function = cb;
        rcb.object = object;
        cbs.push_back(rcb);
    }

    void remove(void (*cb)(void *object), void *object) {
        auto c = std::begin(cbs);
        while (c != std::end(cbs)) {
            if (c->function == cb && c->object == object) {
                c = cbs.erase(c);
            } else {
                ++c;
            }
        }
    }

    void run() {
        std::vector<struct RunCBs> temp_cbs = cbs;
        for (auto c : temp_cbs) {
            c.function(c.object);
        }
    }
};

void set_whitelist_ips(uint32_t *from, uint32_t *to, unsigned num_ips);
#ifdef EMU_EXPERIMENTAL_BUILD
bool crack_SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID);
bool crack_SteamAPI_Init();
#endif

#endif
