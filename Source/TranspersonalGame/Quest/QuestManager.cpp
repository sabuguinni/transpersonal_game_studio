#include "QuestManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
    bDebugLog = false;
    TimeSinceLastTick = 0.0f;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Initialized with %d quests"), AllQuests.Num());
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TimeSinceLastTick += DeltaTime;

    // Check timed quests every second
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State == EQuest_State::Active && Quest.TimeLimit > 0.0f)
        {
            Quest.TimeLimit -= DeltaTime;
            if (Quest.TimeLimit <= 0.0f)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}

// ============================================================
// Quest lifecycle
// ============================================================

bool AQuestManager::StartQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuestByID(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] StartQuest: Quest '%s' not found"), *QuestID);
        return false;
    }

    if (Quest->State == EQuest_State::Active)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] Quest '%s' already active"), *QuestID);
        return false;
    }

    if (Quest->State == EQuest_State::Completed && !Quest->bIsRepeatable)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestManager] Quest '%s' already completed and not repeatable"), *QuestID);
        return false;
    }

    Quest->State = EQuest_State::Active;
    ActiveQuestIDs.AddUnique(QuestID);

    // Reset objectives if repeating
    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        Obj.CurrentCount = 0;
        Obj.bIsCompleted = false;
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest STARTED: '%s' — %s"), *QuestID, *Quest->QuestTitle);
    return true;
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuestByID(QuestID);
    if (!Quest || Quest->State != EQuest_State::Active)
    {
        return false;
    }

    Quest->State = EQuest_State::Completed;
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);

    GrantReward(Quest->Reward);

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest COMPLETED: '%s' — Reward granted"), *QuestID);
    return true;
}

bool AQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuestByID(QuestID);
    if (!Quest || Quest->State != EQuest_State::Active)
    {
        return false;
    }

    Quest->State = EQuest_State::Failed;
    ActiveQuestIDs.Remove(QuestID);

    UE_LOG(LogTemp, Warning, TEXT("[QuestManager] Quest FAILED: '%s'"), *QuestID);
    return true;
}

void AQuestManager::AbandonQuest(const FString& QuestID)
{
    FQuest_Data* Quest = FindQuestByID(QuestID);
    if (!Quest) return;

    Quest->State = EQuest_State::Inactive;
    ActiveQuestIDs.Remove(QuestID);

    // Reset progress
    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        Obj.CurrentCount = 0;
        Obj.bIsCompleted = false;
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Quest ABANDONED: '%s'"), *QuestID);
}

// ============================================================
// Objective tracking
// ============================================================

void AQuestManager::NotifyObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    FQuest_Data* Quest = FindQuestByID(QuestID);
    if (!Quest || Quest->State != EQuest_State::Active) return;

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestManager] Objective '%s' completed in quest '%s'"), *ObjectiveID, *QuestID);
            }
            CheckObjectiveCompletion(*Quest);
            return;
        }
    }
}

void AQuestManager::NotifyKill(const FString& TargetTag)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Hunt &&
                Obj.TargetTag == TargetTag &&
                !Obj.bIsCompleted)
            {
                Obj.CurrentCount++;
                if (bDebugLog)
                {
                    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Kill registered: %s (%d/%d)"),
                        *TargetTag, Obj.CurrentCount, Obj.RequiredCount);
                }
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                }
                CheckObjectiveCompletion(Quest);
            }
        }
    }
}

void AQuestManager::NotifyGather(const FString& ItemTag)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Gather &&
                Obj.TargetTag == ItemTag &&
                !Obj.bIsCompleted)
            {
                Obj.CurrentCount++;
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                }
                CheckObjectiveCompletion(Quest);
            }
        }
    }
}

void AQuestManager::NotifyLocationReached(FVector Location)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Explore && !Obj.bIsCompleted)
            {
                float Distance = FVector::Dist(Location, Obj.TargetLocation);
                if (Distance <= Obj.TargetRadius)
                {
                    Obj.CurrentCount = Obj.RequiredCount;
                    Obj.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Explore objective reached: %s"), *Obj.ObjectiveID);
                    CheckObjectiveCompletion(Quest);
                }
            }
        }
    }
}

void AQuestManager::NotifyCraft(const FString& ItemTag)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State != EQuest_State::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Craft &&
                Obj.TargetTag == ItemTag &&
                !Obj.bIsCompleted)
            {
                Obj.CurrentCount++;
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                }
                CheckObjectiveCompletion(Quest);
            }
        }
    }
}

