#pragma once

#include <cstdint>

using ShaderHandle = uint8_t;

constexpr ShaderHandle InvalidShaderHandle = static_cast<ShaderHandle>(-1);
