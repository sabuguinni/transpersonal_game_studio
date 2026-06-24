// QuestManager.cpp
// Quest & Narrative Manager — Transpersonal Game Studio
// Agent #15 — Narrative & Dialogue Agent

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// ============================================================
// UNarr_QuestManager — Implementation
// ============================================================

UNarr_QuestManager::UNarr_QuestManager()
{
    ActiveQuestIndex = -1;
    bQuestSystemInitialized = false;
}

void UNarr_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeQuestDatabase();
    bQuestSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Initialized with %d quests"), AllQuests.Num());
}

void UNarr_QuestManager::Deinitialize()
{
    AllQuests.Empty();
    ActiveQuests.Empty();
    CompletedQuestIDs.Empty();
    Super::Deinitialize();
}

void UNarr_QuestManager::InitializeQuestDatabase()
{
    // Quest 1 — Tutorial: First Hunt
    FNarr_QuestData FirstHunt;
    FirstHunt.QuestID = FName("Q_FirstHunt");
    FirstHunt.QuestTitle = FText::FromString("First Blood");
    FirstHunt.QuestDescription = FText::FromString(
        "The tribe is starving. Track down a small herbivore and bring back meat before nightfall. "
        "Avoid the larger predators — you are not ready for them yet."
    );
    FirstHunt.QuestType = ENarr_QuestType::Hunt;
    FirstHunt.bIsMainQuest = false;
    FirstHunt.bIsRepeatable = false;
    FirstHunt.RequiredLevel = 1;
    FirstHunt.RewardExperience = 50;
    FirstHunt.RewardResources.Add(FName("Meat"), 3);
    FirstHunt.RewardResources.Add(FName("Hide"), 1);

    FNarr_QuestObjective HuntObjective;
    HuntObjective.ObjectiveID = FName("OBJ_KillHerbivore");
    HuntObjective.Description = FText::FromString("Kill a small herbivore (0/1)");
    HuntObjective.TargetCount = 1;
    HuntObjective.CurrentCount = 0;
    HuntObjective.bIsCompleted = false;
    HuntObjective.TargetTag = FName("Herbivore_Small");
    FirstHunt.Objectives.Add(HuntObjective);

    FNarr_QuestObjective ReturnObjective;
    ReturnObjective.ObjectiveID = FName("OBJ_ReturnToCamp");
    ReturnObjective.Description = FText::FromString("Return to the camp");
    ReturnObjective.TargetCount = 1;
    ReturnObjective.CurrentCount = 0;
    ReturnObjective.bIsCompleted = false;
    ReturnObjective.TargetTag = FName("Camp_Main");
    FirstHunt.Objectives.Add(ReturnObjective);

    AllQuests.Add(FirstHunt);

    // Quest 2 — Main Story: The Migration
    FNarr_QuestData Migration;
    Migration.QuestID = FName("Q_Migration");
    Migration.QuestTitle = FText::FromString("The Great Migration");
    Migration.QuestDescription = FText::FromString(
        "The elder has seen signs — the great herds are moving north. "
        "Scout the migration route and find a safe passage before the predators follow."
    );
    Migration.QuestType = ENarr_QuestType::Explore;
    Migration.bIsMainQuest = true;
    Migration.bIsRepeatable = false;
    Migration.RequiredLevel = 3;
    Migration.RewardExperience = 200;
    Migration.RewardResources.Add(FName("Flint"), 5);
    Migration.RewardResources.Add(FName("Bone"), 3);

    FNarr_QuestObjective ScoutObj;
    ScoutObj.ObjectiveID = FName("OBJ_ScoutNorth");
    ScoutObj.Description = FText::FromString("Reach the northern ridge (0/1)");
    ScoutObj.TargetCount = 1;
    ScoutObj.CurrentCount = 0;
    ScoutObj.bIsCompleted = false;
    ScoutObj.TargetTag = FName("Location_NorthRidge");
    Migration.Objectives.Add(ScoutObj);

    FNarr_QuestObjective ObserveObj;
    ObserveObj.ObjectiveID = FName("OBJ_ObserveDinos");
    ObserveObj.Description = FText::FromString("Observe the migrating herd (0/3)");
    ObserveObj.TargetCount = 3;
    ObserveObj.CurrentCount = 0;
    ObserveObj.bIsCompleted = false;
    ObserveObj.TargetTag = FName("Dinosaur_Herd");
    Migration.Objectives.Add(ObserveObj);

    AllQuests.Add(Migration);

    // Quest 3 — Survival: Winter Preparation
    FNarr_QuestData WinterPrep;
    WinterPrep.QuestID = FName("Q_WinterPrep");
    WinterPrep.QuestTitle = FText::FromString("Before the Cold");
    WinterPrep.QuestDescription = FText::FromString(
        "The nights are getting longer. Gather enough resources to survive the cold season. "
        "The tribe needs food stores, warm hides, and a reinforced shelter."
    );
    WinterPrep.QuestType = ENarr_QuestType::Gather;
    WinterPrep.bIsMainQuest = false;
    WinterPrep.bIsRepeatable = false;
    WinterPrep.RequiredLevel = 5;
    WinterPrep.RewardExperience = 300;
    WinterPrep.RewardResources.Add(FName("Meat"), 10);
    WinterPrep.RewardResources.Add(FName("Hide"), 8);
    WinterPrep.RewardResources.Add(FName("Wood"), 15);

    FNarr_QuestObjective GatherMeat;
    GatherMeat.ObjectiveID = FName("OBJ_GatherMeat");
    GatherMeat.Description = FText::FromString("Gather meat stores (0/10)");
    GatherMeat.TargetCount = 10;
    GatherMeat.CurrentCount = 0;
    GatherMeat.bIsCompleted = false;
    GatherMeat.TargetTag = FName("Resource_Meat");
    WinterPrep.Objectives.Add(GatherMeat);

    FNarr_QuestObjective GatherHide;
    GatherHide.ObjectiveID = FName("OBJ_GatherHide");
    GatherHide.Description = FText::FromString("Collect thick hides (0/5)");
    GatherHide.TargetCount = 5;
    GatherHide.CurrentCount = 0;
    GatherHide.bIsCompleted = false;
    GatherHide.TargetTag = FName("Resource_Hide");
    WinterPrep.Objectives.Add(GatherHide);

    AllQuests.Add(WinterPrep);

    // Quest 4 — Combat: Raptor Threat
    FNarr_QuestData RaptorThreat;
    RaptorThreat.QuestID = FName("Q_RaptorThreat");
    RaptorThreat.QuestTitle = FText::FromString("Pack Hunters");
    RaptorThreat.QuestDescription = FText::FromString(
        "A raptor pack has moved into the valley and is attacking the tribe's hunting grounds. "
        "Drive them away or eliminate the pack leader to scatter the group."
    );
    RaptorThreat.QuestType = ENarr_QuestType::Eliminate;
    RaptorThreat.bIsMainQuest = false;
    RaptorThreat.bIsRepeatable = false;
    RaptorThreat.RequiredLevel = 7;
    RaptorThreat.RewardExperience = 400;
    RaptorThreat.RewardResources.Add(FName("RaptorClaw"), 3);
    RaptorThreat.RewardResources.Add(FName("Meat"), 5);

    FNarr_QuestObjective KillRaptors;
    KillRaptors.ObjectiveID = FName("OBJ_KillRaptors");
    KillRaptors.Description = FText::FromString("Eliminate raptors (0/4)");
    KillRaptors.TargetCount = 4;
    KillRaptors.CurrentCount = 0;
    KillRaptors.bIsCompleted = false;
    KillRaptors.TargetTag = FName("Dinosaur_Raptor");
    RaptorThreat.Objectives.Add(KillRaptors);

    AllQuests.Add(RaptorThreat);

    UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Database initialized with %d quests"), AllQuests.Num());
}

