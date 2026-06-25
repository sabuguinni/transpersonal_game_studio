#include "QuestManager.h"
#include "Engine/World.h"

// ============================================================
// AQuestManager — Implementation
// ============================================================

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void AQuestManager::InitializeDefaultQuests()
{
    QuestDatabase.Empty();

    // -------------------------------------------------------
    // QUEST 1: "First Blood" — Hunt a Velociraptor
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_HUNT_RAPTOR_01");
        Q.QuestTitle = TEXT("First Blood");
        Q.QuestDescription = TEXT("A pack of Velociraptors has been raiding the camp's food stores. Hunt one down to drive the pack away.");
        Q.Status = EQuest_Status::Inactive;
        Q.bIsMainQuest = false;
        Q.GiverNPCLabel = TEXT("TribeElder_QuestGiver");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_KILL_RAPTOR");
        Obj1.Description = TEXT("Kill a Velociraptor (0/1)");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_RETURN_CAMP");
        Obj2.Description = TEXT("Return to camp");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        Q.Reward.ResourceFood = 5;
        Q.Reward.ResourceRocks = 2;
        Q.Reward.HealthBonus = 20.0f;

        QuestDatabase.Add(Q);
    }

    // -------------------------------------------------------
    // QUEST 2: "Stone Tools" — Gather crafting materials
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_CRAFT_STONE_AXE");
        Q.QuestTitle = TEXT("Stone Tools");
        Q.QuestDescription = TEXT("Craft a stone axe to improve your chances of survival. Gather rocks and sticks from the forest floor.");
        Q.Status = EQuest_Status::Inactive;
        Q.bIsMainQuest = true;
        Q.GiverNPCLabel = TEXT("TribeElder_QuestGiver");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_GATHER_ROCKS");
        Obj1.Description = TEXT("Gather rocks (0/2)");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.RequiredCount = 2;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_GATHER_STICKS");
        Obj2.Description = TEXT("Gather sticks (0/1)");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_CRAFT_AXE");
        Obj3.Description = TEXT("Craft a Stone Axe at the crafting stone");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bIsCompleted = false;
        Q.Objectives.Add(Obj3);

        Q.Reward.ResourceRocks = 3;
        Q.Reward.ResourceSticks = 2;
        Q.Reward.HealthBonus = 10.0f;

        QuestDatabase.Add(Q);
    }

    // -------------------------------------------------------
    // QUEST 3: "River Crossing" — Explore the eastern valley
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_EXPLORE_RIVER");
        Q.QuestTitle = TEXT("River Crossing");
        Q.QuestDescription = TEXT("The tribe needs to know what lies beyond the eastern river. Scout the valley and return alive.");
        Q.Status = EQuest_Status::Inactive;
        Q.bIsMainQuest = true;
        Q.GiverNPCLabel = TEXT("Scout_NPC_QuestLine");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_REACH_RIVER");
        Obj1.Description = TEXT("Reach the eastern river");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_CROSS_RIVER");
        Obj2.Description = TEXT("Cross the river without being detected by the T-Rex");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_SCOUT_VALLEY");
        Obj3.Description = TEXT("Scout the eastern valley (0/3 areas)");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj3.RequiredCount = 3;
        Obj3.CurrentCount = 0;
        Obj3.bIsCompleted = false;
        Q.Objectives.Add(Obj3);

        FQuest_Objective Obj4;
        Obj4.ObjectiveID = TEXT("OBJ_RETURN_ALIVE");
        Obj4.Description = TEXT("Return to camp alive");
        Obj4.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj4.RequiredCount = 1;
        Obj4.CurrentCount = 0;
        Obj4.bIsCompleted = false;
        Q.Objectives.Add(Obj4);

        Q.Reward.ResourceFood = 8;
        Q.Reward.ResourceRocks = 4;
        Q.Reward.ResourceSticks = 3;
        Q.Reward.HealthBonus = 30.0f;

        QuestDatabase.Add(Q);
    }

    // -------------------------------------------------------
    // QUEST 4: "Defend the Camp" — Survive a predator attack
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_DEFEND_CAMP");
        Q.QuestTitle = TEXT("Defend the Camp");
        Q.QuestDescription = TEXT("A Triceratops is charging toward the camp. Drive it away before it destroys the food stores.");
        Q.Status = EQuest_Status::Inactive;
        Q.bIsMainQuest = false;
        Q.GiverNPCLabel = TEXT("TribeElder_QuestGiver");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_DRIVE_TRIKE");
        Obj1.Description = TEXT("Drive away the Triceratops");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Defend;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_PROTECT_STORES");
        Obj2.Description = TEXT("Keep food stores above 50% health");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Defend;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        Q.Reward.ResourceFood = 10;
        Q.Reward.HealthBonus = 25.0f;

        QuestDatabase.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized %d quests"), QuestDatabase.Num());
}

bool AQuestManager::StartQuest(const FString& QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest not found: %s"), *QuestID);
        return false;
    }

    if (QuestDatabase[Idx].Status != EQuest_Status::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest already started or finished: %s"), *QuestID);
        return false;
    }

    QuestDatabase[Idx].Status = EQuest_Status::Active;
    ActiveQuests.Add(QuestDatabase[Idx]);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest: %s"), *QuestDatabase[Idx].QuestTitle);
    return true;
}

void AQuestManager::AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    int32 ActiveIdx = FindActiveQuestIndex(QuestID);
    if (ActiveIdx == INDEX_NONE)
    {
        return;
    }

    FQuest_Data& Quest = ActiveQuests[ActiveIdx];
    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective completed: %s"), *Obj.Description);
            }
            break;
        }
    }

    // Check if all objectives done
    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone)
    {
        CompleteQuest(QuestID);
    }
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    int32 ActiveIdx = FindActiveQuestIndex(QuestID);
    if (ActiveIdx == INDEX_NONE)
    {
        return false;
    }

    ActiveQuests[ActiveIdx].Status = EQuest_Status::Completed;

    // Sync back to database
    int32 DbIdx = FindQuestIndex(QuestID);
    if (DbIdx != INDEX_NONE)
    {
        QuestDatabase[DbIdx].Status = EQuest_Status::Completed;
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest completed: %s"), *ActiveQuests[ActiveIdx].QuestTitle);
    return true;
}

void AQuestManager::FailQuest(const FString& QuestID)
{
    int32 ActiveIdx = FindActiveQuestIndex(QuestID);
    if (ActiveIdx == INDEX_NONE)
    {
        return;
    }

    ActiveQuests[ActiveIdx].Status = EQuest_Status::Failed;

    int32 DbIdx = FindQuestIndex(QuestID);
    if (DbIdx != INDEX_NONE)
    {
        QuestDatabase[DbIdx].Status = EQuest_Status::Failed;
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest failed: %s"), *ActiveQuests[ActiveIdx].QuestTitle);
}

bool AQuestManager::GetQuestData(const FString& QuestID, FQuest_Data& OutData)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE)
    {
        return false;
    }
    OutData = QuestDatabase[Idx];
    return true;
}

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    return FindActiveQuestIndex(QuestID) != INDEX_NONE;
}

int32 AQuestManager::FindQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < QuestDatabase.Num(); ++i)
    {
        if (QuestDatabase[i].QuestID == QuestID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 AQuestManager::FindActiveQuestIndex(const FString& QuestID) const
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
