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

#include "dll/steam_client.h"
#include "dll/dll.h"

void Client_Background_Thread::thread_proc()
{
    // wait for some time
    {

        std::unique_lock lck(kill_background_thread_mutex);
        if (kill_background_thread_cv.wait_for(lck, initial_delay, [&] { return kill_background_thread; })) {
            PRINT_DEBUG("early exit");
            return;
        }
    }

    PRINT_DEBUG("starting");

    while (1) {
        {
            std::unique_lock lck(kill_background_thread_mutex);
            if (kill_background_thread_cv.wait_for(lck, max_stall_ms, [&] { return kill_background_thread; })) {
                PRINT_DEBUG("exit");
                return;
            }
        }

        auto now_ms = (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

        // if our time exceeds last run time of callbacks and it wasn't processing already
        const auto runcallbacks_timeout_ms = client_instance->get_last_runcallbacks_time() + max_stall_ms.count();
        if (!client_instance->runcallbacks_active() && (now_ms >= runcallbacks_timeout_ms)) {
            std::lock_guard lock(global_mutex);
            PRINT_DEBUG("run @@@@@@@@@@@@@@@@@@@@@@@@@@@");
            client_instance->set_last_runcallbacks_time(now_ms); // update the time counter just to avoid overlap
            client_instance->network->Run(); // networking must run first since it receives messages used by each run_callback()
            client_instance->run_every_runcb->run(); // call each run_callback()
        }
    }
}


Client_Background_Thread::Client_Background_Thread()
{

}

Client_Background_Thread::~Client_Background_Thread()
{
    kill();
}

void Client_Background_Thread::start(Steam_Client *client_instance)
{
    if (background_keepalive.joinable()) return; // alrady spawned
    
    this->client_instance = client_instance;
    background_keepalive = std::thread([this] { thread_proc(); });
    PRINT_DEBUG("spawned background thread *********");
}

void Client_Background_Thread::kill()
{
    if (!background_keepalive.joinable()) return; // already killed
    
    {
        std::lock_guard lk(kill_background_thread_mutex);
        kill_background_thread = true;
    }
    kill_background_thread_cv.notify_one();
    PRINT_DEBUG("joining worker thread");
    background_keepalive.join();
    PRINT_DEBUG("worker thread killed");
}
