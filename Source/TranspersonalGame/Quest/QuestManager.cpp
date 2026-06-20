// QuestManager.cpp
// Quest & Mission Designer — Agent #14
// Transpersonal Game Studio

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

// ============================================================
// UQuestManager — WorldSubsystem
// ============================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ActiveQuests.Empty();
    CompletedQuestIDs.Empty();
    FailedQuestIDs.Empty();

    // Register all available quests on startup
    RegisterDefaultQuests();

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized with %d quests registered."), AvailableQuests.Num());
}

void UQuestManager::Deinitialize()
{
    ActiveQuests.Empty();
    AvailableQuests.Empty();
    Super::Deinitialize();
}

void UQuestManager::RegisterDefaultQuests()
{
    // ---- QUEST 1: First Hunt ----
    FQuest_Descriptor Hunt;
    Hunt.QuestID = FName("Q_FirstHunt");
    Hunt.Title = FText::FromString("First Blood");
    Hunt.Description = FText::FromString("Hunt a small dinosaur near the river to secure food before nightfall. Track it, close the distance, and make the kill.");
    Hunt.QuestType = EQuest_Type::Hunting;
    Hunt.Difficulty = EQuest_Difficulty::Easy;
    Hunt.bIsMainQuest = true;
    Hunt.bIsRepeatable = false;
    Hunt.RewardFood = 40.0f;
    Hunt.RewardXP = 100.0f;

    FQuest_Objective HuntObj;
    HuntObj.ObjectiveID = FName("OBJ_KillSmallDino");
    HuntObj.Description = FText::FromString("Kill 1 small dinosaur near the river");
    HuntObj.TargetCount = 1;
    HuntObj.CurrentCount = 0;
    HuntObj.bIsCompleted = false;
    Hunt.Objectives.Add(HuntObj);

    AvailableQuests.Add(Hunt.QuestID, Hunt);

    // ---- QUEST 2: Gather Firewood ----
    FQuest_Descriptor Fire;
    Fire.QuestID = FName("Q_GatherFirewood");
    Fire.Title = FText::FromString("Against the Cold");
    Fire.Description = FText::FromString("Gather sticks and stones to build a campfire. Without fire, the night will kill you as surely as any predator.");
    Fire.QuestType = EQuest_Type::Crafting;
    Fire.Difficulty = EQuest_Difficulty::Easy;
    Fire.bIsMainQuest = true;
    Fire.bIsRepeatable = false;
    Fire.RewardFood = 0.0f;
    Fire.RewardXP = 75.0f;

    FQuest_Objective GatherSticks;
    GatherSticks.ObjectiveID = FName("OBJ_GatherSticks");
    GatherSticks.Description = FText::FromString("Gather 3 sticks");
    GatherSticks.TargetCount = 3;
    GatherSticks.CurrentCount = 0;
    GatherSticks.bIsCompleted = false;
    Fire.Objectives.Add(GatherSticks);

    FQuest_Objective GatherRocks;
    GatherRocks.ObjectiveID = FName("OBJ_GatherRocks");
    GatherRocks.Description = FText::FromString("Gather 2 rocks");
    GatherRocks.TargetCount = 2;
    GatherRocks.CurrentCount = 0;
    GatherRocks.bIsCompleted = false;
    Fire.Objectives.Add(GatherRocks);

    FQuest_Objective BuildCampfire;
    BuildCampfire.ObjectiveID = FName("OBJ_BuildCampfire");
    BuildCampfire.Description = FText::FromString("Build a campfire at the campsite");
    BuildCampfire.TargetCount = 1;
    BuildCampfire.CurrentCount = 0;
    BuildCampfire.bIsCompleted = false;
    Fire.Objectives.Add(BuildCampfire);

    AvailableQuests.Add(Fire.QuestID, Fire);

    // ---- QUEST 3: Find Water ----
    FQuest_Descriptor Water;
    Water.QuestID = FName("Q_FindWater");
    Water.Title = FText::FromString("Thirst");
    Water.Description = FText::FromString("Your throat burns. Find the river to the south and drink. Watch for predators — the river is a dangerous place.");
    Water.QuestType = EQuest_Type::Exploration;
    Water.Difficulty = EQuest_Difficulty::Easy;
    Water.bIsMainQuest = true;
    Water.bIsRepeatable = false;
    Water.RewardFood = 0.0f;
    Water.RewardXP = 50.0f;

    FQuest_Objective ReachRiver;
    ReachRiver.ObjectiveID = FName("OBJ_ReachRiver");
    ReachRiver.Description = FText::FromString("Reach the river crossing to the south");
    ReachRiver.TargetCount = 1;
    ReachRiver.CurrentCount = 0;
    ReachRiver.bIsCompleted = false;
    Water.Objectives.Add(ReachRiver);

    FQuest_Objective DrinkWater;
    DrinkWater.ObjectiveID = FName("OBJ_DrinkWater");
    DrinkWater.Description = FText::FromString("Drink from the river");
    DrinkWater.TargetCount = 1;
    DrinkWater.CurrentCount = 0;
    DrinkWater.bIsCompleted = false;
    Water.Objectives.Add(DrinkWater);

    AvailableQuests.Add(Water.QuestID, Water);

    // ---- QUEST 4: Survive the Night ----
    FQuest_Descriptor Night;
    Night.QuestID = FName("Q_SurviveNight");
    Night.Title = FText::FromString("First Night");
    Night.Description = FText::FromString("Survive until dawn. Predators are more active at night. Stay near your fire, keep your weapon ready, and do not stray far.");
    Night.QuestType = EQuest_Type::Survival;
    Night.Difficulty = EQuest_Difficulty::Medium;
    Night.bIsMainQuest = true;
    Night.bIsRepeatable = false;
    Night.RewardFood = 0.0f;
    Night.RewardXP = 200.0f;

    FQuest_Objective StayAlive;
    StayAlive.ObjectiveID = FName("OBJ_SurviveToDawn");
    StayAlive.Description = FText::FromString("Survive until dawn (5 in-game minutes)");
    StayAlive.TargetCount = 1;
    StayAlive.CurrentCount = 0;
    StayAlive.bIsCompleted = false;
    Night.Objectives.Add(StayAlive);

    AvailableQuests.Add(Night.QuestID, Night);

    // ---- QUEST 5: Track the Herd ----
    FQuest_Descriptor Herd;
    Herd.QuestID = FName("Q_TrackHerd");
    Herd.Title = FText::FromString("Follow the Herd");
    Herd.Description = FText::FromString("A herd of Triceratops has been spotted moving north. Follow their migration corridor — where herds travel, resources follow. But so do predators.");
    Herd.QuestType = EQuest_Type::Exploration;
    Herd.Difficulty = EQuest_Difficulty::Medium;
    Herd.bIsMainQuest = false;
    Herd.bIsRepeatable = false;
    Herd.RewardFood = 20.0f;
    Herd.RewardXP = 150.0f;

    FQuest_Objective FindHerd;
    FindHerd.ObjectiveID = FName("OBJ_LocateHerd");
    FindHerd.Description = FText::FromString("Locate the Triceratops herd");
    FindHerd.TargetCount = 1;
    FindHerd.CurrentCount = 0;
    FindHerd.bIsCompleted = false;
    Herd.Objectives.Add(FindHerd);

    FQuest_Objective FollowCorridor;
    FollowCorridor.ObjectiveID = FName("OBJ_FollowMigrationCorridor");
    FollowCorridor.Description = FText::FromString("Follow the herd to the northern migration corridor");
    FollowCorridor.TargetCount = 1;
    FollowCorridor.CurrentCount = 0;
    FollowCorridor.bIsCompleted = false;
    Herd.Objectives.Add(FollowCorridor);

    AvailableQuests.Add(Herd.QuestID, Herd);
}

