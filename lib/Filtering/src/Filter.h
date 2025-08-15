#pragma once

#include <IniWriter.h>
#include <Node.h>

#include "FilterType.h"

class Filter : public Node
{
public:
    Filter(const FilterType type);

    FilterType GetType() const;

    void WriteDescriptor(IniWriter& iniWriter) const;
    void WriteConfig(IniWriter& iniWriter) const;

private:
    const FilterType m_type;
};
