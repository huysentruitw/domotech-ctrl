#pragma once

#include <stdint.h>
#include <BusProtocol.h>
#include <Node.h>
#include "ModuleType.h"

class ModuleBase : public Node
{
public:
    ModuleBase(const uint8_t address, const ModuleType moduleType);

    virtual bool Process(const BusProtocol& bus) const = 0;

    uint8_t GetAddress() const;
    ModuleType GetType() const;

protected:
    ScanResponse Poll(const BusProtocol& bus) const;
    ScanResponse Exchange(const BusProtocol& bus, const uint16_t data) const;

private:
    const uint8_t address;
    const ModuleType type;
};
