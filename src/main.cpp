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

std::string calculateColor(double percentage) {

    if(percentage > 90) {
        return "#00FF00";
    } else if(percentage > 80) {
        return "#8AFF8A";
    } else if(percentage > 50) {
        return "#FFD700";
    } else {
        return "#FF0000";
    }

}

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

    GameObject *scoreTextObject = self->scoreText->get_gameObject();
    GameObject *goodCutsObject = self->goodCutsPercentageText->get_gameObject();
    GameObject *rankTextObject = self->rankText->get_gameObject();

    double percentage = calculatePercentage(ScoreModel::ComputeMaxMultipliedScoreForBeatmap(beatMapData), result->modifiedScore);

    QuestUI::BeatSaberUI::AddHoverHint(rankTextObject, string_format("<color=%s>%.2f</color>", calculateColor(percentage).c_str(), percentage) + "%");
    QuestUI::BeatSaberUI::AddHoverHint(goodCutsObject, string_format("<color=#50C878>%d Good</color> - <color=#ff4118>%d Bad</color> - <color=#000000>%d Missed</color>", result->goodCutsCount, result->badCutsCount, result->missedCount));
    
    ResultsViewController_Init(self, result, beatMapData, beatmap, practice, newHighScore);
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

    QuestUI::Init();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, ResultsViewController_Init);
    getLogger().info("Installed all hooks!");

}