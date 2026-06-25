#include "QuestSystemManager.h"

// ============================================================
// Quest System Manager — Implementation
// Agent #14 Quest & Mission Designer
// PROD_CYCLE_AUTO_20260625_004
// ============================================================

UQuestSystemManager::UQuestSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check time limits every second
    MaxActiveQuests = 3;
}

void UQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void UQuestSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update elapsed time for time-limited quests
    for (FQuest_Definition& Quest : ActiveQuests)
    {
        if (Quest.QuestState == EQuest_State::Active && Quest.bHasTimeLimit)
        {
            float& Elapsed = QuestElapsedTimes.FindOrAdd(Quest.QuestID);
            Elapsed += DeltaTime;
            if (Elapsed >= Quest.TimeLimit)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}

void UQuestSystemManager::InitializeDefaultQuests()
{
    // --- Quest 1: Track the Parasaurolophus Herd ---
    FQuest_Definition TrackHerd;
    TrackHerd.QuestID = TEXT("QUEST_TRACK_HERD_001");
    TrackHerd.QuestTitle = TEXT("Follow the Herd");
    TrackHerd.QuestDescription = TEXT("The Parasaurolophus herd is migrating north. Follow their tracks to find the water source before the raptors cut you off.");
    TrackHerd.QuestType = EQuest_Type::Track;
    TrackHerd.QuestState = EQuest_State::Inactive;
    TrackHerd.bHasTimeLimit = false;

    FQuest_Objective WaypointA;
    WaypointA.ObjectiveID = TEXT("OBJ_WAYPOINT_A");
    WaypointA.Description = TEXT("Find the first herd tracks near the river bend");
    WaypointA.WorldLocation = FVector(2900.0f, 1900.0f, 420.0f);
    WaypointA.RequiredCount = 1;

    FQuest_Objective WaypointB;
    WaypointB.ObjectiveID = TEXT("OBJ_WAYPOINT_B");
    WaypointB.Description = TEXT("Follow the trail through the open valley");
    WaypointB.WorldLocation = FVector(3100.0f, 2050.0f, 420.0f);
    WaypointB.RequiredCount = 1;

    FQuest_Objective WaypointC;
    WaypointC.ObjectiveID = TEXT("OBJ_WAYPOINT_C");
    WaypointC.Description = TEXT("Reach the herd's resting ground");
    WaypointC.WorldLocation = FVector(3250.0f, 2150.0f, 420.0f);
    WaypointC.RequiredCount = 1;

    TrackHerd.Objectives.Add(WaypointA);
    TrackHerd.Objectives.Add(WaypointB);
    TrackHerd.Objectives.Add(WaypointC);

    // --- Quest 2: Rescue the Hunters ---
    FQuest_Definition RescueHunters;
    RescueHunters.QuestID = TEXT("QUEST_RESCUE_HUNTERS_001");
    RescueHunters.QuestTitle = TEXT("Find the Missing Hunters");
    RescueHunters.QuestDescription = TEXT("Three hunters went to investigate raptor activity on the eastern ridge. None have returned. Find them before nightfall.");
    RescueHunters.QuestType = EQuest_Type::Rescue;
    RescueHunters.QuestState = EQuest_State::Inactive;
    RescueHunters.bHasTimeLimit = true;
    RescueHunters.TimeLimit = 600.0f; // 10 minutes real time

    FQuest_Objective RescuePoint1;
    RescuePoint1.ObjectiveID = TEXT("OBJ_RESCUE_1");
    RescuePoint1.Description = TEXT("Find the first hunter near the eastern boulder field");
    RescuePoint1.WorldLocation = FVector(4300.0f, 1650.0f, 420.0f);
    RescuePoint1.RequiredCount = 1;

    FQuest_Objective RescuePoint2;
    RescuePoint2.ObjectiveID = TEXT("OBJ_RESCUE_2");
    RescuePoint2.Description = TEXT("Find the remaining hunters at the ridge camp");
    RescuePoint2.WorldLocation = FVector(4450.0f, 1720.0f, 420.0f);
    RescuePoint2.RequiredCount = 2;

    RescueHunters.Objectives.Add(RescuePoint1);
    RescueHunters.Objectives.Add(RescuePoint2);

    // --- Quest 3: Survive the Stampede ---
    FQuest_Definition StampedeSurvival;
    StampedeSurvival.QuestID = TEXT("QUEST_STAMPEDE_SURVIVAL_001");
    StampedeSurvival.QuestTitle = TEXT("Survive the Stampede");
    StampedeSurvival.QuestDescription = TEXT("The herd has been spooked. Hundreds of Parasaurolophus are charging through the valley. Get to high ground before you are trampled.");
    StampedeSurvival.QuestType = EQuest_Type::Survive;
    StampedeSurvival.QuestState = EQuest_State::Inactive;
    StampedeSurvival.bHasTimeLimit = true;
    StampedeSurvival.TimeLimit = 120.0f; // 2 minutes to escape

    FQuest_Objective EscapeRoute;
    EscapeRoute.ObjectiveID = TEXT("OBJ_ESCAPE_HIGH_GROUND");
    EscapeRoute.Description = TEXT("Reach the high ground before the stampede arrives");
    EscapeRoute.WorldLocation = FVector(3100.0f, 2600.0f, 600.0f);
    EscapeRoute.RequiredCount = 1;

    StampedeSurvival.Objectives.Add(EscapeRoute);

    // Register all default quests
    RegisterQuest(TrackHerd);
    RegisterQuest(RescueHunters);
    RegisterQuest(StampedeSurvival);
}

void UQuestSystemManager::RegisterQuest(const FQuest_Definition& QuestDef)
{
    // Don't register duplicates
    for (const FQuest_Definition& Existing : ActiveQuests)
    {
        if (Existing.QuestID == QuestDef.QuestID)
        {
            return;
        }
    }
    ActiveQuests.Add(QuestDef);
}

bool UQuestSystemManager::ActivateQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return false;
    }

    if (ActiveQuests[Idx].QuestState != EQuest_State::Inactive)
    {
        return false;
    }

    // Count currently active quests
    int32 CurrentActive = 0;
    for (const FQuest_Definition& Q : ActiveQuests)
    {
        if (Q.QuestState == EQuest_State::Active)
        {
            ++CurrentActive;
        }
    }

    if (CurrentActive >= MaxActiveQuests)
    {
        return false;
    }

    ActiveQuests[Idx].QuestState = EQuest_State::Active;
    QuestElapsedTimes.Add(QuestID, 0.0f);
    return true;
}

bool UQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    int32 QIdx = FindQuestIndex(QuestID);
    if (QIdx == INDEX_NONE)
    {
        return false;
    }

    if (ActiveQuests[QIdx].QuestState != EQuest_State::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : ActiveQuests[QIdx].Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
            }
            CheckQuestCompletion(QuestID);
            return true;
        }
    }
    return false;
}

bool UQuestSystemManager::CheckQuestCompletion(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return false;
    }

    for (const FQuest_Objective& Obj : ActiveQuests[Idx].Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            return false;
        }
    }

    // All objectives complete
    ActiveQuests[Idx].QuestState = EQuest_State::Completed;
    CompletedQuestIDs.AddUnique(QuestID);
    QuestElapsedTimes.Remove(QuestID);
    return true;
}

void UQuestSystemManager::FailQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx != INDEX_NONE)
    {
        ActiveQuests[Idx].QuestState = EQuest_State::Failed;
        QuestElapsedTimes.Remove(QuestID);
    }
}

EQuest_State UQuestSystemManager::GetQuestState(const FString& QuestID) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return EQuest_State::Inactive;
    }
    return ActiveQuests[Idx].QuestState;
}

int32 UQuestSystemManager::FindQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < ActiveQuests.Num(); ++i)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}
