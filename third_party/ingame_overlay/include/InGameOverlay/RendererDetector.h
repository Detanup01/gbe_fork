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

#include <future>
#include <chrono>
#include <memory>

#include "RendererHook.h"

namespace InGameOverlay {

/// <summary>
/// Starts a detector to automatically find the renderer used by the application.
/// </summary>
/// <param name="timeout">The time before the future will timeout if no renderer has been found.</param>
/// <param name="rendererToDetect">Set this to any combined RendererHookType_t value to filter the renderers you want to detect.</param>
/// <param name="preferSystemLibraries">Prefer hooking the system libraries instead of the first one found.</param>
/// <returns>A future nullptr or the renderer.</returns>
std::future<RendererHook_t*> DetectRenderer(std::chrono::milliseconds timeout = std::chrono::milliseconds{ -1 }, RendererHookType_t rendererToDetect = RendererHookType_t::Any, bool preferSystemLibraries = true);

/// <summary>
/// Stops the detector, the future will return as soon as possible.
/// </summary>
void StopRendererDetection();

/// <summary>
/// Free the detector allocated by DetectRenderer to you to restart detection, else, the same result will always return.
/// </summary>
void FreeDetector();

}