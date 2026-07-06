#pragma once
#include <ztour/ztour.h>

// For dealing with relocating pieces of functions
namespace ztour::relocation {
    // Relocates if needed
    std::vector<uint8_t> relocate_code(const std::vector<uint8_t>& code_bytes, Ptr from_base, Ptr to_base);
}