#pragma once

#include <stdint.h>

// Standardize ImTextureID as uint64_t to match most modern renderers (Vulkan/DX12/etc.)
// handles and avoid mangling mismatches when used as an external library.
#define ImTextureID uint64_t
