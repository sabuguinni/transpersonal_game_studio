// QuestManager.cpp
// Quest & Mission Designer — Agent #14
// Survival quest system for prehistoric world

#include "QuestManager.h"

UQuestManager::UQuestManager()
{
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitDefaultQuests();
}

void UQuestManager::Deinitialize()
{
    QuestRegistry.Empty();
    Super::Deinitialize();
}

// ============================================================
// Default Survival Quests — registered at world init
// ============================================================

void UQuestManager::InitDefaultQuests()
{
    RegisterDefaultSurvivalQuests();
}

void UQuestManager::RegisterDefaultSurvivalQuests()
{
    // --------------------------------------------------------
    // QUEST 1: Find Food for the Tribe
    // Type: Gather | Giver: TribalElder
    // --------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_FindFoodForTribe");
        Q.QuestName = TEXT("Find Food for the Tribe");
        Q.Description = TEXT("The tribe is starving. Track the herbivore herds east of the ridge and bring back meat.");
        Q.QuestType = EQuest_Type::Gather;
        Q.QuestState = EQuest_State::Inactive;
        Q.GiverNPCLabel = TEXT("QuestNPC_TribalElder_Body");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_TrackHerd");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.Description = TEXT("Follow the tracks east to the ridge");
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Obj1.TargetLocation = FVector(3000.f, 500.f, 100.f);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_CollectMeat");
        Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj2.Description = TEXT("Collect meat from the food cache");
        Obj2.RequiredCount = 3;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Obj2.TargetLocation = FVector(3000.f, 500.f, 100.f);
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_ReturnToTribe");
        Obj3.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj3.Description = TEXT("Return to the tribe settlement");
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bCompleted = false;
        Obj3.TargetLocation = FVector(800.f, 0.f, 50.f);
        Q.Objectives.Add(Obj3);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // --------------------------------------------------------
    // QUEST 2: Defend the Camp
    // Type: Defend | Triggered by: DinosaurAlert from CrowdSimulationManager
    // --------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_DefendTheCamp");
        Q.QuestName = TEXT("Defend the Camp");
        Q.Description = TEXT("A pack of raptors has been spotted near the settlement. Drive them away before they attack the tribe.");
        Q.QuestType = EQuest_Type::Defend;
        Q.QuestState = EQuest_State::Inactive;
        Q.GiverNPCLabel = TEXT("QuestNPC_TribalElder_Body");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_DriveOffRaptors");
        Obj1.ObjectiveType = EQuest_ObjectiveType::KillTarget;
        Obj1.Description = TEXT("Drive off or kill the raptors threatening the camp");
        Obj1.RequiredCount = 3;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Obj1.TargetLocation = FVector(500.f, 300.f, 0.f);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_SurviveAttack");
        Obj2.ObjectiveType = EQuest_ObjectiveType::SurviveTime;
        Obj2.Description = TEXT("Survive the raptor attack for 2 minutes");
        Obj2.RequiredCount = 120;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // --------------------------------------------------------
    // QUEST 3: Craft a Stone Axe
    // Type: Craft | Tutorial quest — first tool
    // --------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_CraftStoneAxe");
        Q.QuestName = TEXT("Craft Your First Weapon");
        Q.Description = TEXT("You need a stone axe to survive. Find 2 rocks and 1 sturdy stick to craft one.");
        Q.QuestType = EQuest_Type::Craft;
        Q.QuestState = EQuest_State::Inactive;
        Q.GiverNPCLabel = TEXT("QuestNPC_TribalElder_Body");
        Q.bIsMainQuest = true;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_CollectRocks");
        Obj1.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj1.Description = TEXT("Collect 2 sharp rocks");
        Obj1.RequiredCount = 2;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_CollectStick");
        Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj2.Description = TEXT("Find a sturdy branch");
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_CraftAxe");
        Obj3.ObjectiveType = EQuest_ObjectiveType::CraftItem;
        Obj3.Description = TEXT("Craft the stone axe at a crafting spot");
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bCompleted = false;
        Q.Objectives.Add(Obj3);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // --------------------------------------------------------
    // QUEST 4: Scout the Volcano Ridge
    // Type: Explore | Dangerous — T-Rex territory
    // --------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_ScoutVolcanoRidge");
        Q.QuestName = TEXT("Scout the Volcano Ridge");
        Q.Description = TEXT("The tribe needs obsidian for better tools. Scout the volcano ridge — but beware, a T-Rex patrols that territory.");
        Q.QuestType = EQuest_Type::Explore;
        Q.QuestState = EQuest_State::Inactive;
        Q.GiverNPCLabel = TEXT("QuestNPC_TribalElder_Body");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_ReachRidge");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.Description = TEXT("Reach the volcano ridge");
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Obj1.TargetLocation = FVector(5000.f, 2000.f, 500.f);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_CollectObsidian");
        Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj2.Description = TEXT("Collect 5 obsidian fragments");
        Obj2.RequiredCount = 5;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Obj2.TargetLocation = FVector(5000.f, 2000.f, 500.f);
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_EscapeTRex");
        Obj3.ObjectiveType = EQuest_ObjectiveType::EscapeArea;
        Obj3.Description = TEXT("Escape the T-Rex territory alive");
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bCompleted = false;
        Q.Objectives.Add(Obj3);

        QuestRegistry.Add(Q.QuestID, Q);
    }
}

// ============================================================
// Quest Lifecycle
// ============================================================

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    QuestRegistry.Add(QuestData.QuestID, QuestData);
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        return false;
    }
    if (Quest->QuestState != EQuest_State::Inactive)
    {
        return false;
    }
    Quest->QuestState = EQuest_State::Active;
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->QuestState != EQuest_State::Active)
    {
        return false;
    }
    // Verify all objectives complete
    for (const FQuest_Objective& Obj : Quest->Objectives)
    {
        if (!Obj.bCompleted)
        {
            return false;
        }
    }
    Quest->QuestState = EQuest_State::Completed;
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->QuestState != EQuest_State::Active)
    {
        return false;
    }
    Quest->QuestState = EQuest_State::Failed;
    return true;
}

bool UQuestManager::AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->QuestState != EQuest_State::Active)
    {
        return false;
    }
    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
            }
            return true;
        }
    }
    return false;
}

// ============================================================
// Query
// ============================================================

EQuest_State UQuestManager::GetQuestState(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        return EQuest_State::Inactive;
    }
    return Quest->QuestState;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    return GetQuestState(QuestID) == EQuest_State::Active;
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Active;
    for (const TPair<FString, FQuest_Data>& Pair : QuestRegistry)
    {
        if (Pair.Value.QuestState == EQuest_State::Active)
        {
            Active.Add(Pair.Value);
        }
    }
    return Active;
}

int32 UQuestManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const TPair<FString, FQuest_Data>& Pair : QuestRegistry)
    {
        if (Pair.Value.QuestState == EQuest_State::Active)
        {
            ++Count;
        }
    }
    return Count;
}
