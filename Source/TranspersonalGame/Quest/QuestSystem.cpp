#include "QuestSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// Constructor
// ============================================================

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check quests at 2Hz
}

// ============================================================
// BeginPlay — register all prehistoric quests
// ============================================================

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializePrehistoricQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Initialized with %d quests"), QuestRegistry.Num());
}

// ============================================================
// Tick — check location-based objectives
// ============================================================

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player location for location-based objectives
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        ReportLocationReached(PlayerPawn->GetActorLocation());
    }
}

// ============================================================
// Quest Registration
// ============================================================

void AQuestSystemManager::RegisterQuest(const FQuest_Data& QuestData)
{
    QuestRegistry.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Registered quest: %s"), *QuestData.QuestName);
}

bool AQuestSystemManager::ActivateQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;
    if (Quest->Status != EQuest_Status::Available) return false;
    if (!ArePrerequisitesMet(*Quest)) return false;

    Quest->Status = EQuest_Status::Active;
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest ACTIVATED: %s"), *Quest->QuestName);
    return true;
}

bool AQuestSystemManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;
    if (Quest->Status != EQuest_Status::Active) return false;

    Quest->Status = EQuest_Status::Completed;
    GrantRewards(*Quest);

    // Unlock dependent quests
    CheckQuestUnlocks();

    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest COMPLETED: %s"), *Quest->QuestName);
    return true;
}

bool AQuestSystemManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;
    if (Quest->Status != EQuest_Status::Active) return false;

    Quest->Status = EQuest_Status::Failed;

    // Repeatable quests reset to Available
    if (Quest->bIsRepeatable)
    {
        Quest->Status = EQuest_Status::Available;
        for (FQuest_Objective& Obj : Quest->Objectives)
        {
            Obj.CurrentCount = 0;
            Obj.bIsCompleted = false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest FAILED: %s"), *Quest->QuestName);
    return true;
}

// ============================================================
// Objective Progress
// ============================================================

void AQuestSystemManager::ReportObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active) return;

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bIsCompleted)
        {
            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + ProgressDelta, 0, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Objective DONE: %s / %s"), *QuestID, *ObjectiveID);
            }
            break;
        }
    }

    CheckObjectiveCompletion(*Quest);
}

void AQuestSystemManager::ReportTagProgress(const FString& TargetTag, int32 ProgressDelta)
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status != EQuest_Status::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.TargetTag == TargetTag && !Obj.bIsCompleted)
            {
                Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + ProgressDelta, 0, Obj.RequiredCount);
                if (Obj.CurrentCount >= Obj.RequiredCount)
                {
                    Obj.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Tag objective done: %s in quest %s"), *TargetTag, *Quest.QuestID);
                }
                CheckObjectiveCompletion(Quest);
                break;
            }
        }
    }
}

void AQuestSystemManager::ReportLocationReached(const FVector& PlayerLocation)
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status != EQuest_Status::Active) continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveType == EQuest_ObjectiveType::Reach && !Obj.bIsCompleted)
            {
                float Dist = FVector::Dist(PlayerLocation, Obj.TargetLocation);
                if (Dist <= Obj.LocationRadius)
                {
                    Obj.CurrentCount = Obj.RequiredCount;
                    Obj.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Location reached for: %s"), *Obj.ObjectiveID);
                    CheckObjectiveCompletion(Quest);
                }
            }
        }
    }
}

// ============================================================
// Query
// ============================================================

bool AQuestSystemManager::IsQuestActive(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest && Quest->Status == EQuest_Status::Active;
}

bool AQuestSystemManager::IsQuestCompleted(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest && Quest->Status == EQuest_Status::Completed;
}

TArray<FQuest_Data> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Active)
            Result.Add(Pair.Value);
    }
    return Result;
}

TArray<FQuest_Data> AQuestSystemManager::GetAvailableQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Available)
            Result.Add(Pair.Value);
    }
    return Result;
}

FQuest_Data AQuestSystemManager::GetQuestByID(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest ? *Quest : FQuest_Data();
}

// ============================================================
// Internal helpers
// ============================================================

void AQuestSystemManager::CheckObjectiveCompletion(FQuest_Data& Quest)
{
    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bIsOptional && !Obj.bIsCompleted)
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone && Quest.Status == EQuest_Status::Active)
    {
        CompleteQuest(Quest.QuestID);
    }
}

void AQuestSystemManager::CheckQuestUnlocks()
{
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status == EQuest_Status::Locked && ArePrerequisitesMet(Quest))
        {
            Quest.Status = EQuest_Status::Available;
            UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest UNLOCKED: %s"), *Quest.QuestName);
        }
    }
}

