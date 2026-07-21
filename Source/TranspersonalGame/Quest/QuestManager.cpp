// QuestManager.cpp
// Transpersonal Game Studio — Agent #4 Performance Optimizer
// Tutorial quest system: "Find Water" — player walks to water source at (8000,0,0)
// Optimized: event-driven, no per-frame polling

#include "QuestManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
    ActiveQuestIndex = -1;
    bTutorialComplete = false;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Register tutorial quests on startup
    RegisterTutorialQuests();

    // Auto-start first quest
    StartQuest(0);

    UE_LOG(LogTemp, Log, TEXT("QuestManager initialized — Tutorial quest active: %s"),
        ActiveQuestIndex >= 0 ? *Quests[ActiveQuestIndex].QuestName : TEXT("NONE"));
}

void UQuestManager::Deinitialize()
{
    Quests.Empty();
    Super::Deinitialize();
}

void UQuestManager::RegisterTutorialQuests()
{
    // Quest 0: Find Water
    FPerf_QuestData FindWater;
    FindWater.QuestID = 0;
    FindWater.QuestName = TEXT("Find Water");
    FindWater.Description = TEXT("You are thirsty. Find a water source to survive.");
    FindWater.ObjectiveText = TEXT("Reach the water source to the east.");
    FindWater.State = EPerf_QuestState::NotStarted;
    FindWater.TargetLocation = FVector(8000.0f, 0.0f, 50.0f);
    FindWater.CompletionRadius = 500.0f;
    FindWater.bHasLocationObjective = true;
    Quests.Add(FindWater);

    // Quest 1: Find Food
    FPerf_QuestData FindFood;
    FindFood.QuestID = 1;
    FindFood.QuestName = TEXT("Find Food");
    FindFood.Description = TEXT("You are hungry. Find something to eat.");
    FindFood.ObjectiveText = TEXT("Gather berries or hunt small prey.");
    FindFood.State = EPerf_QuestState::NotStarted;
    FindFood.bHasLocationObjective = false;
    Quests.Add(FindFood);

    // Quest 2: Survive the Night
    FPerf_QuestData SurviveNight;
    SurviveNight.QuestID = 2;
    SurviveNight.QuestName = TEXT("Survive the Night");
    SurviveNight.Description = TEXT("Darkness brings danger. Find shelter before nightfall.");
    SurviveNight.ObjectiveText = TEXT("Find or build a shelter before night.");
    SurviveNight.State = EPerf_QuestState::NotStarted;
    SurviveNight.bHasLocationObjective = false;
    Quests.Add(SurviveNight);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered %d tutorial quests"), Quests.Num());
}

void UQuestManager::StartQuest(int32 QuestIndex)
{
    if (!Quests.IsValidIndex(QuestIndex)) return;

    FPerf_QuestData& Quest = Quests[QuestIndex];
    if (Quest.State != EPerf_QuestState::NotStarted) return;

    Quest.State = EPerf_QuestState::Active;
    ActiveQuestIndex = QuestIndex;

    OnQuestStarted.Broadcast(Quest);
    UE_LOG(LogTemp, Log, TEXT("Quest started: %s — %s"), *Quest.QuestName, *Quest.ObjectiveText);
}

void UQuestManager::CompleteQuest(int32 QuestID)
{
    for (FPerf_QuestData& Quest : Quests)
    {
        if (Quest.QuestID == QuestID && Quest.State == EPerf_QuestState::Active)
        {
            Quest.State = EPerf_QuestState::Completed;
            OnQuestCompleted.Broadcast(Quest);
            UE_LOG(LogTemp, Log, TEXT("Quest COMPLETED: %s"), *Quest.QuestName);

            // Auto-start next quest
            int32 NextIndex = QuestID + 1;
            if (Quests.IsValidIndex(NextIndex))
            {
                StartQuest(NextIndex);
            }
            else
            {
                bTutorialComplete = true;
                UE_LOG(LogTemp, Log, TEXT("Tutorial sequence COMPLETE"));
            }
            return;
        }
    }
}

void UQuestManager::FailQuest(int32 QuestID)
{
    for (FPerf_QuestData& Quest : Quests)
    {
        if (Quest.QuestID == QuestID && Quest.State == EPerf_QuestState::Active)
        {
            Quest.State = EPerf_QuestState::Failed;
            OnQuestFailed.Broadcast(Quest);
            UE_LOG(LogTemp, Log, TEXT("Quest FAILED: %s"), *Quest.QuestName);
            return;
        }
    }
}

void UQuestManager::CheckLocationObjective(FVector PlayerLocation)
{
    if (!Quests.IsValidIndex(ActiveQuestIndex)) return;

    FPerf_QuestData& Quest = Quests[ActiveQuestIndex];
    if (Quest.State != EPerf_QuestState::Active) return;
    if (!Quest.bHasLocationObjective) return;

    float DistSq = FVector::DistSquared(PlayerLocation, Quest.TargetLocation);
    float RadiusSq = Quest.CompletionRadius * Quest.CompletionRadius;

    if (DistSq <= RadiusSq)
    {
        CompleteQuest(Quest.QuestID);
    }
}

void UQuestManager::OnPlayerDrankWater()
{
    // Called by SurvivalComponent::OnWaterConsumed
    // Completes "Find Water" quest if active
    if (Quests.IsValidIndex(ActiveQuestIndex) &&
        Quests[ActiveQuestIndex].QuestID == 0 &&
        Quests[ActiveQuestIndex].State == EPerf_QuestState::Active)
    {
        CompleteQuest(0);
    }
}

FPerf_QuestData UQuestManager::GetActiveQuest() const
{
    if (Quests.IsValidIndex(ActiveQuestIndex))
    {
        return Quests[ActiveQuestIndex];
    }
    return FPerf_QuestData();
}

bool UQuestManager::HasActiveQuest() const
{
    return Quests.IsValidIndex(ActiveQuestIndex) &&
           Quests[ActiveQuestIndex].State == EPerf_QuestState::Active;
}

TArray<FPerf_QuestData> UQuestManager::GetCompletedQuests() const
{
    TArray<FPerf_QuestData> Completed;
    for (const FPerf_QuestData& Quest : Quests)
    {
        if (Quest.State == EPerf_QuestState::Completed)
        {
            Completed.Add(Quest);
        }
    }
    return Completed;
}
