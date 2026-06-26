// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// World Subsystem managing all quest state, objectives, and crowd-based triggers

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ============================================================
// UWorldSubsystem Interface
// ============================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized with %d quests registered"), QuestRegistry.Num());
}

void UQuestManager::Deinitialize()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(TimeLimitTimerHandle);
    }
    QuestRegistry.Empty();
    Super::Deinitialize();
}

// ============================================================
// Quest Lifecycle
// ============================================================

bool UQuestManager::ActivateQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] ActivateQuest: Quest '%s' not found"), *QuestID);
        return false;
    }

    if (Quest->Status != EQuest_Status::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] ActivateQuest: Quest '%s' is not inactive (status=%d)"), *QuestID, (int32)Quest->Status);
        return false;
    }

    Quest->Status = EQuest_Status::Active;
    Quest->ElapsedTime = 0.0f;

    OnQuestActivated.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ACTIVATED: %s — %s"), *QuestID, *Quest->Title);
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return false;
    }

    Quest->Status = EQuest_Status::Completed;
    OnQuestCompleted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest COMPLETED: %s"), *QuestID);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return false;
    }

    Quest->Status = EQuest_Status::Failed;
    OnQuestFailed.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest FAILED: %s"), *QuestID);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            if (Obj.bCompleted)
            {
                return false; // Already done
            }
            Obj.bCompleted = true;
            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
            UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective DONE: %s / %s"), *QuestID, *ObjectiveID);

            // Auto-complete quest if all mandatory objectives are done
            if (AreAllMandatoryObjectivesComplete(*Quest))
            {
                CompleteQuest(QuestID);
            }
            return true;
        }
    }
    return false;
}

// ============================================================
// Quest Queries
// ============================================================

EQuest_Status UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest ? Quest->Status : EQuest_Status::Inactive;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    return GetQuestStatus(QuestID) == EQuest_Status::Active;
}

bool UQuestManager::IsObjectiveComplete(const FString& QuestID, const FString& ObjectiveID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;

    for (const FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            return Obj.bCompleted;
        }
    }
    return false;
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Active;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Active)
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
        if (Pair.Value.Status == EQuest_Status::Active)
        {
            Count++;
        }
    }
    return Count;
}

FQuest_Data UQuestManager::GetQuestData(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest ? *Quest : FQuest_Data();
}

// ============================================================
// Registration
// ============================================================

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] RegisterQuest: QuestID is empty, skipping"));
        return;
    }
    QuestRegistry.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Registered quest: %s"), *QuestData.QuestID);
}

void UQuestManager::RegisterDefaultQuests()
{
    BuildDefaultQuestData();
}

// ============================================================
// Crowd Integration
// ============================================================

void UQuestManager::OnCrowdAlertTriggered(const FString& GroupID, float AlertLevel)
{
    // Quest: "Escape the Raptor Pack" — triggers when RaptorPack_Alpha alert > 0.6
    if (GroupID == TEXT("RaptorPack_Alpha") && AlertLevel > 0.6f)
    {
        if (GetQuestStatus(TEXT("Q_Raptor_Siege")) == EQuest_Status::Inactive)
        {
            ActivateQuest(TEXT("Q_Raptor_Siege"));
        }
    }

    // Quest: "Stampede Survival" — triggers when TrikeHerd_A alert > 0.8
    if (GroupID == TEXT("TrikeHerd_A") && AlertLevel > 0.8f)
    {
        if (GetQuestStatus(TEXT("Q_Herd_Stampede")) == EQuest_Status::Inactive)
        {
            ActivateQuest(TEXT("Q_Herd_Stampede"));
        }
    }

    // Quest: "Apex Predator" — triggers when TRex_Roamer alert > 0.5
    if (GroupID == TEXT("TRex_Roamer") && AlertLevel > 0.5f)
    {
        if (GetQuestStatus(TEXT("Q_TRex_Encounter")) == EQuest_Status::Inactive)
        {
            ActivateQuest(TEXT("Q_TRex_Encounter"));
        }
    }
}

void UQuestManager::CheckCrowdBasedQuestConditions()
{
    // Called periodically to check if crowd state satisfies quest conditions
    // Integration point for UCrowdSimulationManager::GetAlertingGroups()
    UE_LOG(LogTemp, Verbose, TEXT("[QuestManager] CheckCrowdBasedQuestConditions: %d active quests"), GetActiveQuestCount());
}

// ============================================================
// Private Helpers
// ============================================================

bool UQuestManager::AreAllMandatoryObjectivesComplete(const FQuest_Data& Quest) const
{
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bOptional && !Obj.bCompleted)
        {
            return false;
        }
    }
    return true;
}

