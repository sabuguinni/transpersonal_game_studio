#include "SurvivalQuestManager.h"

ASurvivalQuestManager::ASurvivalQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxActiveQuests = 5;
    ElapsedTime = 0.0f;
}

void ASurvivalQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void ASurvivalQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    ElapsedTime += DeltaTime;

    // Check time-limited quests
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        FQuest_Definition& Quest = ActiveQuests[i];
        if (Quest.bHasTimeLimit && Quest.QuestState == EQuest_State::Active)
        {
            Quest.TimeLimit -= DeltaTime;
            if (Quest.TimeLimit <= 0.0f)
            {
                Quest.QuestState = EQuest_State::Failed;
            }
        }
    }
}

void ASurvivalQuestManager::InitializeDefaultQuests()
{
    RegisterRaptorPackHuntQuest();
    RegisterHerdMigrationWitnessQuest();
    RegisterStampedeSurvivalQuest();
}

void ASurvivalQuestManager::RegisterRaptorPackHuntQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = TEXT("QUEST_RAPTOR_HUNT_01");
    Quest.QuestName = TEXT("Raptor Pack Hunt");
    Quest.QuestDescription = TEXT("A raptor pack has been spotted near the eastern ridge. Track them, avoid their flanking maneuver, and escape to safety.");
    Quest.QuestType = EQuest_Type::Hunt;
    Quest.QuestState = EQuest_State::Inactive;
    Quest.bHasTimeLimit = true;
    Quest.TimeLimit = 180.0f; // 3 minutes

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_LOCATE_PACK");
    Obj1.Description = TEXT("Locate the raptor pack");
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bCompleted = false;
    Quest.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_AVOID_FLANKING");
    Obj2.Description = TEXT("Avoid the flanking maneuver");
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bCompleted = false;
    Quest.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("OBJ_ESCAPE_RAPTORS");
    Obj3.Description = TEXT("Escape to the safe zone");
    Obj3.RequiredCount = 1;
    Obj3.CurrentCount = 0;
    Obj3.bCompleted = false;
    Quest.Objectives.Add(Obj3);

    ActiveQuests.Add(Quest);
}

void ASurvivalQuestManager::RegisterHerdMigrationWitnessQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = TEXT("QUEST_HERD_MIGRATION_01");
    Quest.QuestName = TEXT("Herd Migration Witness");
    Quest.QuestDescription = TEXT("Follow the Brachiosaurus herd as it migrates across the valley. Observe all four waypoints to map the migration route.");
    Quest.QuestType = EQuest_Type::Exploration;
    Quest.QuestState = EQuest_State::Inactive;
    Quest.bHasTimeLimit = false;

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_WITNESS_WAYPOINT_A");
    Obj1.Description = TEXT("Observe the herd at Grazing Ground A");
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bCompleted = false;
    Quest.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_WITNESS_WAYPOINT_B");
    Obj2.Description = TEXT("Follow the herd to the river crossing");
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bCompleted = false;
    Quest.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("OBJ_WITNESS_WAYPOINT_C");
    Obj3.Description = TEXT("Track the herd to the highland meadow");
    Obj3.RequiredCount = 1;
    Obj3.CurrentCount = 0;
    Obj3.bCompleted = false;
    Quest.Objectives.Add(Obj3);

    FQuest_Objective Obj4;
    Obj4.ObjectiveID = TEXT("OBJ_WITNESS_WAYPOINT_D");
    Obj4.Description = TEXT("Observe the herd reach the northern plains");
    Obj4.RequiredCount = 1;
    Obj4.CurrentCount = 0;
    Obj4.bCompleted = false;
    Quest.Objectives.Add(Obj4);

    ActiveQuests.Add(Quest);
}

void ASurvivalQuestManager::RegisterStampedeSurvivalQuest()
{
    FQuest_Definition Quest;
    Quest.QuestID = TEXT("QUEST_STAMPEDE_SURVIVAL_01");
    Quest.QuestName = TEXT("Stampede Survival");
    Quest.QuestDescription = TEXT("The herd has been spooked and is stampeding. Survive the stampede zone and reach safety before being trampled.");
    Quest.QuestType = EQuest_Type::Survival;
    Quest.QuestState = EQuest_State::Inactive;
    Quest.bHasTimeLimit = true;
    Quest.TimeLimit = 60.0f; // 60 seconds

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("OBJ_SURVIVE_STAMPEDE");
    Obj1.Description = TEXT("Survive the stampede");
    Obj1.RequiredCount = 1;
    Obj1.CurrentCount = 0;
    Obj1.bCompleted = false;
    Quest.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("OBJ_REACH_HIGH_GROUND");
    Obj2.Description = TEXT("Reach high ground to escape the herd");
    Obj2.RequiredCount = 1;
    Obj2.CurrentCount = 0;
    Obj2.bCompleted = false;
    Quest.Objectives.Add(Obj2);

    ActiveQuests.Add(Quest);
}

