#pragma once

#include "pch.hpp"

namespace Plang
{
    struct Location
    {
        std::string module;
        size_t line = 0;
        size_t column = 0;

        inline bool operator != (const Location& other) const
        {
            return (module != other.module ||
                    line != other.line ||
                    column != other.column);
        }

        inline operator std::string() const
        {
            return module + ":" + std::to_string(line) + "," + std::to_string(column);
        }
    };

    inline std::ostream& operator << (std::ostream& stream, const Location& location)
    {
        stream << (location.module.empty() ? "(no-module)" : location.module);
        stream << std::dec << std::noshowbase << ":";
        stream << location.line << ",";
        stream << location.column;
        return stream;
    }
}
