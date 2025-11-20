#pragma once

#include <stdexcept>
#include <windows.h> // For HRESULT

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // In a real engine, you'd want more detailed error reporting.
        // This could involve logging, formatting the HRESULT, etc.
        throw std::runtime_error("DirectX Error");
    }
}
