// QuestManager.cpp — Quest system implementation for prehistoric survival game
// Agent: #04 Performance Optimizer — Cycle AUTO_20260630_011
// Implements: Tutorial quest "Find Water", quest state machine, objective tracking

#include "QuestManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
    // Initialize tutorial quest on construction
    ActiveQuestID = NAME_None;
    bQuestSystemInitialized = false;
    CurrentObjectiveIndex = 0;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bQuestSystemInitialized = true;
    RegisterDefaultQuests();
    unreal::log("QuestManager initialized");
}

void UQuestManager::RegisterDefaultQuests()
{
    // Tutorial Quest: Find Water
    FPerf_QuestData TutorialQuest;
    TutorialQuest.QuestID = FName("QUEST_TUTORIAL_WATER");
    TutorialQuest.QuestName = FText::FromString("Survive the First Day");
    TutorialQuest.QuestDescription = FText::FromString(
        "You have awoken in an unknown prehistoric wilderness. "
        "Your throat is parched. Find a water source before dehydration kills you."
    );
    TutorialQuest.QuestState = EPerf_QuestState::NotStarted;
    TutorialQuest.bIsMainQuest = false;
    TutorialQuest.bIsTutorial = true;

    // Objective 1: Find water source
    FPerf_QuestObjective WaterObjective;
    WaterObjective.ObjectiveID = FName("OBJ_FIND_WATER");
    WaterObjective.Description = FText::FromString("Find a water source (follow the sound of running water)");
    WaterObjective.TargetCount = 1;
    WaterObjective.CurrentCount = 0;
    WaterObjective.bIsComplete = false;
    WaterObjective.ObjectiveType = EPerf_ObjectiveType::ReachLocation;
    WaterObjective.TargetLocation = FVector(8000.0f, 0.0f, 50.0f);
    WaterObjective.CompletionRadius = 500.0f;
    TutorialQuest.Objectives.Add(WaterObjective);

    // Objective 2: Drink water
    FPerf_QuestObjective DrinkObjective;
    DrinkObjective.ObjectiveID = FName("OBJ_DRINK_WATER");
    DrinkObjective.Description = FText::FromString("Drink from the water source to restore thirst");
    DrinkObjective.TargetCount = 1;
    DrinkObjective.CurrentCount = 0;
    DrinkObjective.bIsComplete = false;
    DrinkObjective.ObjectiveType = EPerf_ObjectiveType::UseItem;
    TutorialQuest.Objectives.Add(DrinkObjective);

    RegisteredQuests.Add(TutorialQuest.QuestID, TutorialQuest);

    // Second quest: Craft a tool
    FPerf_QuestData CraftQuest;
    CraftQuest.QuestID = FName("QUEST_CRAFT_TOOL");
    CraftQuest.QuestName = FText::FromString("Primitive Craftsmanship");
    CraftQuest.QuestDescription = FText::FromString(
        "Rocks and sticks litter the ground around you. "
        "Combine them to craft a primitive stone tool — your first step toward survival."
    );
    CraftQuest.QuestState = EPerf_QuestState::NotStarted;
    CraftQuest.bIsMainQuest = false;
    CraftQuest.bIsTutorial = true;

    FPerf_QuestObjective GatherRockObjective;
    GatherRockObjective.ObjectiveID = FName("OBJ_GATHER_ROCK");
    GatherRockObjective.Description = FText::FromString("Pick up a sharp rock (2)");
    GatherRockObjective.TargetCount = 2;
    GatherRockObjective.CurrentCount = 0;
    GatherRockObjective.bIsComplete = false;
    GatherRockObjective.ObjectiveType = EPerf_ObjectiveType::CollectItem;
    CraftQuest.Objectives.Add(GatherRockObjective);

    FPerf_QuestObjective CraftObjective;
    CraftObjective.ObjectiveID = FName("OBJ_CRAFT_STONE_TOOL");
    CraftObjective.Description = FText::FromString("Craft a stone scraper at a crafting spot");
    CraftObjective.TargetCount = 1;
    CraftObjective.CurrentCount = 0;
    CraftObjective.bIsComplete = false;
    CraftObjective.ObjectiveType = EPerf_ObjectiveType::CraftItem;
    CraftQuest.Objectives.Add(CraftObjective);

    RegisteredQuests.Add(CraftQuest.QuestID, CraftQuest);
}

bool UQuestManager::StartQuest(FName QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found"), *QuestID.ToString());
        return false;
    }

    FPerf_QuestData& Quest = RegisteredQuests[QuestID];
    if (Quest.QuestState != EPerf_QuestState::NotStarted)
    {
        return false;
    }

    Quest.QuestState = EPerf_QuestState::Active;
    ActiveQuestID = QuestID;
    CurrentObjectiveIndex = 0;

    OnQuestStarted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest %s"), *QuestID.ToString());
    return true;
}

bool UQuestManager::UpdateObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    FPerf_QuestData& Quest = RegisteredQuests[QuestID];
    if (Quest.QuestState != EPerf_QuestState::Active)
    {
        return false;
    }

    for (FPerf_QuestObjective& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentCount = FMath::Min(
                Objective.CurrentCount + ProgressAmount,
                Objective.TargetCount
            );

            if (Objective.CurrentCount >= Objective.TargetCount)
            {
                Objective.bIsComplete = true;
                OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %s complete"), *ObjectiveID.ToString());

                // Check if all objectives complete
                CheckQuestCompletion(QuestID);
            }
            return true;
        }
    }
    return false;
}

void UQuestManager::CheckQuestCompletion(FName QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return;
    }

    FPerf_QuestData& Quest = RegisteredQuests[QuestID];
    bool bAllComplete = true;

    for (const FPerf_QuestObjective& Objective : Quest.Objectives)
    {
        if (!Objective.bIsComplete)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        Quest.QuestState = EPerf_QuestState::Completed;
        if (ActiveQuestID == QuestID)
        {
            ActiveQuestID = NAME_None;
        }
        OnQuestCompleted.Broadcast(QuestID);
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest %s COMPLETED!"), *QuestID.ToString());
    }
}

FPerf_QuestData UQuestManager::GetQuestData(FName QuestID) const
{
    if (RegisteredQuests.Contains(QuestID))
    {
        return RegisteredQuests[QuestID];
    }
    return FPerf_QuestData();
}

TArray<FName> UQuestManager::GetActiveQuests() const
{
    TArray<FName> ActiveQuests;
    for (const auto& Pair : RegisteredQuests)
    {
        if (Pair.Value.QuestState == EPerf_QuestState::Active)
        {
            ActiveQuests.Add(Pair.Key);
        }
    }
    return ActiveQuests;
}

void UQuestManager::StartTutorialSequence()
{
    // Auto-start tutorial quest on game begin
    StartQuest(FName("QUEST_TUTORIAL_WATER"));
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Tutorial sequence started"));
}
