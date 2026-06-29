// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// World Subsystem: manages all quest lifecycle, objective tracking, and rewards.
// Cycle: PROD_CYCLE_AUTO_20260629_008

#include "Quest/QuestManager.h"
#include "Quest/QuestSystemTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuestManager::UQuestManager()
{
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterAllBuiltInQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized — %d quests registered"), QuestLibrary.Num());
}

void UQuestManager::Deinitialize()
{
    QuestLibrary.Empty();
    QuestStatusMap.Empty();
    ObjectiveProgress.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Quest Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

bool UQuestManager::ActivateQuest(FName QuestID)
{
    if (!QuestLibrary.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] ActivateQuest: Quest '%s' not found in library"), *QuestID.ToString());
        return false;
    }

    EQuest_Status* CurrentStatus = QuestStatusMap.Find(QuestID);
    if (CurrentStatus && (*CurrentStatus == EQuest_Status::Active || *CurrentStatus == EQuest_Status::Completed))
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] ActivateQuest: Quest '%s' already active or completed"), *QuestID.ToString());
        return false;
    }

    // Activate
    QuestStatusMap.Add(QuestID, EQuest_Status::Active);

    // Initialize objective progress
    const FQuest_Definition& QuestDef = QuestLibrary[QuestID];
    TArray<int32> Progress;
    Progress.SetNum(QuestDef.Objectives.Num());
    for (int32& P : Progress) P = 0;
    ObjectiveProgress.Add(QuestID, Progress);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ACTIVATED: '%s' — %s"), *QuestID.ToString(), *QuestDef.Title);
    OnQuestActivated.Broadcast(QuestDef);
    return true;
}

bool UQuestManager::AbandonQuest(FName QuestID)
{
    if (!QuestStatusMap.Contains(QuestID)) return false;
    if (QuestStatusMap[QuestID] != EQuest_Status::Active) return false;

    QuestStatusMap[QuestID] = EQuest_Status::Abandoned;
    ObjectiveProgress.Remove(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ABANDONED: '%s'"), *QuestID.ToString());
    return true;
}

bool UQuestManager::FailQuest(FName QuestID)
{
    if (!QuestStatusMap.Contains(QuestID)) return false;
    if (QuestStatusMap[QuestID] != EQuest_Status::Active) return false;

    QuestStatusMap[QuestID] = EQuest_Status::Failed;
    ObjectiveProgress.Remove(QuestID);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest FAILED: '%s'"), *QuestID.ToString());
    OnQuestFailed.Broadcast(QuestID);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Objective Tracking
// ─────────────────────────────────────────────────────────────────────────────

void UQuestManager::ReportObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Amount)
{
    if (!QuestStatusMap.Contains(QuestID) || QuestStatusMap[QuestID] != EQuest_Status::Active) return;
    if (!ObjectiveProgress.Contains(QuestID)) return;

    TArray<int32>& Progress = ObjectiveProgress[QuestID];
    if (!Progress.IsValidIndex(ObjectiveIndex)) return;

    const FQuest_Definition& QuestDef = QuestLibrary[QuestID];
    if (!QuestDef.Objectives.IsValidIndex(ObjectiveIndex)) return;

    const FQuest_Objective& Obj = QuestDef.Objectives[ObjectiveIndex];
    if (Obj.bIsCompleted) return;

    Progress[ObjectiveIndex] = FMath::Min(Progress[ObjectiveIndex] + Amount, Obj.RequiredCount);
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective progress [%s][%d]: %d/%d"),
        *QuestID.ToString(), ObjectiveIndex, Progress[ObjectiveIndex], Obj.RequiredCount);

    OnQuestObjectiveUpdated.Broadcast(QuestID, ObjectiveIndex);

    if (Progress[ObjectiveIndex] >= Obj.RequiredCount)
    {
        CompleteObjective(QuestID, ObjectiveIndex);
    }
}

