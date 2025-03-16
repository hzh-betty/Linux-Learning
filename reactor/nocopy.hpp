#pragma once
class nocopy
{
public:
    nocopy() =default;
    nocopy(const nocopy&) = delete;
    nocopy& operator = (const nocopy&) = delete;
};