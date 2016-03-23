#pragma once

#include "pch.hpp"

//A handle to an object. Allows for relocation. Handle typically point to an object in a scope
template <typename T>
class Handle
{
public:
    Handle(T** Location) : location(Location) { }

protected:
    T** location;
};
