#pragma once

#include <Module.h>

class MockModule : public Module
{
public:
    MockModule(const Bus& bus, ModuleType type, uint8_t address, uint16_t initialData)
        : Module(bus, address, type), m_initialData(initialData)
    {
    }

    uint16_t GenerateInitialData() const noexcept override { return m_initialData; }
    ProcessResponse Process() noexcept override { return {true}; }

private:
    uint16_t m_initialData;
};