void UQuestManager::CompleteObjective(FName QuestID, int32 ObjectiveIndex)
{
    if (!QuestLibrary.Contains(QuestID)) return;

    FQuest_Definition& QuestDef = QuestLibrary[QuestID];
    if (!QuestDef.Objectives.IsValidIndex(ObjectiveIndex)) return;

    QuestDef.Objectives[ObjectiveIndex].bIsCompleted = true;
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective COMPLETE [%s][%d]: %s"),
        *QuestID.ToString(), ObjectiveIndex, *QuestDef.Objectives[ObjectiveIndex].Description);

    OnQuestObjectiveUpdated.Broadcast(QuestID, ObjectiveIndex);
    CheckQuestCompletion(QuestID);
}

// ─────────────────────────────────────────────────────────────────────────────
// Query
// ─────────────────────────────────────────────────────────────────────────────

TArray<FQuest_Definition> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Definition> Active;
    for (const auto& Pair : QuestStatusMap)
    {
        if (Pair.Value == EQuest_Status::Active && QuestLibrary.Contains(Pair.Key))
        {
            Active.Add(QuestLibrary[Pair.Key]);
        }
    }
    return Active;
}

bool UQuestManager::GetQuestByID(FName QuestID, FQuest_Definition& OutQuest) const
{
    if (QuestLibrary.Contains(QuestID))
    {
        OutQuest = QuestLibrary[QuestID];
        return true;
    }
    return false;
}

bool UQuestManager::IsQuestActive(FName QuestID) const
{
    const EQuest_Status* Status = QuestStatusMap.Find(QuestID);
    return Status && *Status == EQuest_Status::Active;
}

bool UQuestManager::IsQuestCompleted(FName QuestID) const
{
    const EQuest_Status* Status = QuestStatusMap.Find(QuestID);
    return Status && *Status == EQuest_Status::Completed;
}

EQuest_Status UQuestManager::GetQuestStatus(FName QuestID) const
{
    const EQuest_Status* Status = QuestStatusMap.Find(QuestID);
    return Status ? *Status : EQuest_Status::Inactive;
}

// ─────────────────────────────────────────────────────────────────────────────
// Registration
// ─────────────────────────────────────────────────────────────────────────────

void UQuestManager::RegisterQuest(const FQuest_Definition& QuestDef)
{
    QuestLibrary.Add(QuestDef.QuestID, QuestDef);
    QuestStatusMap.Add(QuestDef.QuestID, EQuest_Status::Inactive);
}

void UQuestManager::RegisterAllBuiltInQuests()
{
    BuildBuiltInQuests();
}

