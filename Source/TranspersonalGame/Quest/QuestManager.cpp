// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// Transpersonal Game Studio — Prehistoric Survival Game

#include "QuestManager.h"
#include "Engine/World.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update elapsed time for time-limited quests
    for (FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.Status == EQuest_Status::Active && Quest.TimeLimit > 0.f)
        {
            Quest.ElapsedTime += DeltaTime;
            if (Quest.ElapsedTime >= Quest.TimeLimit)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}

void AQuestManager::InitializeDefaultQuests()
{
    ActiveQuests.Empty();

    // Quest 1: Raptor Hunt — track and eliminate raptor pack
    {
        FQuest_Data RaptorHunt;
        RaptorHunt.QuestID = TEXT("QUEST_RAPTOR_HUNT");
        RaptorHunt.QuestTitle = TEXT("Silence the Pack");
        RaptorHunt.QuestDescription = TEXT("A raptor pack has been spotted near the eastern ridge. Eliminate the threat before they reach camp.");
        RaptorHunt.Status = EQuest_Status::Inactive;
        RaptorHunt.TimeLimit = 0.f;

        FQuest_Objective TrackObj;
        TrackObj.ObjectiveID = TEXT("OBJ_TRACK_RAPTORS");
        TrackObj.Description = TEXT("Track the raptor pack (find 3 footprint sites)");
        TrackObj.ObjectiveType = EQuest_ObjectiveType::Explore;
        TrackObj.RequiredCount = 3;
        TrackObj.CurrentCount = 0;
        TrackObj.bCompleted = false;

        FQuest_Objective HuntObj;
        HuntObj.ObjectiveID = TEXT("OBJ_HUNT_RAPTORS");
        HuntObj.Description = TEXT("Eliminate the raptor pack (kill 3 raptors)");
        HuntObj.ObjectiveType = EQuest_ObjectiveType::Hunt;
        HuntObj.RequiredCount = 3;
        HuntObj.CurrentCount = 0;
        HuntObj.bCompleted = false;

        RaptorHunt.Objectives.Add(TrackObj);
        RaptorHunt.Objectives.Add(HuntObj);
        ActiveQuests.Add(RaptorHunt);
    }

    // Quest 2: Flint Gathering — collect flint from river basin
    {
        FQuest_Data FlintGather;
        FlintGather.QuestID = TEXT("QUEST_FLINT_GATHER");
        FlintGather.QuestTitle = TEXT("Stone for Survival");
        FlintGather.QuestDescription = TEXT("The camp needs flint to craft tools and weapons. Gather enough from the river basin before the storm arrives.");
        FlintGather.Status = EQuest_Status::Inactive;
        FlintGather.TimeLimit = 600.f; // 10 minutes — storm is coming

        FQuest_Objective GatherObj;
        GatherObj.ObjectiveID = TEXT("OBJ_GATHER_FLINT");
        GatherObj.Description = TEXT("Gather flint from the river basin (collect 5 pieces)");
        GatherObj.ObjectiveType = EQuest_ObjectiveType::Gather;
        GatherObj.RequiredCount = 5;
        GatherObj.CurrentCount = 0;
        GatherObj.bCompleted = false;

        FQuest_Objective ReturnObj;
        ReturnObj.ObjectiveID = TEXT("OBJ_RETURN_CAMP");
        ReturnObj.Description = TEXT("Return to camp before the storm");
        ReturnObj.ObjectiveType = EQuest_ObjectiveType::Survive;
        ReturnObj.RequiredCount = 1;
        ReturnObj.CurrentCount = 0;
        ReturnObj.bCompleted = false;

        FlintGather.Objectives.Add(GatherObj);
        FlintGather.Objectives.Add(ReturnObj);
        ActiveQuests.Add(FlintGather);
    }

    // Quest 3: Camp Defense — protect camp from predator attack
    {
        FQuest_Data CampDefense;
        CampDefense.QuestID = TEXT("QUEST_CAMP_DEFENSE");
        CampDefense.QuestTitle = TEXT("Hold the Line");
        CampDefense.QuestDescription = TEXT("Predators are circling the camp at nightfall. Defend the camp until dawn.");
        CampDefense.Status = EQuest_Status::Inactive;
        CampDefense.TimeLimit = 0.f;

        FQuest_Objective BuildFireObj;
        BuildFireObj.ObjectiveID = TEXT("OBJ_BUILD_FIRE");
        BuildFireObj.Description = TEXT("Build a campfire to deter predators");
        BuildFireObj.ObjectiveType = EQuest_ObjectiveType::Craft;
        BuildFireObj.RequiredCount = 1;
        BuildFireObj.CurrentCount = 0;
        BuildFireObj.bCompleted = false;

        FQuest_Objective SurviveNightObj;
        SurviveNightObj.ObjectiveID = TEXT("OBJ_SURVIVE_NIGHT");
        SurviveNightObj.Description = TEXT("Survive until dawn (repel 5 predator attacks)");
        SurviveNightObj.ObjectiveType = EQuest_ObjectiveType::Defend;
        SurviveNightObj.RequiredCount = 5;
        SurviveNightObj.CurrentCount = 0;
        SurviveNightObj.bCompleted = false;

        CampDefense.Objectives.Add(BuildFireObj);
        CampDefense.Objectives.Add(SurviveNightObj);
        ActiveQuests.Add(CampDefense);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized %d default quests"), ActiveQuests.Num());
}

bool AQuestManager::StartQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest not found: %s"), *QuestID);
        return false;
    }

    if (Quest->Status != EQuest_Status::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is not inactive (status=%d)"), *QuestID, (int32)Quest->Status);
        return false;
    }

    Quest->Status = EQuest_Status::Active;
    Quest->ElapsedTime = 0.f;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest: %s"), *Quest->QuestTitle);
    return true;
}

void AQuestManager::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    FQuest_Data* Quest = FindQuest(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return;
    }

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective completed: %s"), *Obj.Description);
            }
            break;
        }
    }

    CheckQuestCompletion(*Quest);
}

bool AQuestManager::IsQuestComplete(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.Status == EQuest_Status::Completed;
        }
    }
    return false;
}

void AQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuest(QuestID);
    if (Quest && Quest->Status == EQuest_Status::Active)
    {
        Quest->Status = EQuest_Status::Failed;
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest failed: %s"), *Quest->QuestTitle);
    }
}

EQuest_Status AQuestManager::GetQuestStatus(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.Status;
        }
    }
    return EQuest_Status::Inactive;
}

void AQuestManager::CheckQuestCompletion(FQuest_Data& Quest)
{
    if (Quest.Status != EQuest_Status::Active)
    {
        return;
    }

    bool bAllComplete = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        Quest.Status = EQuest_Status::Completed;
        CompletedQuestIDs.Add(Quest.QuestID);
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest completed: %s"), *Quest.QuestTitle);
    }
}

FQuest_Data* AQuestManager::FindQuest(const FString& QuestID)
{
    for (FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}