// ============================================================
// Herd integration — connects to DinosaurHerdManager
// ============================================================

void AQuestManager::OnStampedeTriggered(FVector StampedeOrigin)
{
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Stampede triggered at %s — checking survival quests"), *StampedeOrigin.ToString());

    // Activate "Survive the Stampede" quest if not already active
    FQuest_Data* SurvivalQuest = FindQuestByID(TEXT("QUEST_SURVIVE_STAMPEDE"));
    if (SurvivalQuest && SurvivalQuest->State == EQuest_State::Inactive)
    {
        StartQuest(TEXT("QUEST_SURVIVE_STAMPEDE"));
    }
}

void AQuestManager::OnHerdStateChanged(const FString& HerdSpecies, const FString& NewState)
{
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Herd '%s' state changed to '%s'"), *HerdSpecies, *NewState);

    // Trigger migration quest when herd starts migrating
    if (NewState == TEXT("Migrating"))
    {
        FQuest_Data* MigrationQuest = FindQuestByID(TEXT("QUEST_FOLLOW_MIGRATION"));
        if (MigrationQuest && MigrationQuest->State == EQuest_State::Inactive)
        {
            StartQuest(TEXT("QUEST_FOLLOW_MIGRATION"));
        }
    }

    // Trigger hunting quest when herd is grazing (easier to hunt)
    if (NewState == TEXT("Grazing"))
    {
        FQuest_Data* HuntQuest = FindQuestByID(TEXT("QUEST_HUNT_WHILE_GRAZING"));
        if (HuntQuest && HuntQuest->State == EQuest_State::Inactive)
        {
            StartQuest(TEXT("QUEST_HUNT_WHILE_GRAZING"));
        }
    }
}

// ============================================================
// Query
// ============================================================

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool AQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

FQuest_Data AQuestManager::GetQuestData(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    return FQuest_Data();
}

TArray<FQuest_Data> AQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State == EQuest_State::Active)
        {
            Result.Add(Quest);
        }
    }
    return Result;
}

int32 AQuestManager::GetActiveQuestCount() const
{
    return ActiveQuestIDs.Num();
}

// ============================================================
// Private helpers
// ============================================================