void UQuestManager::BuildBuiltInQuests()
{
    // ── QUEST 1: Raptor Threat ────────────────────────────────────────────────
    {
        FQuest_Definition Q;
        Q.QuestID = FName("Q_RaptorThreat");
        Q.Title = TEXT("Raptor Threat");
        Q.Description = TEXT("Three raptors are circling the eastern camp. Hunt them down before they attack the tribe.");
        Q.Tier = EQuest_Tier::Hunt;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.Description = TEXT("Kill 3 raptors near the eastern camp");
        Obj1.Type = EQuest_ObjectiveType::Hunt;
        Obj1.TargetTag = FName("Raptor");
        Obj1.RequiredCount = 3;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.Description = TEXT("Collect raptor claws as proof (3)");
        Obj2.Type = EQuest_ObjectiveType::Collect;
        Obj2.TargetTag = FName("RaptorClaw");
        Obj2.RequiredCount = 3;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward;
        Reward.BoneCount = 5;
        Reward.HideCount = 3;
        Reward.XPAmount = 150;
        Q.Reward = Reward;

        RegisterQuest(Q);
    }

    // ── QUEST 2: Follow the Migration ────────────────────────────────────────
    {
        FQuest_Definition Q;
        Q.QuestID = FName("Q_BrachioMigration");
        Q.Title = TEXT("Follow the Migration");
        Q.Description = TEXT("The Brachiosaurus herd is moving south toward the river. Follow them to discover new water sources and hunting grounds.");
        Q.Tier = EQuest_Tier::Migrate;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.Description = TEXT("Locate the Brachiosaurus herd anchor");
        Obj1.Type = EQuest_ObjectiveType::Reach;
        Obj1.TargetTag = FName("HerdAnchor_Brachiosaurus");
        Obj1.RequiredCount = 1;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.Description = TEXT("Follow the herd south for 500 meters without being detected");
        Obj2.Type = EQuest_ObjectiveType::Scout;
        Obj2.TargetTag = FName("MigrationPath_South");
        Obj2.RequiredCount = 1;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.Description = TEXT("Reach the river valley");
        Obj3.Type = EQuest_ObjectiveType::Reach;
        Obj3.TargetTag = FName("RiverValley");
        Obj3.RequiredCount = 1;
        Obj3.bIsCompleted = false;
        Q.Objectives.Add(Obj3);

        FQuest_Reward Reward;
        Reward.MeatCount = 8;
        Reward.XPAmount = 200;
        Reward.bUnlocksRecipe = true;
        Reward.UnlockedRecipeID = FName("Recipe_WaterContainer");
        Q.Reward = Reward;

        RegisterQuest(Q);
    }

    // ── QUEST 3: Survive the Stampede ────────────────────────────────────────
    {
        FQuest_Definition Q;
        Q.QuestID = FName("Q_SurviveStampede");
        Q.Title = TEXT("Survive the Stampede");
        Q.Description = TEXT("A Triceratops stampede has been triggered. Get to high ground and survive until the herd passes.");
        Q.Tier = EQuest_Tier::Survival;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.Description = TEXT("Reach high ground before the stampede arrives");
        Obj1.Type = EQuest_ObjectiveType::Reach;
        Obj1.TargetTag = FName("HighGround_Zone");
        Obj1.RequiredCount = 1;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.Description = TEXT("Survive for 90 seconds while the herd passes");
        Obj2.Type = EQuest_ObjectiveType::Survive;
        Obj2.TargetTag = FName("StampedeEvent");
        Obj2.RequiredCount = 90;  // seconds
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward;
        Reward.HideCount = 6;
        Reward.BoneCount = 4;
        Reward.XPAmount = 300;
        Q.Reward = Reward;

        RegisterQuest(Q);
    }

    // ── QUEST 4: Observe the Parasaurolophus Herd ────────────────────────────
    {
        FQuest_Definition Q;
        Q.QuestID = FName("Q_ObserveParasaur");
        Q.Title = TEXT("Eyes on the Herd");
        Q.Description = TEXT("Observe the Parasaurolophus herd to learn their patrol patterns. Stay hidden — if they spot you, the mission fails.");
        Q.Tier = EQuest_Tier::Explore;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.Description = TEXT("Approach the Parasaurolophus herd without being detected");
        Obj1.Type = EQuest_ObjectiveType::Scout;
        Obj1.TargetTag = FName("HerdAnchor_Parasaurolophus");
        Obj1.RequiredCount = 1;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.Description = TEXT("Observe the herd for 60 seconds from concealment");
        Obj2.Type = EQuest_ObjectiveType::Survive;
        Obj2.TargetTag = FName("ParasaurObservation");
        Obj2.RequiredCount = 60;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward;
        Reward.XPAmount = 175;
        Reward.bUnlocksRecipe = true;
        Reward.UnlockedRecipeID = FName("Recipe_StoneAxe");
        Q.Reward = Reward;

        RegisterQuest(Q);
    }

    // ── QUEST 5: Defend the Camp ──────────────────────────────────────────────
    {
        FQuest_Definition Q;
        Q.QuestID = FName("Q_DefendCamp");
        Q.Title = TEXT("Defend the Camp");
        Q.Description = TEXT("A T-Rex has been spotted approaching the valley. Distract it with fire and drive it away before it reaches the tribe.");
        Q.Tier = EQuest_Tier::Defend;
        Q.bIsMainQuest = true;

        FQuest_Objective Obj1;
        Obj1.Description = TEXT("Craft 3 torches to use as deterrents");
        Obj1.Type = EQuest_ObjectiveType::Craft;
        Obj1.TargetTag = FName("Torch");
        Obj1.RequiredCount = 3;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.Description = TEXT("Use torches to drive the T-Rex away from the camp");
        Obj2.Type = EQuest_ObjectiveType::Destroy;
        Obj2.TargetTag = FName("TRex_Alpha");
        Obj2.RequiredCount = 1;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.Description = TEXT("Ensure no tribe members are killed");
        Obj3.Type = EQuest_ObjectiveType::Protect;
        Obj3.TargetTag = FName("TribeMember");
        Obj3.RequiredCount = 5;
        Obj3.bIsCompleted = false;
        Q.Objectives.Add(Obj3);

        FQuest_Reward Reward;
        Reward.MeatCount = 10;
        Reward.HideCount = 8;
        Reward.BoneCount = 6;
        Reward.XPAmount = 500;
        Reward.bUnlocksRecipe = true;
        Reward.UnlockedRecipeID = FName("Recipe_Campfire");
        Q.Reward = Reward;

        RegisterQuest(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Built-in quests registered: %d"), QuestLibrary.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Helpers
// ─────────────────────────────────────────────────────────────────────────────

void UQuestManager::CheckQuestCompletion(FName QuestID)
{
    if (!QuestLibrary.Contains(QuestID)) return;
    if (!QuestStatusMap.Contains(QuestID) || QuestStatusMap[QuestID] != EQuest_Status::Active) return;

    const FQuest_Definition& QuestDef = QuestLibrary[QuestID];
    bool bAllComplete = true;
    for (const FQuest_Objective& Obj : QuestDef.Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        QuestStatusMap[QuestID] = EQuest_Status::Completed;
        GrantRewards(QuestDef);
        UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest COMPLETED: '%s' — %s"), *QuestID.ToString(), *QuestDef.Title);
        OnQuestCompleted.Broadcast(QuestID);
    }
}

void UQuestManager::GrantRewards(const FQuest_Definition& Quest)
{
    const FQuest_Reward& R = Quest.Reward;
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Granting rewards for '%s': Bone=%d Hide=%d Meat=%d Stone=%d XP=%d RecipeUnlock=%s"),
        *Quest.QuestID.ToString(),
        R.BoneCount, R.HideCount, R.MeatCount, R.StoneCount, R.XPAmount,
        R.bUnlocksRecipe ? *R.UnlockedRecipeID.ToString() : TEXT("none"));
    // TODO: Integrate with inventory system when available
}

// ─────────────────────────────────────────────────────────────────────────────
// Herd Integration
// ─────────────────────────────────────────────────────────────────────────────

void UQuestManager::OnStampedeTriggered(FVector StampedeOrigin, FVector StampedeDirection)
{
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Stampede triggered at %s — activating Q_SurviveStampede"),
        *StampedeOrigin.ToString());
    ActivateQuest(FName("Q_SurviveStampede"));
}

void UQuestManager::OnPlayerEnteredHerdZone(FName HerdSpecies, FVector ZoneLocation)
{
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Player entered herd zone: %s at %s"),
        *HerdSpecies.ToString(), *ZoneLocation.ToString());

    if (HerdSpecies == FName("Brachiosaurus"))
    {
        ActivateQuest(FName("Q_BrachioMigration"));
    }
    else if (HerdSpecies == FName("Parasaurolophus"))
    {
        ActivateQuest(FName("Q_ObserveParasaur"));
    }
    else if (HerdSpecies == FName("Triceratops"))
    {
        ActivateQuest(FName("Q_SurviveStampede"));
    }
}