bool AQuestSystemManager::ArePrerequisitesMet(const FQuest_Data& Quest) const
{
    for (const FString& PrereqID : Quest.PrerequisiteQuestIDs)
    {
        if (!IsQuestCompleted(PrereqID)) return false;
    }
    return true;
}

void AQuestSystemManager::GrantRewards(const FQuest_Data& Quest)
{
    for (const FQuest_Reward& Reward : Quest.Rewards)
    {
        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Reward granted: %s x%d (%s)"),
            *Reward.ItemID, Reward.Quantity, *Reward.Description);
        // Actual inventory integration: call InventoryComponent->AddItem(Reward.ItemID, Reward.Quantity)
    }
}

// ============================================================
// InitializePrehistoricQuests — 5 core quests
// ============================================================

void AQuestSystemManager::InitializePrehistoricQuests()
{
    // ---- QUEST 1: First Hunt (Tutorial) ----
    {
        FQuest_Data Q;
        Q.QuestID = "QUEST_FIRST_HUNT";
        Q.QuestName = "First Hunt";
        Q.Description = "The tribe needs meat. Hunt a small dinosaur near the river to prove your worth.";
        Q.Category = EQuest_Category::Hunting;
        Q.Status = EQuest_Status::Available;
        Q.bIsMainQuest = true;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_HUNT_SMALL_DINO";
        Obj1.Description = "Hunt 1 small dinosaur near the river";
        Obj1.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj1.TargetTag = "DinoSmall";
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_RETURN_CAMP";
        Obj2.Description = "Return to the camp";
        Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj2.TargetLocation = FVector(500.0f, -1000.0f, 0.0f); // Human camp location
        Obj2.LocationRadius = 800.0f;
        Q.Objectives.Add(Obj2);

        FQuest_Reward R;
        R.RewardType = EQuest_RewardType::Tool;
        R.ItemID = "ITEM_STONE_AXE";
        R.Quantity = 1;
        R.Description = "Stone Axe — crafted by the tribe elder";
        Q.Rewards.Add(R);

        RegisterQuest(Q);
    }

    // ---- QUEST 2: Stampede Survival ----
    {
        FQuest_Data Q;
        Q.QuestID = "QUEST_STAMPEDE_SURVIVAL";
        Q.QuestName = "Survive the Stampede";
        Q.Description = "A herd of herbivores has been spooked by a carnivore. The stampede is heading toward camp. Get out of the path and survive.";
        Q.Category = EQuest_Category::Emergency;
        Q.Status = EQuest_Status::Locked;
        Q.bIsRepeatable = false;
        Q.PrerequisiteQuestIDs.Add("QUEST_FIRST_HUNT");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_ESCAPE_STAMPEDE_PATH";
        Obj1.Description = "Escape the stampede path (move 1500 units north)";
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.TargetLocation = FVector(-3000.0f, 2000.0f, 0.0f);
        Obj1.LocationRadius = 600.0f;
        Obj1.TimeLimit = 60.0f; // 60 seconds
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_SURVIVE_DURATION";
        Obj2.Description = "Survive for 30 seconds while the herd passes";
        Obj2.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj2.TimeLimit = 30.0f;
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Reward R1;
        R1.RewardType = EQuest_RewardType::Knowledge;
        R1.ItemID = "KNOWLEDGE_HERD_BEHAVIOR";
        R1.Quantity = 1;
        R1.Description = "You now understand herd migration patterns";
        Q.Rewards.Add(R1);

        FQuest_Reward R2;
        R2.RewardType = EQuest_RewardType::MapReveal;
        R2.ItemID = "MAP_NORTHERN_PLAINS";
        R2.Quantity = 1;
        R2.Description = "Northern Plains revealed on map";
        Q.Rewards.Add(R2);

        RegisterQuest(Q);
    }

    // ---- QUEST 3: Craft a Campfire ----
    {
        FQuest_Data Q;
        Q.QuestID = "QUEST_FIRST_CAMPFIRE";
        Q.QuestName = "Fire Keeper";
        Q.Description = "Night is coming and the predators grow bold in darkness. Gather materials and build a campfire to protect the camp.";
        Q.Category = EQuest_Category::Crafting;
        Q.Status = EQuest_Status::Available;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_GATHER_STICKS";
        Obj1.Description = "Gather 3 sticks from the forest";
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.TargetTag = "ResourceStick";
        Obj1.RequiredCount = 3;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_CRAFT_CAMPFIRE";
        Obj2.Description = "Craft a campfire using the gathered sticks";
        Obj2.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj2.TargetTag = "ITEM_CAMPFIRE";
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Reward R;
        R.RewardType = EQuest_RewardType::Resource;
        R.ItemID = "ITEM_COOKED_MEAT";
        R.Quantity = 2;
        R.Description = "Cooked meat from the tribe stores";
        Q.Rewards.Add(R);

        RegisterQuest(Q);
    }

    // ---- QUEST 4: Rescue the Hunters ----
    {
        FQuest_Data Q;
        Q.QuestID = "QUEST_RESCUE_HUNTERS";
        Q.QuestName = "Rescue at the River";
        Q.Description = "Three hunters are trapped near the river crossing. A raptor pack has cut off their escape route. Reach them and escort them back safely.";
        Q.Category = EQuest_Category::Tribal;
        Q.Status = EQuest_Status::Locked;
        Q.PrerequisiteQuestIDs.Add("QUEST_FIRST_HUNT");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_REACH_RIVER_CROSSING";
        Obj1.Description = "Reach the river crossing";
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.TargetLocation = FVector(2000.0f, 1500.0f, 0.0f); // Herbivore herd area
        Obj1.LocationRadius = 700.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_DRIVE_OFF_RAPTORS";
        Obj2.Description = "Drive off or kill 2 raptors threatening the hunters";
        Obj2.ObjectiveType = EQuest_ObjectiveType::Hunt;
        Obj2.TargetTag = "DinoRaptor";
        Obj2.RequiredCount = 2;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = "OBJ_ESCORT_HUNTERS_BACK";
        Obj3.Description = "Escort the hunters back to camp";
        Obj3.ObjectiveType = EQuest_ObjectiveType::Escort;
        Obj3.TargetLocation = FVector(500.0f, -1000.0f, 0.0f);
        Obj3.LocationRadius = 800.0f;
        Q.Objectives.Add(Obj3);

        FQuest_Reward R1;
        R1.RewardType = EQuest_RewardType::TribeRank;
        R1.ItemID = "RANK_WARRIOR";
        R1.Quantity = 1;
        R1.Description = "You are now recognized as a Warrior of the tribe";
        Q.Rewards.Add(R1);

        FQuest_Reward R2;
        R2.RewardType = EQuest_RewardType::Tool;
        R2.ItemID = "ITEM_BONE_SPEAR";
        R2.Quantity = 1;
        R2.Description = "Bone Spear — crafted from the raptor you killed";
        Q.Rewards.Add(R2);

        RegisterQuest(Q);
    }

    // ---- QUEST 5: Track the Migration ----
    {
        FQuest_Data Q;
        Q.QuestID = "QUEST_TRACK_MIGRATION";
        Q.QuestName = "Follow the Herd";
        Q.Description = "The great herbivore herd is migrating south. Follow them to discover new water sources and hunting grounds — but stay clear of the carnivores that trail behind.";
        Q.Category = EQuest_Category::Exploration;
        Q.Status = EQuest_Status::Locked;
        Q.PrerequisiteQuestIDs.Add("QUEST_STAMPEDE_SURVIVAL");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = "OBJ_OBSERVE_HERD";
        Obj1.Description = "Observe the herd from a safe distance (within 1500 units)";
        Obj1.ObjectiveType = EQuest_ObjectiveType::Observe;
        Obj1.TargetLocation = FVector(2000.0f, 1500.0f, 0.0f);
        Obj1.LocationRadius = 1500.0f;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = "OBJ_FIND_WATER_SOURCE";
        Obj2.Description = "Follow the herd to discover a water source";
        Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj2.TargetLocation = FVector(4000.0f, 3000.0f, 0.0f);
        Obj2.LocationRadius = 1000.0f;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = "OBJ_AVOID_CARNIVORES";
        Obj3.Description = "Complete the journey without being attacked by carnivores";
        Obj3.ObjectiveType = EQuest_ObjectiveType::Survive;
        Obj3.TimeLimit = 120.0f;
        Obj3.RequiredCount = 1;
        Obj3.bIsOptional = false;
        Q.Objectives.Add(Obj3);

        FQuest_Reward R1;
        R1.RewardType = EQuest_RewardType::MapReveal;
        R1.ItemID = "MAP_SOUTHERN_WETLANDS";
        R1.Quantity = 1;
        R1.Description = "Southern Wetlands revealed — rich hunting and water";
        Q.Rewards.Add(R1);

        FQuest_Reward R2;
        R2.RewardType = EQuest_RewardType::Knowledge;
        R2.ItemID = "KNOWLEDGE_MIGRATION_ROUTES";
        R2.Quantity = 1;
        R2.Description = "You now know the seasonal migration routes";
        Q.Rewards.Add(R2);

        RegisterQuest(Q);
    }

    // Unlock quests with no prerequisites
    CheckQuestUnlocks();

    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] %d prehistoric quests initialized"), QuestRegistry.Num());
}
