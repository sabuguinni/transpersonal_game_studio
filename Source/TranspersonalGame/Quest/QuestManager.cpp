// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// WorldSubsystem managing all quest lifecycle, objective tracking, and crowd integration

#include "QuestManager.h"
#include "Engine/World.h"

// ============================================================
// USubsystem interface
// ============================================================

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    StampedeQuestTimer = 0.f;
    bStampedeQuestActive = false;

    RegisterDefaultQuests();

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized — %d quests registered"), QuestRegistry.Num());
}

void UQuestManager::Deinitialize()
{
    QuestRegistry.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    Super::Deinitialize();
}

// ============================================================
// Default quest registration — 5 starter quests
// ============================================================

void UQuestManager::RegisterDefaultQuests()
{
    // ---- QUEST 1: First Blood — Hunt a Raptor ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("HUNT_001_FirstBlood");
        Q.QuestTitle = TEXT("First Blood");
        Q.QuestDescription = TEXT("A lone Velociraptor has been circling the camp. Hunt it before nightfall or it will grow bold enough to attack.");
        Q.QuestType = EQuest_Type::Hunting;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Tracker_Kael");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_KillRaptor");
        Obj1.Description = TEXT("Kill the Velociraptor stalking the camp");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_ReturnToKael");
        Obj2.Description = TEXT("Return to Kael with proof of the kill");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.TargetLocation = FVector(200.f, 300.f, 0.f);
        Obj2.LocationRadius = 400.f;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        Q.Reward.BoneTokens = 15;
        Q.Reward.UnlockedRecipes.Add(TEXT("RECIPE_StoneTip_Spear"));
        Q.Reward.NarrativeUnlock = TEXT("LORE_RaptorPackBehavior");

        RegisterQuest(Q);
    }

    // ---- QUEST 2: Follow the Herd — Migration tracking ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("EXPLORE_001_FollowHerd");
        Q.QuestTitle = TEXT("Follow the Herd");
        Q.QuestDescription = TEXT("The Triceratops herd moves south before the dry season. Follow their trail — where they graze, water is never far.");
        Q.QuestType = EQuest_Type::Migration;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Elder_Mara");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FindHerd");
        Obj1.Description = TEXT("Locate the Triceratops herd");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Track;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.TargetLocation = FVector(1600.f, 2800.f, 0.f);
        Obj1.LocationRadius = 1200.f;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_FollowToWater");
        Obj2.Description = TEXT("Follow the herd to the river crossing");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.TargetLocation = FVector(3500.f, 4200.f, 0.f);
        Obj2.LocationRadius = 800.f;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        Q.Reward.BoneTokens = 20;
        Q.Reward.UnlockedAreas.Add(TEXT("AREA_RiverDelta"));
        Q.Reward.NarrativeUnlock = TEXT("LORE_TriceratopsMigration");

        RegisterQuest(Q);
    }

    // ---- QUEST 3: Survive the Stampede — Crowd integration ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("SURVIVE_001_Stampede");
        Q.QuestTitle = TEXT("Survive the Stampede");
        Q.QuestDescription = TEXT("You fired your sling in the middle of the herd. Now a hundred tons of panicked Triceratops are heading straight for you. Get out of the valley — NOW.");
        Q.QuestType = EQuest_Type::Survival;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT(""); // Triggered automatically by crowd alert
        Q.bIsRepeatable = true;
        Q.TimeLimit = 90.f; // 90 seconds to escape

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_EscapeStampede");
        Obj1.Description = TEXT("Escape the stampede radius (get 2000 units away from the herd)");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        Q.Reward.BoneTokens = 30;
        Q.Reward.NarrativeUnlock = TEXT("LORE_HerdPanicBehavior");

        RegisterQuest(Q);
    }

    // ---- QUEST 4: Stone Tools — Crafting quest ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("CRAFT_001_StoneTools");
        Q.QuestTitle = TEXT("Stone and Bone");
        Q.QuestDescription = TEXT("Bare hands will not keep you alive. Gather flint from the riverbed and craft a stone axe before the next predator finds you.");
        Q.QuestType = EQuest_Type::Crafting;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Crafter_Duna");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_GatherFlint");
        Obj1.Description = TEXT("Gather 3 pieces of flint from the riverbed");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.RequiredCount = 3;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_GatherStick");
        Obj2.Description = TEXT("Find a sturdy branch");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_CraftAxe");
        Obj3.Description = TEXT("Craft a Stone Axe at the crafting station");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bIsCompleted = false;
        Q.Objectives.Add(Obj3);

        Q.Reward.BoneTokens = 10;
        Q.Reward.UnlockedRecipes.Add(TEXT("RECIPE_BoneKnife"));
        Q.Reward.UnlockedRecipes.Add(TEXT("RECIPE_WoodenShield"));

        RegisterQuest(Q);
    }

    // ---- QUEST 5: Defend the Camp — Defense quest ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("DEFENSE_001_NightAttack");
        Q.QuestTitle = TEXT("Night Raid");
        Q.QuestDescription = TEXT("Three raptors are circling the camp at dusk. Light the perimeter torches and drive them off before they reach the food stores.");
        Q.QuestType = EQuest_Type::Defense;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Guard_Bren");
        Q.bIsRepeatable = true;
        Q.TimeLimit = 120.f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_LightTorches");
        Obj1.Description = TEXT("Light 4 perimeter torches");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.RequiredCount = 4;
        Obj1.CurrentCount = 0;
        Obj1.bIsCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_DriveOffRaptors");
        Obj2.Description = TEXT("Drive off or kill the 3 raptors");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj2.RequiredCount = 3;
        Obj2.CurrentCount = 0;
        Obj2.bIsCompleted = false;
        Q.Objectives.Add(Obj2);

        Q.Reward.BoneTokens = 25;
        Q.Reward.UnlockedRecipes.Add(TEXT("RECIPE_TorchBundle"));
        Q.Reward.NarrativeUnlock = TEXT("LORE_RaptorNightHunting");

        RegisterQuest(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Registered %d default quests"), QuestRegistry.Num());
}

