#include "QuestSystemManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ============================================================
// Quest System Manager — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260625_003
// Three survival quests tied to MinPlayableMap ecosystem
// ============================================================

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f;
}

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Initialized with %d quests"), AllQuests.Num());
}

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= QuestCheckInterval)
    {
        TimeSinceLastCheck = 0.0f;
        CheckProximityObjectives();
    }
}

// ---- Quest Registration ----

void AQuestSystemManager::RegisterQuest(const FQuest_Definition& Quest)
{
    // Prevent duplicate registration
    for (const FQuest_Definition& Existing : AllQuests)
    {
        if (Existing.QuestID == Quest.QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Quest already registered: %s"), *Quest.QuestID);
            return;
        }
    }
    AllQuests.Add(Quest);
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Registered quest: %s"), *Quest.QuestTitle);
}

bool AQuestSystemManager::ActivateQuest(const FString& QuestID)
{
    int32 ActiveCount = GetActiveQuestCount();
    if (ActiveCount >= MaxConcurrentQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Max concurrent quests reached (%d)"), MaxConcurrentQuests);
        return false;
    }

    for (FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EQuest_State::Inactive)
        {
            Quest.QuestState = EQuest_State::Active;
            UE_LOG(LogTemp, Log, TEXT("[QuestSystem] ACTIVATED: %s"), *Quest.QuestTitle);
            return true;
        }
    }
    return false;
}

bool AQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    for (FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestID != QuestID || Quest.QuestState != EQuest_State::Active)
            continue;

        for (FQuest_Objective& Obj : Quest.Objectives)
        {
            if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Objective complete: %s / %s"), *QuestID, *ObjectiveID);

                if (AreAllObjectivesComplete(Quest))
                {
                    Quest.QuestState = EQuest_State::Completed;
                    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] QUEST COMPLETE: %s"), *Quest.QuestTitle);
                }
                return true;
            }
        }
    }
    return false;
}

bool AQuestSystemManager::FailQuest(const FString& QuestID)
{
    for (FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EQuest_State::Active)
        {
            Quest.QuestState = EQuest_State::Failed;
            UE_LOG(LogTemp, Log, TEXT("[QuestSystem] QUEST FAILED: %s"), *Quest.QuestTitle);
            return true;
        }
    }
    return false;
}

// ---- Ecosystem-Driven Triggers ----

void AQuestSystemManager::OnHerdPanic(const FString& HerdGroupID, FVector PanicOrigin)
{
    // "Follow the Herd" quest activates when Parasaurolophus herd panics
    if (HerdGroupID.Contains(TEXT("Para")) || HerdGroupID.Contains(TEXT("Herd")))
    {
        ActivateQuest(TEXT("QUEST_FOLLOW_THE_HERD"));
        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Herd panic triggered quest at (%.0f, %.0f, %.0f)"),
            PanicOrigin.X, PanicOrigin.Y, PanicOrigin.Z);
    }
}

void AQuestSystemManager::OnPredatorDetected(const FString& PredatorLabel, FVector PredatorLocation)
{
    // "Raptor Ambush" quest activates when raptor pack is detected
    if (PredatorLabel.Contains(TEXT("Raptor")) || PredatorLabel.Contains(TEXT("Pack")))
    {
        ActivateQuest(TEXT("QUEST_RAPTOR_AMBUSH"));
        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Predator detected — raptor ambush quest activated: %s"), *PredatorLabel);
    }
    // "Survive the T-Rex" activates for apex predators
    if (PredatorLabel.Contains(TEXT("Trex")) || PredatorLabel.Contains(TEXT("TRex")))
    {
        ActivateQuest(TEXT("QUEST_SURVIVE_TREX"));
        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] T-Rex detected — survival quest activated"));
    }
}

void AQuestSystemManager::OnPlayerEnterQuestZone(const FString& ZoneID, FVector PlayerLocation)
{
    if (ZoneID.Contains(TEXT("FollowTheHerd")))
    {
        ActivateQuest(TEXT("QUEST_FOLLOW_THE_HERD"));
    }
    else if (ZoneID.Contains(TEXT("RaptorAmbush")))
    {
        ActivateQuest(TEXT("QUEST_RAPTOR_AMBUSH"));
    }
    else if (ZoneID.Contains(TEXT("WoundedTrike")))
    {
        ActivateQuest(TEXT("QUEST_WOUNDED_TRIKE"));
    }
    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Player entered zone: %s"), *ZoneID);
}

// ---- Query ----

TArray<FQuest_Definition> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuest_Definition> Active;
    for (const FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestState == EQuest_State::Active)
            Active.Add(Quest);
    }
    return Active;
}

FQuest_Definition AQuestSystemManager::GetQuestByID(const FString& QuestID) const
{
    for (const FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
            return Quest;
    }
    return FQuest_Definition();
}

int32 AQuestSystemManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestState == EQuest_State::Active)
            Count++;
    }
    return Count;
}

// ---- Private ----

