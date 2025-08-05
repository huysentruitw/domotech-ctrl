#pragma once

#include <IniWriter.h>
#include <Node.h>

class Filter : public Node
{
public:
    virtual const char* GetFilterName() const = 0;

    void WriteDescriptor(IniWriter& iniWriter)
    {
        iniWriter.WriteSection("Filter:" + std::string(GetFilterName()));

        const auto inputPins = GetInputPins();
        for (std::size_t i = 0; i < inputPins.size(); ++i) {
            if (auto sharedPin = inputPins[i].lock()) {
                const std::string stateTypeName = PinStateTypes[sharedPin->GetState().index()];
                iniWriter.WriteKeyValue("Input:" + std::to_string(i), sharedPin->GetName() + "," + stateTypeName);
            }
        }

        const auto outputPins = GetOutputPins();
        for (std::size_t i = 0; i < outputPins.size(); ++i) {
            if (auto sharedPin = outputPins[i].lock()) {
                const std::string stateTypeName = PinStateTypes[sharedPin->GetState().index()];
                iniWriter.WriteKeyValue("Output:" + std::to_string(i), sharedPin->GetName() + "," + stateTypeName);
            }
        }
    }
};
