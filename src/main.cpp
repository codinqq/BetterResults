#include "main.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/LevelStatsView.hpp"
#include "GlobalNamespace/CustomBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapData.hpp"

using namespace GlobalNamespace;

#include "questui/shared/CustomTypes/Components/ExternalComponents.hpp"
#include "questui/shared/CustomTypes/Components/Backgroundable.hpp"

using namespace QuestUI;

#include "questui/shared/BeatSaberUI.hpp"
using namespace QuestUI::BeatSaberUI;

#include "UnityEngine/GameObject.hpp"
using namespace UnityEngine;

// <--------
static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

// Loads the config from disk using our modInfo, then returns it for use
Configuration &getConfig()
{
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger &getLogger()
{
    static Logger *logger = new Logger(modInfo);
    return *logger;
}

static double calculatePercentage(int maxScore, int resultScore)
{
    double resultPercentage = (double)(100 / (double)maxScore * (double)resultScore);
    return resultPercentage;
}

static int calculateMaxScore(int blockCount)
{
    int maxScore;
    if (blockCount < 14)
    {
        if (blockCount == 1)
        {
            maxScore = 115;
        }
        else if (blockCount < 5)
        {
            maxScore = (blockCount - 1) * 230 + 115;
        }
        else
        {
            maxScore = (blockCount - 5) * 460 + 1035;
        }
    }
    else
    {
        maxScore = (blockCount - 13) * 920 + 4715;
    }
    return maxScore;
}

TMPro::TextMeshProUGUI *title = nullptr;
TMPro::TextMeshProUGUI *score = nullptr;
TMPro::TextMeshProUGUI *combo = nullptr;
TMPro::TextMeshProUGUI *avg = nullptr;
TMPro::TextMeshProUGUI *scoreTextPercentage = nullptr;

UnityEngine::Vector2 vectorTitle = UnityEngine::Vector2(-75, 70);
UnityEngine::Vector2 vectorscore = UnityEngine::Vector2(-75, 60);
UnityEngine::Vector2 vectormaxcombo = UnityEngine::Vector2(0, -20);
UnityEngine::Vector2 vectoraverage = UnityEngine::Vector2(0, -20);
UnityEngine::Vector2 vectorpercentage = UnityEngine::Vector2(0, -10);

std::string titleText = "<color=#0026ff><size=10>Better Results</size></color>";

MAKE_HOOK_MATCH(
    ResultsViewController_Init,
    &ResultsViewController::Init,
    void,
    ResultsViewController *self,
    LevelCompletionResults *result,
    IDifficultyBeatmap *beatmap,
    bool practice,
    bool newHighScore)
{


    double percentage = calculatePercentage(calculateMaxScore(beatmap->get_beatmapData()->get_cuttableNotesCount()), result->dyn_modifiedScore());

    std::string averageCut = string_format("<align=\"center\"><color=#D3D3D3><size=4>Average Cut</size></color>\n<color=#00AFF1><size=13>%d</size></color></align>", result->dyn_averageCutScore());
    std::string maxCombo = string_format("<align=\"center\"><color=#D3D3D3><size=4>Max Combo</size></color>\n<color=#00AFF1><size=13>%d</size></color></align>", result->dyn_maxCombo());

    TMPro::TextMeshProUGUI scoretext = self->dyn__scoreText();
    UnityEngine::Vector2 scoreTextVector = UnityEngine::Vector2(0, -10);

    if (title == nullptr || score == nullptr || combo == nullptr || avg == nullptr || scoreTextPercentage == nullptr)
    {

        UnityEngine::Transform *trans = self->dyn__clearedPanel()->get_transform();
        UnityEngine::UI::HorizontalLayoutGroup *layout = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(trans);


        combo = QuestUI::BeatSaberUI::CreateText(layout->get_transform(), "", true, vectormaxcombo);
        avg = QuestUI::BeatSaberUI::CreateText(layout->get_transform(), "", true, vectoraverage);

        scoreTextPercentage = QuestUI::BeatSaberUI::CreateText(self->dyn__scoreText()->get_transform(), "", true, vectorpercentage);
        scoreTextPercentage->set_alignment(TMPro::TextAlignmentOptions::Center);

        
    }

    combo->set_text(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(maxCombo));
    avg->set_text(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(averageCut));
    scoreTextPercentage->set_text(il2cpp_utils::newcsstr<il2cpp_utils::CreationType::Manual>(string_format("<color=#D3D3D3><size=4>%.2f percent</size></color>", std::round(percentage))));

    BeatSaberUI::AddHoverHint(avg->get_gameObject(), string_format("Good - %d / Bad - %d", result->dyn_goodCutsCount(), result->dyn_badCutsCount()));

    ResultsViewController_Init(self, result, beatmap, practice, newHighScore);
}

MAKE_HOOK_MATCH(ResultsViewController_Restart, &ResultsViewController::RestartButtonPressed, void, ResultsViewController *self) {

    UnityEngine::GameObject::Destroy(combo->get_gameObject());
    UnityEngine::GameObject::Destroy(avg->get_gameObject());

    ResultsViewController_Restart(self);

}

MAKE_HOOK_MATCH(ResultsViewController_Continue, &ResultsViewController::ContinueButtonPressed, void, ResultsViewController *self) {

    UnityEngine::GameObject::Destroy(combo->get_gameObject());
    UnityEngine::GameObject::Destroy(avg->get_gameObject());

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

    LoggerContextObject logger = getLogger().WithContext("load");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, ResultsViewController_Init);
    INSTALL_HOOK(logger, ResultsViewController_Restart);
    INSTALL_HOOK(logger, ResultsViewController_Continue);

    getLogger().info("Installed all hooks!");
}