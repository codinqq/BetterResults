#pragma once

#include "questui/shared/BeatSaberUI.hpp"
#include "questui/shared/QuestUI.hpp"
#include "UnityEngine/Color.hpp"
#include "questui/shared/CustomTypes/Components/ModalColorPicker.hpp"
#include "GlobalNamespace/ColorChangeUIEventType.hpp"

namespace BetterResults::UI {

    // Joinked from rxzz0 - https://github.com/rxzz0/CustomMissText/blob/main/src/UI/Custom/ConfigValueColorPickerModal.hpp
    // Let me know if you want this removed, rxzz0. I would 

    inline ::QuestUI::ModalColorPicker* AddConfigValueColorPickerModal(UnityEngine::Transform* parent, ConfigUtils::ConfigValue<::UnityEngine::Color>& configValue) {
    auto object = ::QuestUI::BeatSaberUI::CreateColorPickerModal(parent, configValue.GetName(), configValue.GetValue(), nullptr, nullptr, [&configValue](::UnityEngine::Color value) {
            configValue.SetValue(value);
        }
    );
    if(!configValue.GetHoverHint().empty())
        ::QuestUI::BeatSaberUI::AddHoverHint(object, configValue.GetHoverHint());
    return object;
}
}