bool ASurvivalQuestManager::ActivateQuest(const FString& QuestID)
{
    int32 Index = -1;
    if (!FindQuestIndex(QuestID, Index)) return false;

    if (ActiveQuests[Index].QuestState == EQuest_State::Inactive)
    {
        ActiveQuests[Index].QuestState = EQuest_State::Active;
        return true;
    }
    return false;
}

bool ASurvivalQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    int32 QuestIndex = -1;
    if (!FindQuestIndex(QuestID, QuestIndex)) return false;

    FQuest_Definition& Quest = ActiveQuests[QuestIndex];
    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount++;
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
            }
            CheckQuestCompletion(QuestIndex);
            return true;
        }
    }
    return false;
}

bool ASurvivalQuestManager::FailQuest(const FString& QuestID)
{
    int32 Index = -1;
    if (!FindQuestIndex(QuestID, Index)) return false;

    ActiveQuests[Index].QuestState = EQuest_State::Failed;
    return true;
}

bool ASurvivalQuestManager::CompleteQuest(const FString& QuestID)
{
    int32 Index = -1;
    if (!FindQuestIndex(QuestID, Index)) return false;

    FQuest_Definition CompletedQuest = ActiveQuests[Index];
    CompletedQuest.QuestState = EQuest_State::Completed;
    CompletedQuests.Add(CompletedQuest);
    ActiveQuests.RemoveAt(Index);
    return true;
}

FQuest_Definition ASurvivalQuestManager::GetQuestByID(const FString& QuestID) const
{
    for (const FQuest_Definition& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID) return Quest;
    }
    for (const FQuest_Definition& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID) return Quest;
    }
    return FQuest_Definition();
}

EQuest_State ASurvivalQuestManager::GetQuestState(const FString& QuestID) const
{
    for (const FQuest_Definition& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID) return Quest.QuestState;
    }
    for (const FQuest_Definition& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID) return Quest.QuestState;
    }
    return EQuest_State::Inactive;
}

TArray<FQuest_Definition> ASurvivalQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Definition> Result;
    for (const FQuest_Definition& Quest : ActiveQuests)
    {
        if (Quest.QuestState == EQuest_State::Active)
        {
            Result.Add(Quest);
        }
    }
    return Result;
}

void ASurvivalQuestManager::OnRaptorFlankingTriggered()
{
    ActivateQuest(TEXT("QUEST_RAPTOR_HUNT_01"));
    CompleteObjective(TEXT("QUEST_RAPTOR_HUNT_01"), TEXT("OBJ_LOCATE_PACK"));
}

void ASurvivalQuestManager::OnHerdStampedeTriggered()
{
    ActivateQuest(TEXT("QUEST_STAMPEDE_SURVIVAL_01"));
}

void ASurvivalQuestManager::OnPlayerEnteredHerdZone()
{
    ActivateQuest(TEXT("QUEST_HERD_MIGRATION_01"));
    CompleteObjective(TEXT("QUEST_HERD_MIGRATION_01"), TEXT("OBJ_WITNESS_WAYPOINT_A"));
}

void ASurvivalQuestManager::OnPlayerSurvivedStampede()
{
    CompleteObjective(TEXT("QUEST_STAMPEDE_SURVIVAL_01"), TEXT("OBJ_SURVIVE_STAMPEDE"));
}

void ASurvivalQuestManager::OnPlayerEscapedRaptors()
{
    CompleteObjective(TEXT("QUEST_RAPTOR_HUNT_01"), TEXT("OBJ_AVOID_FLANKING"));
    CompleteObjective(TEXT("QUEST_RAPTOR_HUNT_01"), TEXT("OBJ_ESCAPE_RAPTORS"));
}

bool ASurvivalQuestManager::FindQuestIndex(const FString& QuestID, int32& OutIndex) const
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            OutIndex = i;
            return true;
        }
    }
    return false;
}

void ASurvivalQuestManager::CheckQuestCompletion(int32 QuestIndex)
{
    if (!ActiveQuests.IsValidIndex(QuestIndex)) return;

    FQuest_Definition& Quest = ActiveQuests[QuestIndex];
    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllDone = false;
            break;
        }
    }
    if (bAllDone)
    {
        CompleteQuest(Quest.QuestID);
    }
}
