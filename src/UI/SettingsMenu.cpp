#include "UI/SettingsMenu.hpp"

#include "main.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace QuestUI;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI::BeatSaberUI;

#include "UnityEngine/GameObject.hpp"
using namespace UnityEngine;

#include "ModConfig.hpp"

#include <string>
#include <iostream>
#include <sstream>

#include "UI/Modals/ColorPicker.hpp"


DEFINE_TYPE(BetterResults::UI::Settings, SettingsMenu);

void BetterResults::UI::Settings::SettingsMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {

    if(firstActivation){
    
        UnityEngine::GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(get_transform());
        
        Color titleColor = getModConfig().titleColor.GetValue();
        auto titleObjColorSelector = AddConfigValueColorPickerModal(container->get_transform(), getModConfig().titleColor);
        QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Title Color", [=] {
            titleObjColorSelector->Show();
        });

        Color valueColor = getModConfig().valueColor.GetValue();
        auto valueObjColorSelector = AddConfigValueColorPickerModal(container->get_transform(), getModConfig().valueColor);


        QuestUI::BeatSaberUI::CreateUIButton(container->get_transform(), "Value Color", [=] {
            valueObjColorSelector->Show();
        });

    }

}