// ============================================================
// Quest lifecycle
// ============================================================

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] RegisterQuest: empty QuestID — skipped"));
        return;
    }
    QuestRegistry.Add(QuestData.QuestID, QuestData);
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] StartQuest: QuestID '%s' not found"), *QuestID);
        return false;
    }
    if (Quest->Status == EQuest_Status::Active)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] StartQuest: '%s' already active"), *QuestID);
        return false;
    }
    if (Quest->Status == EQuest_Status::Completed && !Quest->bIsRepeatable)
    {
        return false;
    }

    // Reset objectives for repeatable quests
    if (Quest->bIsRepeatable)
    {
        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            Obj.CurrentCount = 0;
            Obj.bIsCompleted = false;
        }
    }

    Quest->Status = EQuest_Status::Active;
    ActiveQuestIDs.AddUnique(QuestID);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest STARTED: '%s'"), *Quest->QuestTitle);
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
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);

    GrantReward(Quest->Reward);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest COMPLETED: '%s' — Reward: %d BoneTokens"), *Quest->QuestTitle, Quest->Reward.BoneTokens);
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
    ActiveQuestIDs.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest FAILED: '%s'"), *Quest->QuestTitle);
    return true;
}

bool UQuestManager::AbandonQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return false;
    }

    Quest->Status = EQuest_Status::Inactive;
    ActiveQuestIDs.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ABANDONED: '%s'"), *Quest->QuestTitle);
    return true;
}

// ============================================================
// Objective tracking
// ============================================================

void UQuestManager::NotifyObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return;
    }

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective DONE: '%s' in quest '%s'"), *ObjectiveID, *QuestID);
            }
            break;
        }
    }

    CheckObjectiveCompletion(*Quest);
}

void UQuestManager::NotifyDinosaurKilled(const FString& DinosaurSpecies)
{
    for (const FString& QID : ActiveQuestIDs)
    {
        FQuest_Data* Quest = QuestRegistry.Find(QID);
        if (!Quest) continue;

        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Hunt && !Obj.bIsCompleted)
            {
                Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                }
                UE_LOG(LogTemp, Log, TEXT("[QuestManager] Hunt progress: %s killed (%d/%d) in '%s'"),
                    *DinosaurSpecies, Obj.CurrentCount, Obj.RequiredCount, *Quest->QuestTitle);
            }
        }
        CheckObjectiveCompletion(*Quest);
    }
}

void UQuestManager::NotifyResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (const FString& QID : ActiveQuestIDs)
    {
        FQuest_Data* Quest = QuestRegistry.Find(QID);
        if (!Quest) continue;

        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Gather && !Obj.bIsCompleted)
            {
                Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                }
            }
        }
        CheckObjectiveCompletion(*Quest);
    }
}

