// QuestManager.cpp
// Quest & Mission Designer — Agent #14
// Full implementation of UQuestManager world subsystem

#include "QuestManager.h"

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultQuests();
}

void UQuestManager::Deinitialize()
{
    QuestRegistry.Empty();
    Super::Deinitialize();
}

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (QuestData.QuestID.IsEmpty()) return;
    QuestRegistry.Add(QuestData.QuestID, QuestData);
}

bool UQuestManager::ActivateQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;
    if (Quest->Status != EQuest_Status::Available && Quest->Status != EQuest_Status::Locked) return false;

    Quest->Status = EQuest_Status::Active;
    OnQuestStatusChanged.Broadcast(QuestID);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active) return false;

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
            }
            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID);

            // Auto-complete quest if all objectives done
            bool bAllDone = true;
            for (const FQuest_Objective& O : Quest->Objectives)
            {
                if (!O.bCompleted) { bAllDone = false; break; }
            }
            if (bAllDone) CompleteQuest(QuestID);

            return true;
        }
    }
    return false;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Completed;
    OnQuestStatusChanged.Broadcast(QuestID);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Failed;
    OnQuestStatusChanged.Broadcast(QuestID);
    return true;
}

EQuest_Status UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return EQuest_Status::Locked;
    return Quest->Status;
}

bool UQuestManager::GetQuestData(const FString& QuestID, FQuest_Data& OutData) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;
    OutData = *Quest;
    return true;
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Active)
            Result.Add(Pair.Value);
    }
    return Result;
}

TArray<FQuest_Data> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Available)
            Result.Add(Pair.Value);
    }
    return Result;
}

int32 UQuestManager::GetTotalQuestCount() const
{
    return QuestRegistry.Num();
}

void UQuestManager::CheckProximityTriggers(const FVector& PlayerLocation)
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status != EQuest_Status::Locked) continue;
        if (Quest.TriggerLocation.IsZero()) continue;

        float Dist = FVector::Dist(PlayerLocation, Quest.TriggerLocation);
        if (Dist <= Quest.TriggerRadius)
        {
            Quest.Status = EQuest_Status::Available;
            OnQuestStatusChanged.Broadcast(Quest.QuestID);
        }
    }
}

void UQuestManager::RegisterDefaultQuests()
{
    // -------------------------------------------------------
    // QUEST 1: "Find the Tribe" — Explore quest
    // Player discovers the tribe camp and earns their trust
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = "Q_FindTribe";
        Q.QuestTitle = "Find the Tribe";
        Q.QuestDescription = "Smoke rising from the valley. Someone has made camp. Approach carefully — they may be hostile, or they may be the allies you need to survive.";
        Q.QuestType = EQuest_Type::Explore;
        Q.Status = EQuest_Status::Locked;
        Q.TriggerLocation = FVector(400.0f, 200.0f, 100.0f);
        Q.TriggerRadius = 500.0f;
        Q.RewardDescription = "Access to tribe crafting knowledge. Stone Axe recipe unlocked.";

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_ReachCamp";
        Obj1.Description = "Reach the tribe camp";
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_TalkElder";
        Obj2.Description = "Speak with the tribe elder";
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        RegisterQuest(Q);
    }

    // -------------------------------------------------------
    // QUEST 2: "Protect the Herd" — Defend quest
    // Raptor pack is hunting the brachiosaur herd
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = "Q_ProtectHerd";
        Q.QuestTitle = "Protect the Herd";
        Q.QuestDescription = "A raptor pack is closing in on the brachiosaur herd moving through the valley. Drive off the raptors before they bring down one of the young ones.";
        Q.QuestType = EQuest_Type::Defend;
        Q.Status = EQuest_Status::Locked;
        Q.TriggerLocation = FVector(-800.0f, 600.0f, 100.0f);
        Q.TriggerRadius = 600.0f;
        Q.RewardDescription = "Brachiosaur hide material. Tribe gratitude — unlocks Campfire recipe.";

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_DriveRaptors";
        Obj1.Description = "Drive off the raptor pack (3 raptors)";
        Obj1.RequiredCount = 3;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_HerdSurvives";
        Obj2.Description = "Keep at least 3 herd members alive";
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        RegisterQuest(Q);
    }

    // -------------------------------------------------------
    // QUEST 3: "Raptor Threat" — Track + Hunt quest
    // Scout the raptor den before it becomes a larger problem
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = "Q_RaptorThreat";
        Q.QuestTitle = "Raptor Threat";
        Q.QuestDescription = "The tribe elder warns of a raptor den to the south. If the pack grows any larger, no one in the valley will be safe. Find the den and assess the threat.";
        Q.QuestType = EQuest_Type::Track;
        Q.Status = EQuest_Status::Locked;
        Q.TriggerLocation = FVector(0.0f, -400.0f, 100.0f);
        Q.TriggerRadius = 400.0f;
        Q.RewardDescription = "Raptor claw crafting material. Water Container recipe unlocked.";

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_FindWaypoint1";
        Obj1.Description = "Follow raptor tracks to the first waypoint";
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_FindWaypoint2";
        Obj2.Description = "Track the pack to the second waypoint";
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = "OBJ_LocateDen";
        Obj3.Description = "Locate the raptor den";
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }
}
