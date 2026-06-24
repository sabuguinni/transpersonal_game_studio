// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260624_009
// Full implementation: quest registration, activation, objective tracking, proximity triggers

#include "QuestManager.h"
#include "Engine/World.h"

UQuestManager::UQuestManager()
{
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized — %d quests registered"), QuestRegistry.Num());
}

void UQuestManager::Deinitialize()
{
    QuestRegistry.Empty();
    Super::Deinitialize();
}

// ─── Quest Lifecycle ──────────────────────────────────────────────────────────

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] RegisterQuest: empty QuestID — skipped"));
        return;
    }
    QuestRegistry.Add(QuestData.QuestID, QuestData);
    LogQuestEvent(TEXT("REGISTERED"), QuestData.QuestID);
}

bool UQuestManager::ActivateQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] ActivateQuest: QuestID '%s' not found"), *QuestID);
        return false;
    }
    if (Quest->QuestState != EQuest_State::Inactive)
    {
        return false;
    }
    Quest->QuestState = EQuest_State::Active;
    LogQuestEvent(TEXT("ACTIVATED"), QuestID);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->QuestState != EQuest_State::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                LogQuestEvent(FString::Printf(TEXT("OBJECTIVE_COMPLETE:%s"), *ObjectiveID), QuestID);

                // Auto-complete quest if all objectives done
                bool bAllDone = true;
                for (const FQuest_Objective& O : Quest->Objectives)
                {
                    if (!O.bCompleted) { bAllDone = false; break; }
                }
                if (bAllDone)
                {
                    CompleteQuest(QuestID);
                }
            }
            return true;
        }
    }
    return false;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->QuestState != EQuest_State::Active)
    {
        return false;
    }
    Quest->QuestState = EQuest_State::Completed;
    LogQuestEvent(FString::Printf(TEXT("COMPLETED — XP:%d"), Quest->RewardXP), QuestID);
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
    LogQuestEvent(TEXT("FAILED"), QuestID);
    return true;
}

// ─── Query ────────────────────────────────────────────────────────────────────

EQuest_State UQuestManager::GetQuestState(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest ? Quest->QuestState : EQuest_State::Inactive;
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Active;
    for (const auto& Pair : QuestRegistry)
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
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.QuestState == EQuest_State::Active) Count++;
    }
    return Count;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    return GetQuestState(QuestID) == EQuest_State::Active;
}

// ─── Proximity Triggers ───────────────────────────────────────────────────────

void UQuestManager::CheckProximityTriggers(const FVector& PlayerLocation)
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.QuestState != EQuest_State::Inactive) continue;
        if (Quest.TriggerLocation.IsZero()) continue;

        const float Dist = FVector::Dist(PlayerLocation, Quest.TriggerLocation);
        if (Dist <= Quest.TriggerRadius)
        {
            ActivateQuest(Quest.QuestID);
        }
    }
}

// ─── Default Quest Definitions ────────────────────────────────────────────────

void UQuestManager::RegisterDefaultQuests()
{
    // ── Quest 1: Raptor Ambush ─────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_RAPTOR_AMBUSH");
        Q.QuestTitle = TEXT("Raptor Ambush");
        Q.QuestDescription = TEXT("A raptor pack is stalking the Parasaurolophus herd. Distract or drive off the raptors before they make a kill.");
        Q.QuestType = EQuest_Type::Defend;
        Q.QuestState = EQuest_State::Inactive;
        Q.TriggerLocation = FVector(2800.0f, 1200.0f, 200.0f);
        Q.TriggerRadius = 400.0f;
        Q.RewardXP = 150;
        Q.RewardItemID = TEXT("ITEM_RAPTOR_CLAW");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_RAPTORS");
        Obj1.Description = TEXT("Locate the raptor pack");
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_DRIVE_OFF_RAPTORS");
        Obj2.Description = TEXT("Drive off or distract the raptors (3 raptors)");
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        RegisterQuest(Q);
    }

    // ── Quest 2: Follow the Herd ───────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_FOLLOW_HERD");
        Q.QuestTitle = TEXT("Follow the Herd");
        Q.QuestDescription = TEXT("The Parasaurolophus herd is migrating. Follow them to discover a new water source.");
        Q.QuestType = EQuest_Type::Track;
        Q.QuestState = EQuest_State::Inactive;
        Q.TriggerLocation = FVector(3000.0f, 1500.0f, 200.0f);
        Q.TriggerRadius = 500.0f;
        Q.RewardXP = 200;
        Q.RewardItemID = TEXT("ITEM_WATER_SKIN");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_HERD");
        Obj1.Description = TEXT("Find the Parasaurolophus herd");
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_FOLLOW_WAYPOINT_1");
        Obj2.Description = TEXT("Follow the herd to the river valley");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_FIND_WATER");
        Obj3.Description = TEXT("Discover the water source");
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }

    // ── Quest 3: Protoceratops Nest ────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_PROTO_NEST");
        Q.QuestTitle = TEXT("Protoceratops Nest");
        Q.QuestDescription = TEXT("You have found a Protoceratops nesting site. Collect eggs for food — but be quick, the mother is nearby.");
        Q.QuestType = EQuest_Type::Gather;
        Q.QuestState = EQuest_State::Inactive;
        Q.TriggerLocation = FVector(-1500.0f, 2500.0f, 200.0f);
        Q.TriggerRadius = 300.0f;
        Q.RewardXP = 120;
        Q.RewardItemID = TEXT("ITEM_DINO_EGG");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_NEST");
        Obj1.Description = TEXT("Find the Protoceratops nest");
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_COLLECT_EGGS");
        Obj2.Description = TEXT("Collect eggs (3 required)");
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_ESCAPE_MOTHER");
        Obj3.Description = TEXT("Escape before the mother returns");
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Default quests registered: %d"), QuestRegistry.Num());
}

// ─── Logging ──────────────────────────────────────────────────────────────────

void UQuestManager::LogQuestEvent(const FString& Event, const FString& QuestID) const
{
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] %s | Quest: %s"), *Event, *QuestID);
}