void UQuestManager::NotifyLocationReached(const FVector& PlayerLocation)
{
    for (const FString& QID : ActiveQuestIDs)
    {
        FQuest_Data* Quest = QuestRegistry.Find(QID);
        if (!Quest) continue;

        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Reach && !Obj.bIsCompleted)
            {
                float Dist = FVector::Dist(PlayerLocation, Obj.TargetLocation);
                if (Dist <= Obj.LocationRadius)
                {
                    Obj.CurrentCount = Obj.RequiredCount;
                    Obj.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Location reached for objective '%s'"), *Obj.ObjectiveID);
                }
            }
        }
        CheckObjectiveCompletion(*Quest);
    }
}

void UQuestManager::NotifyItemCrafted(const FString& ItemID)
{
    for (const FString& QID : ActiveQuestIDs)
    {
        FQuest_Data* Quest = QuestRegistry.Find(QID);
        if (!Quest) continue;

        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Craft && !Obj.bIsCompleted)
            {
                Obj.CurrentCount = Obj.RequiredCount;
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestManager] Craft objective done: '%s' crafted"), *ItemID);
            }
        }
        CheckObjectiveCompletion(*Quest);
    }
}

// ============================================================
// Crowd integration — stampede/alert quests
// ============================================================

void UQuestManager::NotifyHerdAlerted(const FString& GroupID, float AlertLevel)
{
    if (AlertLevel >= 0.8f && !bStampedeQuestActive)
    {
        // Auto-trigger stampede survival quest
        if (StartQuest(TEXT("SURVIVE_001_Stampede")))
        {
            bStampedeQuestActive = true;
            StampedeQuestTimer = 0.f;
            UE_LOG(LogTemp, Log, TEXT("[QuestManager] STAMPEDE QUEST triggered by group '%s' alert %.2f"), *GroupID, AlertLevel);
        }
    }
}

void UQuestManager::NotifyPlayerEscapedAlert(float DistanceFromHerd)
{
    if (bStampedeQuestActive && DistanceFromHerd >= 2000.f)
    {
        // Complete the escape objective
        NotifyObjectiveProgress(TEXT("SURVIVE_001_Stampede"), TEXT("OBJ_EscapeStampede"), 1);
        bStampedeQuestActive = false;
        UE_LOG(LogTemp, Log, TEXT("[QuestManager] Player escaped stampede at distance %.0f"), DistanceFromHerd);
    }
}

// ============================================================
// Query
// ============================================================

FQuest_Data UQuestManager::GetQuestData(const FString& QuestID) const
{
    const FQuest_Data* Found = QuestRegistry.Find(QuestID);
    return Found ? *Found : FQuest_Data();
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const FString& QID : ActiveQuestIDs)
    {
        const FQuest_Data* Q = QuestRegistry.Find(QID);
        if (Q) Result.Add(*Q);
    }
    return Result;
}

TArray<FQuest_Data> UQuestManager::GetCompletedQuests() const
{
    TArray<FQuest_Data> Result;
    for (const FString& QID : CompletedQuestIDs)
    {
        const FQuest_Data* Q = QuestRegistry.Find(QID);
        if (Q) Result.Add(*Q);
    }
    return Result;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

int32 UQuestManager::GetActiveQuestCount() const
{
    return ActiveQuestIDs.Num();
}

// ============================================================
// Internal helpers
// ============================================================

void UQuestManager::CheckObjectiveCompletion(FQuest_Data& Quest)
{
    if (Quest.Status != EQuest_Status::Active) return;

    if (AllObjectivesComplete(Quest))
    {
        CompleteQuest(Quest.QuestID);
    }
}

bool UQuestManager::AllObjectivesComplete(const FQuest_Data& Quest) const
{
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted) return false;
    }
    return Quest.Objectives.Num() > 0;
}

void UQuestManager::GrantReward(const FQuest_Reward& Reward)
{
    // Reward integration — BoneTokens go to player inventory when InventorySystem is available
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Reward granted: %d BoneTokens, %d recipes unlocked, %d areas unlocked"),
        Reward.BoneTokens,
        Reward.UnlockedRecipes.Num(),
        Reward.UnlockedAreas.Num());

    if (!Reward.NarrativeUnlock.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("[QuestManager] Narrative unlock: %s"), *Reward.NarrativeUnlock);
    }
}