void AQuestManager::InitializeDefaultQuests()
{
    // ---- QUEST 1: Survive the Stampede ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_SURVIVE_STAMPEDE");
        Q.QuestTitle = TEXT("Survive the Stampede");
        Q.QuestDescription = TEXT("A massive herd is stampeding through the valley. Reach high ground before the herd crushes everything in its path.");
        Q.Category = EQuest_Category::Survival;
        Q.State = EQuest_State::Inactive;
        Q.GiverNPCTag = TEXT("NPC_StampedeWitness");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 120.0f; // 2 minutes to escape

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_REACH_HIGH_GROUND");
        Obj1.Description = TEXT("Reach the rocky outcrop before the herd arrives");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj1.TargetLocation = FVector(5000.0f, 2000.0f, 800.0f);
        Obj1.TargetRadius = 600.0f;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        Q.Reward.FlintStones = 5;
        Q.Reward.XPBonus = 150.0f;
        AllQuests.Add(Q);
    }

    // ---- QUEST 2: Follow the Migration ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_FOLLOW_MIGRATION");
        Q.QuestTitle = TEXT("Follow the Migration");
        Q.QuestDescription = TEXT("The great herd moves north. Follow them to discover new hunting grounds and water sources.");
        Q.Category = EQuest_Category::Exploration;
        Q.State = EQuest_State::Inactive;
        Q.GiverNPCTag = TEXT("NPC_HerdTracker");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.0f; // No time limit

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FOLLOW_HERD_NORTH");
        Obj1.Description = TEXT("Follow the herd to the northern valley");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj1.TargetLocation = FVector(8000.0f, 0.0f, 200.0f);
        Obj1.TargetRadius = 1000.0f;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_FIND_WATER_SOURCE");
        Obj2.Description = TEXT("Locate the water source the herd is heading toward");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj2.TargetLocation = FVector(10000.0f, 500.0f, 150.0f);
        Obj2.TargetRadius = 800.0f;
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        Q.Reward.Meat = 10;
        Q.Reward.XPBonus = 200.0f;
        AllQuests.Add(Q);
    }

    // ---- QUEST 3: Hunt While Grazing ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_HUNT_WHILE_GRAZING");
        Q.QuestTitle = TEXT("The Grazing Hour");
        Q.QuestDescription = TEXT("The herd is calm and grazing. This is the best time to hunt — take down a young Triceratops before they move on.");
        Q.Category = EQuest_Category::Hunting;
        Q.State = EQuest_State::Inactive;
        Q.GiverNPCTag = TEXT("NPC_HerdTracker");
        Q.bIsRepeatable = true;
        Q.TimeLimit = 300.0f; // 5 minutes while herd grazes

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_HUNT_TRICERATOPS");
        Obj1.Description = TEXT("Hunt 1 young Triceratops from the grazing herd");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj1.TargetTag = TEXT("Triceratops");
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_COLLECT_HIDE");
        Obj2.Description = TEXT("Collect the hide from the kill");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj2.TargetTag = TEXT("DinosaurHide");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        Q.Reward.Hides = 3;
        Q.Reward.Meat = 15;
        Q.Reward.Bones = 5;
        Q.Reward.XPBonus = 100.0f;
        AllQuests.Add(Q);
    }

    // ---- QUEST 4: Missing Scouts ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_MISSING_SCOUTS");
        Q.QuestTitle = TEXT("Missing Scouts");
        Q.QuestDescription = TEXT("Three hunters entered the valley two days ago. None returned. Find out what happened to them.");
        Q.Category = EQuest_Category::Tribal;
        Q.State = EQuest_State::Inactive;
        Q.GiverNPCTag = TEXT("NPC_TribalElder");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_CAMP_REMAINS");
        Obj1.Description = TEXT("Find the scouts' last camp in the valley");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Explore;
        Obj1.TargetLocation = FVector(3500.0f, -1500.0f, 100.0f);
        Obj1.TargetRadius = 500.0f;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_KILL_PREDATOR");
        Obj2.Description = TEXT("Eliminate the predator that attacked the scouts");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj2.TargetTag = TEXT("Predator");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_RECOVER_SUPPLIES");
        Obj3.Description = TEXT("Recover the scouts' supplies");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj3.TargetTag = TEXT("ScoutSupplies");
        Obj3.RequiredCount = 3;
        Q.Objectives.Add(Obj3);

        Q.Reward.FlintStones = 8;
        Q.Reward.Hides = 2;
        Q.Reward.XPBonus = 300.0f;
        AllQuests.Add(Q);
    }

    // ---- QUEST 5: First Crafting ----
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_FIRST_TOOLS");
        Q.QuestTitle = TEXT("Stone and Sinew");
        Q.QuestDescription = TEXT("You need better tools to survive. Gather materials and craft your first stone axe.");
        Q.Category = EQuest_Category::Crafting;
        Q.State = EQuest_State::Inactive;
        Q.GiverNPCTag = TEXT("NPC_Craftsman");
        Q.bIsRepeatable = false;
        Q.TimeLimit = 0.0f;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_GATHER_FLINT");
        Obj1.Description = TEXT("Gather 2 flint stones");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.TargetTag = TEXT("FlintStone");
        Obj1.RequiredCount = 2;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_GATHER_STICK");
        Obj2.Description = TEXT("Gather 1 sturdy stick");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj2.TargetTag = TEXT("Stick");
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_CRAFT_AXE");
        Obj3.Description = TEXT("Craft a stone axe at the crafting area");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj3.TargetTag = TEXT("StoneAxe");
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        Q.Reward.XPBonus = 75.0f;
        AllQuests.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestManager] %d default quests initialized"), AllQuests.Num());
}

void AQuestManager::CheckObjectiveCompletion(FQuest_Data& Quest)
{
    bool bAllComplete = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsCompleted)
        {
            bAllComplete = false;
            break;
        }
    }

    if (bAllComplete)
    {
        CompleteQuest(Quest.QuestID);
    }
}

void AQuestManager::GrantReward(const FQuest_Reward& Reward)
{
    // Log reward — actual inventory integration handled by CraftingSystem/InventorySystem
    UE_LOG(LogTemp, Log, TEXT("[QuestManager] Reward granted — Flint:%d Hides:%d Bones:%d Meat:%d XP:%.0f"),
        Reward.FlintStones, Reward.Hides, Reward.Bones, Reward.Meat, Reward.XPBonus);
}

FQuest_Data* AQuestManager::FindQuestByID(const FString& QuestID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}
