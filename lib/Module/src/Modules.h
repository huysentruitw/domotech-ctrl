#pragma once

#include "ModuleBase.h"

class PushButtonModule final : public ModuleBase
{
public:
    PushButtonModule(const uint8_t address, const uint16_t initialData);

    bool Process(const BusProtocol& bus) const override;

private:
    const uint8_t numberOfButtons;
};

class RelaisPulseModule final : public ModuleBase
{
public:
    RelaisPulseModule(const uint8_t address, const uint16_t initialData);

    bool Process(const BusProtocol& bus) const override;

private:
    const uint8_t numberOfRelais;
};