bool UNarr_QuestManager::StartQuest(FName QuestID)
{
    // Check if already active or completed
    if (CompletedQuestIDs.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("NarrQuestManager: Quest %s already completed"), *QuestID.ToString());
        return false;
    }

    for (const FNarr_QuestData& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("NarrQuestManager: Quest %s already active"), *QuestID.ToString());
            return false;
        }
    }

    // Find quest in database
    for (FNarr_QuestData& Q : AllQuests)
    {
        if (Q.QuestID == QuestID)
        {
            Q.QuestState = ENarr_QuestState::Active;
            ActiveQuests.Add(Q);
            OnQuestStarted.Broadcast(Q);
            UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Started quest '%s'"), *Q.QuestTitle.ToString());
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("NarrQuestManager: Quest %s not found in database"), *QuestID.ToString());
    return false;
}

bool UNarr_QuestManager::UpdateObjective(FName QuestID, FName ObjectiveID, int32 Progress)
{
    for (FNarr_QuestData& AQ : ActiveQuests)
    {
        if (AQ.QuestID != QuestID) continue;

        for (FNarr_QuestObjective& Obj : AQ.Objectives)
        {
            if (Obj.ObjectiveID != ObjectiveID) continue;
            if (Obj.bIsCompleted) continue;

            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + Progress, 0, Obj.TargetCount);

            if (Obj.CurrentCount >= Obj.TargetCount)
            {
                Obj.bIsCompleted = true;
                OnObjectiveCompleted.Broadcast(AQ, Obj);
                UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Objective '%s' completed in quest '%s'"),
                    *ObjectiveID.ToString(), *QuestID.ToString());

                // Check if all objectives done
                CheckQuestCompletion(AQ);
            }
            return true;
        }
    }
    return false;
}

void UNarr_QuestManager::CheckQuestCompletion(FNarr_QuestData& Quest)
{
    for (const FNarr_QuestObjective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted) return;
    }

    // All objectives complete
    Quest.QuestState = ENarr_QuestState::Completed;
    CompletedQuestIDs.Add(Quest.QuestID);
    OnQuestCompleted.Broadcast(Quest);
    UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Quest '%s' COMPLETED!"), *Quest.QuestTitle.ToString());
}

bool UNarr_QuestManager::AbandonQuest(FName QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); ++i)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            FNarr_QuestData Abandoned = ActiveQuests[i];
            Abandoned.QuestState = ENarr_QuestState::Failed;
            ActiveQuests.RemoveAt(i);
            OnQuestFailed.Broadcast(Abandoned);
            UE_LOG(LogTemp, Log, TEXT("NarrQuestManager: Quest '%s' abandoned"), *QuestID.ToString());
            return true;
        }
    }
    return false;
}

TArray<FNarr_QuestData> UNarr_QuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

TArray<FNarr_QuestData> UNarr_QuestManager::GetAvailableQuests(int32 PlayerLevel) const
{
    TArray<FNarr_QuestData> Available;
    for (const FNarr_QuestData& Q : AllQuests)
    {
        if (Q.RequiredLevel <= PlayerLevel &&
            Q.QuestState == ENarr_QuestState::NotStarted &&
            !CompletedQuestIDs.Contains(Q.QuestID))
        {
            Available.Add(Q);
        }
    }
    return Available;
}

bool UNarr_QuestManager::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

bool UNarr_QuestManager::IsQuestActive(FName QuestID) const
{
    for (const FNarr_QuestData& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestID) return true;
    }
    return false;
}