void AQuestSystemManager::InitializeDefaultQuests()
{
    // ---- Quest 1: Follow the Herd (Migration) ----
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("QUEST_FOLLOW_THE_HERD");
        Q.QuestTitle = TEXT("Follow the Herd");
        Q.QuestDescription = TEXT("The Parasaurolophus herd is moving north. Follow their tracks to find the river crossing before the raptors cut you off.");
        Q.QuestType = EQuest_Type::Migration;
        Q.QuestState = EQuest_State::Inactive;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_TRACKS");
        Obj1.Description = TEXT("Find the Parasaurolophus tracks near the eastern waterhole");
        Obj1.TargetLocation = FVector(2600, 3200, 400);
        Obj1.ProximityRadius = 600.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_REACH_RIVER");
        Obj2.Description = TEXT("Follow the herd to the northern river crossing");
        Obj2.TargetLocation = FVector(2800, 4500, 400);
        Obj2.ProximityRadius = 800.0f;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // ---- Quest 2: Raptor Ambush (Defense) ----
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("QUEST_RAPTOR_AMBUSH");
        Q.QuestTitle = TEXT("Raptor Ambush");
        Q.QuestDescription = TEXT("A raptor pack is flanking from the south. Reach high ground before they surround you.");
        Q.QuestType = EQuest_Type::Defense;
        Q.QuestState = EQuest_State::Inactive;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_DETECT_PACK");
        Obj1.Description = TEXT("Spot the raptor pack before they close in");
        Obj1.TargetLocation = FVector(1800, 3600, 400);
        Obj1.ProximityRadius = 700.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_REACH_HIGH_GROUND");
        Obj2.Description = TEXT("Climb to the rocky outcrop to the west");
        Obj2.TargetLocation = FVector(800, 2800, 800);
        Obj2.ProximityRadius = 500.0f;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    // ---- Quest 3: Wounded Triceratops (Crafting) ----
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("QUEST_WOUNDED_TRIKE");
        Q.QuestTitle = TEXT("Wounded Triceratops");
        Q.QuestDescription = TEXT("A Triceratops has been wounded near the eastern waterhole. Craft a spear and drive the predators away before nightfall.");
        Q.QuestType = EQuest_Type::Crafting;
        Q.QuestState = EQuest_State::Inactive;
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_GATHER_MATERIALS");
        Obj1.Description = TEXT("Gather 2 flint stones and 1 hardwood branch");
        Obj1.TargetLocation = FVector(3400, 2000, 400);
        Obj1.ProximityRadius = 900.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_CRAFT_SPEAR");
        Obj2.Description = TEXT("Craft a flint-tipped spear at the crafting stone");
        Obj2.TargetLocation = FVector(500, 500, 400);
        Obj2.ProximityRadius = 400.0f;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_DRIVE_PREDATORS");
        Obj3.Description = TEXT("Drive the predators away from the wounded Triceratops");
        Obj3.TargetLocation = FVector(3400, 2000, 400);
        Obj3.ProximityRadius = 600.0f;
        Q.Objectives.Add(Obj3);

        AllQuests.Add(Q);
    }

    // ---- Quest 4: Survive the T-Rex (Hunt/Survival) ----
    {
        FQuest_Definition Q;
        Q.QuestID = TEXT("QUEST_SURVIVE_TREX");
        Q.QuestTitle = TEXT("Survive the T-Rex");
        Q.QuestDescription = TEXT("A T-Rex has entered the valley. Stay hidden, do not run, and reach the cave shelter before it finds you.");
        Q.QuestType = EQuest_Type::Hunt;
        Q.QuestState = EQuest_State::Inactive;
        Q.bIsMainQuest = true;
        Q.TimeLimit = 180.0f; // 3 minutes

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_STAY_HIDDEN");
        Obj1.Description = TEXT("Stay out of the T-Rex line of sight for 60 seconds");
        Obj1.TargetLocation = FVector(2000, 2500, 400);
        Obj1.ProximityRadius = 2000.0f;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_REACH_CAVE");
        Obj2.Description = TEXT("Reach the cave shelter to the north");
        Obj2.TargetLocation = FVector(1500, 5000, 400);
        Obj2.ProximityRadius = 600.0f;
        Q.Objectives.Add(Obj2);

        AllQuests.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Default quests initialized: %d"), AllQuests.Num());
}

void AQuestSystemManager::CheckProximityObjectives()
{
    // Proximity checking requires player pawn — stub for now
    // Full implementation: get player pawn location, check distance to each active objective
    UWorld* World = GetWorld();
    if (!World) return;

    for (FQuest_Definition& Quest : AllQuests)
    {
        if (Quest.QuestState != EQuest_State::Active) continue;

        // Time limit check
        if (Quest.TimeLimit > 0.0f)
        {
            // TimeLimit tracking would decrement here in full implementation
        }
    }
}

bool AQuestSystemManager::AreAllObjectivesComplete(const FQuest_Definition& Quest) const
{
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted) return false;
    }
    return Quest.Objectives.Num() > 0;
}
