#pragma once

#include <IniWriter.h>
#include <Node.h>

#include "FilterType.h"

#include <string>

class Filter : public Node
{
public:
    Filter(const FilterType type, std::string_view id) noexcept;
    virtual ~Filter() noexcept = default;

    FilterType GetType() const noexcept;

    void WriteDescriptor(IniWriter& iniWriter) const noexcept;
    void WriteConfig(IniWriter& iniWriter) const noexcept;

private:
    const FilterType m_type;
};
