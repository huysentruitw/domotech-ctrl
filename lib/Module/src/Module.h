#pragma once

#include <Bus.h>
#include <IniWriter.h>
#include <Node.h>

#include "ModuleType.h"

#include <cstdint>
#include <string>

struct ProcessResponse
{
    bool Success;
    bool RaisePriority = false;
};

class Module : public Node
{
public:
    Module(const Bus& bus, const uint8_t address, const ModuleType moduleType) noexcept;

    virtual ProcessResponse Process() noexcept = 0;

    uint8_t GetAddress() const noexcept;
    ModuleType GetType() const noexcept;

    void WriteConfig(IniWriter& iniWriter) const noexcept;

protected:
    ScanResponse Exchange(const uint16_t data, const bool forceDataExchange) const noexcept;

private:
    const Bus& m_bus;
    const uint8_t m_address;
    const ModuleType m_type;
};
