// QuestManager.cpp
// Quest & Tutorial System — Agent #04 Performance Optimizer | Cycle 013
// Prehistoric survival game — NO spiritual/mystical content

#include "QuestManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

APerf_QuestManager::APerf_QuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check quests at 2Hz — no need for per-frame

    ActiveQuestID = TEXT("");
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void APerf_QuestManager::BeginPlay()
{
    Super::BeginPlay();

    // Auto-init tutorial quest on game start
    InitTutorialQuest();
    ActivateQuest(TEXT("QUEST_TUTORIAL_FIND_WATER"));
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void APerf_QuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ElapsedTime += DeltaTime;

    // Check proximity objectives against player location
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    CheckProximityObjectives(PlayerLoc);

    // Check time limits on active quest
    if (!ActiveQuestID.IsEmpty())
    {
        int32 Idx = FindQuestIndex(ActiveQuestID);
        if (Idx != INDEX_NONE)
        {
            FPerf_QuestData& Quest = AllQuests[Idx];
            if (Quest.TimeLimit > 0.0f && ElapsedTime >= Quest.TimeLimit)
            {
                FailQuest(ActiveQuestID);
            }
        }
    }
}

// ─── InitTutorialQuest ────────────────────────────────────────────────────────

void APerf_QuestManager::InitTutorialQuest()
{
    // Check if already registered
    if (FindQuestIndex(TEXT("QUEST_TUTORIAL_FIND_WATER")) != INDEX_NONE)
    {
        return;
    }

    FPerf_QuestData TutorialQuest;
    TutorialQuest.QuestID = TEXT("QUEST_TUTORIAL_FIND_WATER");
    TutorialQuest.QuestName = TEXT("Find Water");
    TutorialQuest.Description = TEXT("You are thirsty. Find a water source to survive.");
    TutorialQuest.QuestType = EPerf_QuestType::Tutorial;
    TutorialQuest.State = EPerf_QuestState::Inactive;
    TutorialQuest.bIsMainQuest = true;
    TutorialQuest.TimeLimit = 0.0f; // No time limit for tutorial

    // Objective 1: Reach the river
    FPerf_QuestObjective ReachRiver;
    ReachRiver.ObjectiveID = TEXT("OBJ_REACH_RIVER");
    ReachRiver.Description = TEXT("Follow the sound of water to the river");
    ReachRiver.bCompleted = false;
    ReachRiver.TargetLocation = TutorialWaterLocation;
    ReachRiver.ProximityRadius = TutorialWaterRadius;
    TutorialQuest.Objectives.Add(ReachRiver);

    // Objective 2: Drink (completed by SurvivalComponent calling Drink())
    FPerf_QuestObjective DrinkWater;
    DrinkWater.ObjectiveID = TEXT("OBJ_DRINK_WATER");
    DrinkWater.Description = TEXT("Drink from the river");
    DrinkWater.bCompleted = false;
    DrinkWater.TargetLocation = TutorialWaterLocation;
    DrinkWater.ProximityRadius = TutorialWaterRadius;
    TutorialQuest.Objectives.Add(DrinkWater);

    AllQuests.Add(TutorialQuest);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Tutorial quest QUEST_TUTORIAL_FIND_WATER registered."));
}

// ─── ActivateQuest ────────────────────────────────────────────────────────────

bool APerf_QuestManager::ActivateQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' not found."), *QuestID);
        return false;
    }

    FPerf_QuestData& Quest = AllQuests[Idx];
    if (Quest.State != EPerf_QuestState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' is not inactive (state=%d)."), *QuestID, (int32)Quest.State);
        return false;
    }

    Quest.State = EPerf_QuestState::Active;
    ActiveQuestID = QuestID;
    ElapsedTime = 0.0f;

    OnQuestActivated.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' ACTIVATED."), *QuestID);
    return true;
}

// ─── CompleteObjective ────────────────────────────────────────────────────────

bool APerf_QuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    int32 QIdx = FindQuestIndex(QuestID);
    if (QIdx == INDEX_NONE) return false;

    FPerf_QuestData& Quest = AllQuests[QIdx];
    if (Quest.State != EPerf_QuestState::Active) return false;

    for (FPerf_QuestObjective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.bCompleted = true;
            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' in quest '%s' COMPLETED."), *ObjectiveID, *QuestID);

            // Auto-complete quest if all objectives done
            if (AreAllObjectivesComplete(Quest))
            {
                CompleteQuest(QuestID);
            }
            return true;
        }
    }
    return false;
}

// ─── CompleteQuest ────────────────────────────────────────────────────────────

void APerf_QuestManager::CompleteQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return;

    FPerf_QuestData& Quest = AllQuests[Idx];
    Quest.State = EPerf_QuestState::Completed;

    if (ActiveQuestID == QuestID)
    {
        ActiveQuestID = TEXT("");
    }

    CompletedQuestIDs.AddUnique(QuestID);
    OnQuestCompleted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' COMPLETED."), *QuestID);
}

// ─── FailQuest ────────────────────────────────────────────────────────────────

void APerf_QuestManager::FailQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return;

    FPerf_QuestData& Quest = AllQuests[Idx];
    Quest.State = EPerf_QuestState::Failed;

    if (ActiveQuestID == QuestID)
    {
        ActiveQuestID = TEXT("");
    }

    OnQuestFailed.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' FAILED."), *QuestID);
}

// ─── GetQuestData ─────────────────────────────────────────────────────────────

bool APerf_QuestManager::GetQuestData(const FString& QuestID, FPerf_QuestData& OutData) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;
    OutData = AllQuests[Idx];
    return true;
}

// ─── IsQuestComplete ──────────────────────────────────────────────────────────

bool APerf_QuestManager::IsQuestComplete(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

// ─── CheckProximityObjectives ─────────────────────────────────────────────────

void APerf_QuestManager::CheckProximityObjectives(const FVector& PlayerLocation)
{
    if (ActiveQuestID.IsEmpty()) return;

    int32 Idx = FindQuestIndex(ActiveQuestID);
    if (Idx == INDEX_NONE) return;

    FPerf_QuestData& Quest = AllQuests[Idx];
    if (Quest.State != EPerf_QuestState::Active) return;

    for (FPerf_QuestObjective& Obj : Quest.Objectives)
    {
        if (Obj.bCompleted) continue;
        if (Obj.TargetLocation.IsZero()) continue; // No location target

        float Dist = FVector::Dist(PlayerLocation, Obj.TargetLocation);
        if (Dist <= Obj.ProximityRadius)
        {
            CompleteObjective(ActiveQuestID, Obj.ObjectiveID);
        }
    }
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

bool APerf_QuestManager::AreAllObjectivesComplete(const FPerf_QuestData& Quest) const
{
    for (const FPerf_QuestObjective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted) return false;
    }
    return Quest.Objectives.Num() > 0;
}

int32 APerf_QuestManager::FindQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < AllQuests.Num(); ++i)
    {
        if (AllQuests[i].QuestID == QuestID) return i;
    }
    return INDEX_NONE;
}
