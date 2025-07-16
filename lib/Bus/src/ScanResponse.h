#pragma once

#include <stdint.h>

struct ScanResponse
{
    bool Success;
    bool RespondedWithTypeAndData = false;
    uint8_t ModuleType = 0; // Only valid if RespondedWithTypeAndData is true
    uint16_t Data = 0; // Only valid if RespondedWithTypeAndData is true
};
