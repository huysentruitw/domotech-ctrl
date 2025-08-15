#pragma once

#include <cstdint>
#include <string>

#include <Bus.h>
#include <IniWriter.h>
#include <Node.h>

#include "ModuleType.h"

struct ProcessResponse
{
    bool Success;
    bool RaisePriority = false;
};

class Module : public Node
{
public:
    Module(const Bus& bus, const uint8_t address, const ModuleType moduleType);

    virtual ProcessResponse Process() = 0;

    uint8_t GetAddress() const;
    ModuleType GetType() const;

    void WriteConfig(IniWriter& iniWriter) const;

protected:
    ScanResponse Poll() const;
    ScanResponse Exchange(const uint16_t data) const;

private:
    const Bus& m_bus;
    const uint8_t m_address;
    const ModuleType m_type;
};
