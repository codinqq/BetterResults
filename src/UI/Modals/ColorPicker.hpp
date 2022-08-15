#pragma once

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "UnityEngine/Color.hpp"
#include "questui/shared/CustomTypes/Components/ModalColorPicker.hpp"



namespace BetterResults::UI {

    inline ::QuestUI::ModalColorPicker* AddConfigValueColorPickerModal(UnityEngine::Transform* transform, ConfigUtils::ConfigValue<::UnityEngine::Color>& configValue) {
        auto colorPicker = ::QuestUI::BeatSaberUI::CreateColorPickerModal(transform, configValue.GetName(), configValue.GetValue(), nullptr, nullptr, [&configValue](::UnityEngine::Color valueRGB) {
                configValue.SetValue(valueRGB);
            }
        );
        return colorPicker;
    }
}