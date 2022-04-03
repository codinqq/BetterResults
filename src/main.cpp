#include "main.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/LevelStatsView.hpp"
#include "GlobalNamespace/CustomBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/ScoreModel.hpp"

using namespace GlobalNamespace;

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace QuestUI;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI::BeatSaberUI;

#include "UnityEngine/GameObject.hpp"
using namespace UnityEngine;

#include "ModConfig.hpp"

#include "UI/SettingsMenu.hpp"


#include <string>
#include <iostream>
#include <sstream>


// <--------
static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration &getConfig()
{
    static Configuration config(modInfo);
    config.Load();
    return config;
}


DEFINE_CONFIG(ModConfig);

// Returns a logger, useful for printing debug messages
Logger &getLogger()
{
    static Logger *logger = new Logger(modInfo);
    return *logger;
}

//ez percentage calculation
static double calculatePercentage(float maxScore, float resultScore)
{
    double resultPercentage = (resultScore * 100 ) / maxScore; 
    return resultPercentage;
}

// This was a hell to find.
std::string to_hex_string(uint8_t red, uint8_t green, uint8_t blue) {
    // r, g, b -> "#RRGGBB"
    std::ostringstream oss;
    oss << '#';
    oss.fill('0');
    oss.width(6);
    oss << std::uppercase << std::hex << ((red << 16) | (green << 8) | blue);
    return oss.str();
}

// joinked from rxzz0 (https://github.com/rxzz0/CustomMissText/blob/main/src/UI/Custom/ConfigValueColorPickerModal.hpp) as it was the only one that worked, and I didn't want to make one myself :)
inline ::QuestUI::ModalColorPicker* AddConfigValueColorPickerModal(UnityEngine::Transform* parent, ConfigUtils::ConfigValue<::UnityEngine::Color>& configValue) {
    auto object = ::QuestUI::BeatSaberUI::CreateColorPickerModal(parent, configValue.GetName(), configValue.GetValue(), nullptr, nullptr, [&configValue](::UnityEngine::Color value) {
            configValue.SetValue(value);
        }
    );
    if(!configValue.GetHoverHint().empty())
        ::QuestUI::BeatSaberUI::AddHoverHint(object, configValue.GetHoverHint());
    return object;
}

TMPro::TextMeshProUGUI *averageCutText = nullptr;
TMPro::TextMeshProUGUI *goodCutsText = nullptr;
TMPro::TextMeshProUGUI *badCutsText = nullptr;
TMPro::TextMeshProUGUI *missedCutsText = nullptr;

TMPro::TextMeshProUGUI *scoreTextPercentage = nullptr;
GameObject* container = nullptr;

