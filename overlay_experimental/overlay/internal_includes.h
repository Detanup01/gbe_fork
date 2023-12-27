/*
 * Copyright (C) Nemirtingas
 * This file is part of the ingame overlay project
 *
 * The ingame overlay project is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * The ingame overlay project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the ingame overlay project; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <set>

#include "Base_Hook.h"
#include "Renderer_Hook.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)

#include <windows.h>
#ifdef GetModuleHandle
#undef GetModuleHandle
#endif

#endif

#ifdef USE_SPDLOG
#define SPDLOG_ACTIVE_LEVEL 0
#include <spdlog/spdlog.h>
#endif

#ifndef SPDLOG_TRACE
#define SPDLOG_TRACE(x, ...) PRINT_DEBUG(x "\n", __VA_ARGS__)
#endif
#ifndef SPDLOG_DEBUG
#define SPDLOG_DEBUG(x, ...) PRINT_DEBUG(x "\n", __VA_ARGS__)
#endif
#ifndef SPDLOG_INFO
#define SPDLOG_INFO(x, ...) PRINT_DEBUG(x "\n", __VA_ARGS__)
#endif
#ifndef SPDLOG_WARN
#define SPDLOG_WARN(x, ...) PRINT_DEBUG(x "\n", __VA_ARGS__)
#endif
#ifndef SPDLOG_ERROR
#define SPDLOG_ERROR(x, ...) PRINT_DEBUG(x "\n", __VA_ARGS__)
#endif
