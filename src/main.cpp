#include "main.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"
#include "GlobalNamespace/LevelStatsView.hpp"
#include "GlobalNamespace/CustomBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/ScoreModel.hpp"
#include "GlobalNamespace/CutScoreBuffer.hpp"

using namespace GlobalNamespace;

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

GameObject* resultsContainer = nullptr;

int leftScore = 0;
int rightScore = 0;

int leftNotes = 0;
int rightNotes = 0;


static double calculateAverage(int scoreTotal, int maxNotes) {
    double result = floor((scoreTotal)*100/maxNotes)/100;   
    return result;
}


MAKE_HOOK_MATCH(SaberSwingRatingFinished , &CutScoreBuffer::HandleSaberSwingRatingCounterDidFinish, void, CutScoreBuffer* self, ISaberSwingRatingCounter* swingRatingCounter){

        SaberSwingRatingFinished(self, swingRatingCounter);

        int saber = self->noteCutInfo.saberType;
        int score = self->get_cutScore();

        if(saber == 0)  {
            leftNotes++;
            leftScore = leftScore + score;
        } 
        if(saber == 1)  {
            rightNotes++;
            rightScore = rightScore + score;
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

    // Screen with more infomation
    resultsContainer = QuestUI::BeatSaberUI::CreateFloatingScreen(Vector2(64,128), Vector3(2,0,4), Vector3(0,48,0), 0.0F, true, false, 2);

    UnityEngine::UI::GridLayoutGroup *layout = QuestUI::BeatSaberUI::CreateGridLayoutGroup(resultsContainer->get_transform());
    layout->set_spacing(Vector2(10,10));
    layout->set_cellSize(Vector2(32,64));

    UnityEngine::UI::VerticalLayoutGroup *vertlayout = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(layout->get_transform());
    UnityEngine::UI::HorizontalLayoutGroup *cutsGroup = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(vertlayout->get_transform());
    UnityEngine::UI::HorizontalLayoutGroup *movementGroup = QuestUI::BeatSaberUI::CreateHorizontalLayoutGroup(vertlayout->get_transform());

    movementGroup->set_spacing(10);
    cutsGroup->set_spacing(10);

    TMPro::TextMeshProUGUI *leftCuts = QuestUI::BeatSaberUI::CreateText(cutsGroup->get_transform(), string_format("<size=1>Left Cuts</size>\n<size=2>%.2f</size>", calculateAverage(leftScore, leftNotes)), true);
    leftCuts->set_alignment(TMPro::TextAlignmentOptions::Left);
    TMPro::TextMeshProUGUI *rightCuts = QuestUI::BeatSaberUI::CreateText(cutsGroup->get_transform(), string_format("<size=1>Right Cuts</size>\n<size=2>%.2f</size>", calculateAverage(rightScore, rightNotes)), true);
    rightCuts->set_alignment(TMPro::TextAlignmentOptions::Right);

    TMPro::TextMeshProUGUI *leftMovement = QuestUI::BeatSaberUI::CreateText(movementGroup->get_transform(), string_format("<size=1>Left Movement</size>\n<size=2>%.2f meters</size>", result->leftSaberMovementDistance), true);
    leftMovement->set_alignment(TMPro::TextAlignmentOptions::Left);
    TMPro::TextMeshProUGUI *rightMovement = QuestUI::BeatSaberUI::CreateText(movementGroup->get_transform(), string_format("<size=1>Right Movement</size>\n<size=2>%.2f meters</size>", result->rightSaberMovementDistance), true);
    rightMovement->set_alignment(TMPro::TextAlignmentOptions::Right);

    // Hoverhints
    double percentage = calculatePercentage(ScoreModel::ComputeMaxMultipliedScoreForBeatmap(beatMapData), result->modifiedScore);
    QuestUI::BeatSaberUI::AddHoverHint(rankTextObject, string_format("<color=%s>%.2f</color>", calculateColor(percentage).c_str(), percentage) + "%");
    QuestUI::BeatSaberUI::AddHoverHint(goodCutsObject, string_format("<color=#50C878>%d Good</color> - <color=#ff4118>%d Bad</color> - <color=#000000>%d Missed</color>", result->goodCutsCount, result->badCutsCount, result->missedCount));
    
    ResultsViewController_Init(self, result, beatMapData, beatmap, practice, newHighScore);
}

// Destroy the container when restart or continue has been clicked.
MAKE_HOOK_MATCH(ResultsViewController_Restart, &ResultsViewController::RestartButtonPressed, void, ResultsViewController *self) {
    UnityEngine::GameObject::Destroy(resultsContainer->get_gameObject());
    ResultsViewController_Restart(self);
}

MAKE_HOOK_MATCH(ResultsViewController_Continue, &ResultsViewController::ContinueButtonPressed, void, ResultsViewController *self) {
    UnityEngine::GameObject::Destroy(resultsContainer->get_gameObject());
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

    QuestUI::Init();

    getLogger().info("Installing hooks...");

    INSTALL_HOOK(logger, ResultsViewController_Init);
    INSTALL_HOOK(logger, SaberSwingRatingFinished);

    INSTALL_HOOK(logger, ResultsViewController_Restart);
    INSTALL_HOOK(logger, ResultsViewController_Continue);

    getLogger().info("Installed all hooks!");

}