#pragma once

#include "config-utils/shared/config-utils.hpp"
#include "UnityEngine/Color.hpp"
DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(titleColor, UnityEngine::Color, "Title Color", UnityEngine::Color(211 / 255, 211 / 255, 211 / 255 , 1));
    CONFIG_VALUE(valueColor, UnityEngine::Color, "Value Color", UnityEngine::Color(0 / 255, 175 / 255, 241 / 255 , 1));
    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(titleColor);
        CONFIG_INIT_VALUE(valueColor);
    )
)