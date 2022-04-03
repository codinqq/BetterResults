#pragma once
#include "HMUI/ViewController.hpp"
#include "custom-types/shared/macros.hpp"

DECLARE_CLASS_CODEGEN(BetterResults::UI::Settings, SettingsMenu, HMUI::ViewController,
    DECLARE_OVERRIDE_METHOD(void, DidActivate, il2cpp_utils::FindMethodUnsafe("HMUI", "ViewController", "DidActivate", 3), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
)