MAKE_HOOK_MATCH(
    ResultsViewController_Init,
    &ResultsViewController::Init,
    void,
    ResultsViewController *self,
    LevelCompletionResults *result,
    IReadonlyBeatmapData *beatMapData,
    IDifficultyBeatmap *beatmap,
    bool practice,
    bool newHighScore)
{
    
    container = QuestUI::BeatSaberUI::CreateFloatingScreen(Vector2(64,128), Vector3(-2.5, 0, 3), Vector3(0,-48,0), 0.0F, true, false, 2);

    UnityEngine::UI::GridLayoutGroup *layout = QuestUI::BeatSaberUI::CreateGridLayoutGroup(container->get_transform());
    layout->set_spacing(Vector2(20,20));
    layout->set_cellSize(Vector2(64,64));

    UnityEngine::UI::VerticalLayoutGroup *vertLayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(layout->get_transform());

    UnityEngine::UI::HorizontalLayoutGroup *avgGroup = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(vertLayout->get_transform());
    UnityEngine::UI::HorizontalLayoutGroup *cutsGroup = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(vertLayout->get_transform());

    cutsGroup->set_spacing(10);

    UnityEngine::UI::HorizontalLayoutGroup *percentGroup = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(vertLayout->get_transform());

    double percentage = calculatePercentage(ScoreModel::ComputeMaxMultipliedScoreForBeatmap(beatMapData), result->dyn_modifiedScore());

    Color titleColor = getModConfig().titleColor.GetValue();
    Color valueColor = getModConfig().valueColor.GetValue();

    std::string colorTitle = to_hex_string(titleColor.r*255, titleColor.g*255, titleColor.b*255);
    std::string colorValue = to_hex_string(valueColor.r*255, valueColor.g*255, valueColor.b*255);

    std::string averageCut = string_format(
    "<color=%s><size=4>Average Cuts</size></color>\n<color=%s><size=8>%.2f</size></color>", 
    colorTitle.c_str(), colorValue.c_str(), result->dyn_averageCutScoreForNotesWithFullScoreScoringType());

    std::string goodCuts = string_format(
    "<color=%s><size=4>Good Cuts</size></color>\n<color=%s><size=8>%d</size></color>", 
    colorTitle.c_str(), colorValue.c_str(), result->dyn_goodCutsCount());

    std::string badCuts = string_format(
    "<color=%s><size=4>Bad Cuts</size></color>\n<color=%s><size=8>%d</size></color>", 
    colorTitle.c_str(), colorValue.c_str(), result->dyn_badCutsCount());

    std::string missedCuts = string_format(
    "<color=%s><size=4>Missed Cuts</size></color>\n<color=%s><size=8>%d</size></color>", 
    colorTitle.c_str(), colorValue.c_str(), result->dyn_missedCount());

    std::string scorePercentage = string_format(
    "<color=%s><size=4>Percentage</size></color>\n<color=%s><size=8>%.2f</size></color>", 
    colorTitle.c_str(), colorValue.c_str(), percentage);

    averageCutText = QuestUI::BeatSaberUI::CreateText(avgGroup->get_transform(), il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(averageCut), true);
    averageCutText->set_alignment(TMPro::TextAlignmentOptions::Center);

    goodCutsText = QuestUI::BeatSaberUI::CreateText(cutsGroup->get_transform(), il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(goodCuts), true);
    goodCutsText->set_alignment(TMPro::TextAlignmentOptions::Center);

    badCutsText = QuestUI::BeatSaberUI::CreateText(cutsGroup->get_transform(), il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(badCuts), true);
    badCutsText->set_alignment(TMPro::TextAlignmentOptions::Center);

    missedCutsText = QuestUI::BeatSaberUI::CreateText(cutsGroup->get_transform(), il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(missedCuts), true);
    missedCutsText->set_alignment(TMPro::TextAlignmentOptions::Center);

    
    scoreTextPercentage = QuestUI::BeatSaberUI::CreateText(percentGroup->get_transform(), il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(scorePercentage), true);
    scoreTextPercentage->set_alignment(TMPro::TextAlignmentOptions::Center);

    ResultsViewController_Init(self, result, beatMapData, beatmap, practice, newHighScore);
}


// Destroy the container when restart or continue has been clicked.
MAKE_HOOK_MATCH(ResultsViewController_Restart, &ResultsViewController::RestartButtonPressed, void, ResultsViewController *self) {
    UnityEngine::GameObject::Destroy(container->get_gameObject());
    ResultsViewController_Restart(self);
}

MAKE_HOOK_MATCH(ResultsViewController_Continue, &ResultsViewController::ContinueButtonPressed, void, ResultsViewController *self) {
    UnityEngine::GameObject::Destroy(container->get_gameObject());
    ResultsViewController_Continue(self);
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo &info)
{
    info.id = ID;
    info.version = VERSION;
    modInfo = info;

    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load()
{
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, ResultsViewController_Init);
    INSTALL_HOOK(logger, ResultsViewController_Restart);
    INSTALL_HOOK(logger, ResultsViewController_Continue);
    getLogger().info("Installed all hooks!");

    getLogger().info("Registering custom types...");
    custom_types::Register::AutoRegister();
    getLogger().info("Registered custom types!");

    QuestUI::Register::RegisterMainMenuModSettingsViewController<BetterResults::UI::Settings::SettingsMenu*>(modInfo);
    getLogger().info("Successfully added a button to the Settings UI in the Main Menu!");

}