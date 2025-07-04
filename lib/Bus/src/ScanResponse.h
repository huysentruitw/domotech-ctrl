#pragma once

#include <ModuleType.h>

struct ScanResponse
{
    bool Success;
    bool RespondedWithTypeAndData = false;
    ::ModuleType ModuleType = ::ModuleType::Unknown;
    uint16_t Data = 0; // Only valid if RespondedWithTypeAndData is true
};