void UQuestManager::TickTimeLimitedQuests(float DeltaTime)
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status == EQuest_Status::Active && Quest.TimeLimit > 0.0f)
        {
            Quest.ElapsedTime += DeltaTime;
            if (Quest.bTimeLimitFails && Quest.ElapsedTime >= Quest.TimeLimit)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}

void UQuestManager::BuildDefaultQuestData()
{
    // ============================================================
    // QUEST 1: "Apex Predator" — T-Rex Encounter
    // ============================================================
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_TRex_Encounter");
        Q.Title = TEXT("Apex Predator");
        Q.Description = TEXT("A T-Rex has entered the valley. Survive the encounter and escape to the northern ridge.");
        Q.QuestType = EQuest_Type::Survive;
        Q.Status = EQuest_Status::Inactive;
        Q.TimeLimit = 180.0f; // 3 minutes
        Q.bTimeLimitFails = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_TRex_Detect");
        Obj1.Description = TEXT("Spot the T-Rex before it spots you");
        Obj1.WorldLocation = FVector(3500, 2000, 400);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_TRex_Evade");
        Obj2.Description = TEXT("Reach the rock formation without being caught");
        Obj2.WorldLocation = FVector(3200, 1800, 400);
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_TRex_Escape");
        Obj3.Description = TEXT("Escape north along the river");
        Obj3.WorldLocation = FVector(3800, 1500, 400);
        Obj3.bOptional = true;
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }

    // ============================================================
    // QUEST 2: "Siege at Dusk" — Raptor Pack Defense
    // ============================================================
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_Raptor_Siege");
        Q.Title = TEXT("Siege at Dusk");
        Q.Description = TEXT("A raptor pack has surrounded the camp. Hold your ground until dawn or drive them off.");
        Q.QuestType = EQuest_Type::Defend;
        Q.Status = EQuest_Status::Inactive;
        Q.TimeLimit = 120.0f; // 2 minutes
        Q.bTimeLimitFails = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_Raptor_Fire");
        Obj1.Description = TEXT("Keep the campfire burning");
        Obj1.WorldLocation = FVector(1500, 3200, 400);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_Raptor_Alpha");
        Obj2.Description = TEXT("Drive off or kill the alpha raptor");
        Obj2.WorldLocation = FVector(1700, 3600, 400);
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_Raptor_Survive");
        Obj3.Description = TEXT("Survive until the pack retreats");
        Obj3.bOptional = false;
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }

    // ============================================================
    // QUEST 3: "Stampede" — Herd Panic Survival
    // ============================================================
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_Herd_Stampede");
        Q.Title = TEXT("Stampede");
        Q.Description = TEXT("The Triceratops herd has panicked and is charging south. Get out of their path.");
        Q.QuestType = EQuest_Type::Survive;
        Q.Status = EQuest_Status::Inactive;
        Q.TimeLimit = 60.0f; // 1 minute — urgent
        Q.bTimeLimitFails = true;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_Stampede_Detect");
        Obj1.Description = TEXT("Hear the rumble and identify the direction");
        Obj1.WorldLocation = FVector(2500, 800, 400);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_Stampede_SafeZone");
        Obj2.Description = TEXT("Reach the safe zone before the herd arrives");
        Obj2.WorldLocation = FVector(2200, 200, 400);
        Q.Objectives.Add(Obj2);

        RegisterQuest(Q);
    }

    // ============================================================
    // QUEST 4: "Migration Trail" — Track Brachiosaurus Herd
    // ============================================================
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("Q_Brachio_Track");
        Q.Title = TEXT("Migration Trail");
        Q.Description = TEXT("Follow the Brachiosaurus herd south. They always find water. Their migration path leads to a hidden river.");
        Q.QuestType = EQuest_Type::Track;
        Q.Status = EQuest_Status::Inactive;
        Q.TimeLimit = 0.0f; // No time limit — exploration quest

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_Brachio_Locate");
        Obj1.Description = TEXT("Find the Brachiosaurus herd");
        Obj1.WorldLocation = FVector(4000, 3500, 400);
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_Brachio_Follow");
        Obj2.Description = TEXT("Follow the herd without disturbing them");
        Obj2.WorldLocation = FVector(4500, 3000, 400);
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_Brachio_Water");
        Obj3.Description = TEXT("Discover the hidden water source");
        Obj3.WorldLocation = FVector(5000, 4000, 400);
        Q.Objectives.Add(Obj3);

        RegisterQuest(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Default quests registered: %d total"), QuestRegistry.Num());
}