bool UQuestManager::StartQuest(FName QuestID)
{
    if (!AvailableQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' not found in available quests."), *QuestID.ToString());
        return false;
    }

    if (ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' already active."), *QuestID.ToString());
        return false;
    }

    if (CompletedQuestIDs.Contains(QuestID))
    {
        const FQuest_Descriptor& QDesc = AvailableQuests[QuestID];
        if (!QDesc.bIsRepeatable)
        {
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' already completed and not repeatable."), *QuestID.ToString());
            return false;
        }
    }

    FQuest_Descriptor QuestCopy = AvailableQuests[QuestID];
    QuestCopy.State = EQuest_State::Active;
    ActiveQuests.Add(QuestID, QuestCopy);

    OnQuestStarted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' started."), *QuestID.ToString());
    return true;
}

bool UQuestManager::UpdateObjective(FName QuestID, FName ObjectiveID, int32 CountDelta)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_Descriptor& Quest = ActiveQuests[QuestID];
    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + CountDelta, 0, Obj.TargetCount);
            if (Obj.CurrentCount >= Obj.TargetCount)
            {
                Obj.bIsCompleted = true;
                OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' in quest '%s' completed."), *ObjectiveID.ToString(), *QuestID.ToString());
            }

            // Check if all objectives done
            bool bAllDone = true;
            for (const FQuest_Objective& O : Quest.Objectives)
            {
                if (!O.bIsCompleted) { bAllDone = false; break; }
            }
            if (bAllDone)
            {
                CompleteQuest(QuestID);
            }
            return true;
        }
    }
    return false;
}

