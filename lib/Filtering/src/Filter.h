#pragma once

#include "FilterType.h"

#include <IniWriter.h>
#include <Node.h>

class Filter : public Node
{
public:
    Filter(const FilterType type);

    FilterType GetType() const;

    void WriteDescriptor(IniWriter& iniWriter) const;

private:
    const FilterType m_type;    
};
