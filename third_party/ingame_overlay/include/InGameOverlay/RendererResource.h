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

namespace InGameOverlay {

enum class ResourceAutoLoad_t : uint8_t
{
    None = 0,
    Batch = 1,
    OnUse = 2,
};

/// <summary>
/// A renderer resource. It will be tied to the RendererHook that created it. Don't use it if you recycle the renderer hook.
/// </summary>
class RendererResource_t
{
protected:
    virtual ~RendererResource_t() {}

public:
    /// <summary>
    /// Deletes the resource.
    /// </summary>
    virtual void Delete() = 0;
    /// <summary>
    /// Checks if the resource is loaded.
    /// </summary>
    /// <returns>Is loaded or not</returns>
    virtual bool IsLoaded() const = 0;
    /// <summary>
    /// Returns if the resource will be loaded on demand in a frame.
    /// </summary>
    /// <returns>Will it be loaded automatically if needed</returns>
    virtual ResourceAutoLoad_t AutoLoad() const = 0;
    /// <summary>
    /// Returns if the resource will be loaded on demand in a frame.
    /// </summary>
    virtual void SetAutoLoad(ResourceAutoLoad_t autoLoad) = 0;
    /// <summary>
    /// Returns if the resource has an image attached to it so it can be auto loaded.
    /// </summary>
    /// <returns>Can be loaded</returns>
    virtual bool CanBeLoaded() const = 0;
    /// <summary>
    /// Explicitly load the resource. (If not already loaded or if you changed its attachement)
    /// </summary>
    /// <returns>Is the resource ready on the GPU.</returns>
    virtual bool LoadAttachedResource() = 0;
    /// <summary>
    /// Explicitly load the resource and don't store the data internally. The resource will not be able to reload itself on renderer reset.
    /// </summary>
    /// <param name="data"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <returns></returns>
    virtual bool Load(const void* data, uint32_t width, uint32_t height) = 0;
    /// <summary>
    /// Gets the resource id usable by ImGui::Image(). It will also trigger the autoload if set.
    /// If autoload is in batch mode, the loading can be deferred by a few frames. (at least 1)
    /// </summary>
    /// <returns>The ImGui's image handle, 0 if it is not ready</returns>
    virtual uint64_t GetResourceId() = 0;
    /// <summary>
    ///   Return the loaded or attached resource width.
    /// </summary>
    /// <returns></returns>
    virtual uint32_t Width() const = 0;
    /// <summary>
    ///   Return the loaded or attached resource height.
    /// </summary>
    /// <returns></returns>
    virtual uint32_t Height() const = 0;
    /// <summary>
    /// Attach a resource to this RendererResource, it will NOT OWN the data.
    /// You are responsible to not outlive this object usage to the resource buffer.
    /// Attaching a new resource will trigger the autoload if it is enabled, else, the old resource will still be used until you unload it.
    /// </summary>
    /// <param name="data">The resource raw data (in RGBA format)</param>
    /// <param name="width">The resource width</param>
    /// <param name="height">The resource height</param>
    virtual void AttachResource(const void* data, uint32_t width, uint32_t height) = 0;
    /// <summary>
    /// Clears the attached resource. This will NOT delete the resource loaded onto the GPU. Call Unload for that purpose.
    /// </summary>
    virtual void ClearAttachedResource() = 0;
    /// <summary>
    /// Unloads the resource from the GPU. GetResourceId will return an invalid handle, IsLoaded will return false.
    /// If auto loading is enabled, it will load again the resource if its not cleared.
    /// </summary>
    virtual void Unload(bool clearAttachedResource = true) = 0;
};

}