bool UQuestManager::CompleteQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_Descriptor& Quest = ActiveQuests[QuestID];
    Quest.State = EQuest_State::Completed;
    CompletedQuestIDs.Add(QuestID);
    ActiveQuests.Remove(QuestID);

    OnQuestCompleted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' completed! XP Reward: %.0f"), *QuestID.ToString(), Quest.RewardXP);
    return true;
}

bool UQuestManager::FailQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_Descriptor& Quest = ActiveQuests[QuestID];
    Quest.State = EQuest_State::Failed;
    FailedQuestIDs.Add(QuestID);
    ActiveQuests.Remove(QuestID);

    OnQuestFailed.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' failed."), *QuestID.ToString());
    return true;
}

TArray<FQuest_Descriptor> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Descriptor> Result;
    for (const auto& Pair : ActiveQuests)
    {
        Result.Add(Pair.Value);
    }
    return Result;
}

bool UQuestManager::IsQuestActive(FName QuestID) const
{
    return ActiveQuests.Contains(QuestID);
}

bool UQuestManager::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

void UQuestManager::NotifyZoneEntered(FName ZoneID)
{
    // Map zone IDs to quest objective triggers
    if (ZoneID == FName("QuestZone_RiverCrossing"))
    {
        UpdateObjective(FName("Q_FindWater"), FName("OBJ_ReachRiver"), 1);
    }
    else if (ZoneID == FName("MigrationCorridor_North"))
    {
        UpdateObjective(FName("Q_TrackHerd"), FName("OBJ_FollowMigrationCorridor"), 1);
    }
    else if (ZoneID == FName("HerbivoreHerd_GrazingZone_A") || ZoneID == FName("HerbivoreHerd_GrazingZone_B"))
    {
        UpdateObjective(FName("Q_TrackHerd"), FName("OBJ_LocateHerd"), 1);
    }
    else if (ZoneID == FName("QuestZone_CampfireSite"))
    {
        // Campfire site entered — check if player has materials to build
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Player entered campfire site."));
    }
}

void UQuestManager::NotifyKill(FName CreatureType)
{
    if (CreatureType == FName("SmallDino") || CreatureType == FName("Raptor") || CreatureType == FName("Gallimimus"))
    {
        UpdateObjective(FName("Q_FirstHunt"), FName("OBJ_KillSmallDino"), 1);
    }
}

void UQuestManager::NotifyResourceGathered(FName ResourceType, int32 Count)
{
    if (ResourceType == FName("Stick"))
    {
        UpdateObjective(FName("Q_GatherFirewood"), FName("OBJ_GatherSticks"), Count);
    }
    else if (ResourceType == FName("Rock"))
    {
        UpdateObjective(FName("Q_GatherFirewood"), FName("OBJ_GatherRocks"), Count);
    }
}

void UQuestManager::NotifyCraftingCompleted(FName RecipeID)
{
    if (RecipeID == FName("Recipe_Campfire"))
    {
        UpdateObjective(FName("Q_GatherFirewood"), FName("OBJ_BuildCampfire"), 1);
    }
}
