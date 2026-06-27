// QuestSystemManager.cpp
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260627_004
// Implements quest lifecycle, objective tracking, and default prehistoric survival quests.

#include "QuestSystemManager.h"
#include "Engine/World.h"

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ActiveQuestCount = 0;
    CompletedQuestCount = 0;
}

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ─── Quest lifecycle ──────────────────────────────────────────────────────────

void AQuestSystemManager::StartQuest(const FString& QuestID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.State == EQuest_State::Inactive)
        {
            Quest.State = EQuest_State::Active;
            ActiveQuestCount++;
            UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest started: %s"), *Quest.QuestTitle);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Quest not found or already active: %s"), *QuestID);
}

void AQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.State == EQuest_State::Active)
        {
            for (FQuest_Objective& Obj : Quest.Objectives)
            {
                if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
                {
                    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + 1, Obj.RequiredCount);
                    if (Obj.CurrentCount >= Obj.RequiredCount)
                    {
                        Obj.bCompleted = true;
                        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Objective completed: %s in quest %s"),
                            *ObjectiveID, *QuestID);
                    }
                    CheckQuestCompletion(Quest);
                    return;
                }
            }
        }
    }
}

void AQuestSystemManager::FailQuest(const FString& QuestID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.State == EQuest_State::Active)
        {
            Quest.State = EQuest_State::Failed;
            ActiveQuestCount = FMath::Max(0, ActiveQuestCount - 1);
            UE_LOG(LogTemp, Warning, TEXT("[QuestSystem] Quest failed: %s"), *Quest.QuestTitle);
            return;
        }
    }
}

bool AQuestSystemManager::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.State == EQuest_State::Active;
        }
    }
    return false;
}

bool AQuestSystemManager::IsQuestCompleted(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.State == EQuest_State::Completed;
        }
    }
    return false;
}

FQuest_Data AQuestSystemManager::GetQuestData(const FString& QuestID) const
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

TArray<FQuest_Data> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Active;
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.State == EQuest_State::Active)
        {
            Active.Add(Quest);
        }
    }
    return Active;
}

// ─── Internal helpers ─────────────────────────────────────────────────────────

void AQuestSystemManager::CheckQuestCompletion(FQuest_Data& Quest)
{
    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllDone = false;
            break;
        }
    }
    if (bAllDone && Quest.Objectives.Num() > 0)
    {
        Quest.State = EQuest_State::Completed;
        ActiveQuestCount = FMath::Max(0, ActiveQuestCount - 1);
        CompletedQuestCount++;
        UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Quest COMPLETED: %s | XP Reward: %d"),
            *Quest.QuestTitle, Quest.RewardXP);
    }
}

void AQuestSystemManager::InitializeDefaultQuests()
{
    AllQuests.Empty();

    // ── Quest 1: The First Hunt ───────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_FIRST_HUNT");
        Q.QuestTitle = TEXT("The First Hunt");
        Q.QuestDescription = TEXT("Track and kill a small dinosaur to feed the tribe. "
            "Follow the tracks east of camp and use your spear.");
        Q.State = EQuest_State::Inactive;
        Q.RewardXP = 150;
        Q.bIsMainQuest = true;

        FQuest_Objective O1;
        O1.ObjectiveID = TEXT("OBJ_FIND_TRACKS");
        O1.Description = TEXT("Find dinosaur tracks near the river");
        O1.RequiredCount = 1;
        Q.Objectives.Add(O1);

        FQuest_Objective O2;
        O2.ObjectiveID = TEXT("OBJ_KILL_DINO");
        O2.Description = TEXT("Kill a small dinosaur");
        O2.RequiredCount = 1;
        Q.Objectives.Add(O2);

        FQuest_Objective O3;
        O3.ObjectiveID = TEXT("OBJ_RETURN_CAMP");
        O3.Description = TEXT("Return to camp with the kill");
        O3.RequiredCount = 1;
        Q.Objectives.Add(O3);

        AllQuests.Add(Q);
    }

    // ── Quest 2: Build a Shelter ──────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_BUILD_SHELTER");
        Q.QuestTitle = TEXT("Build a Shelter");
        Q.QuestDescription = TEXT("Gather materials and construct a basic shelter "
            "before nightfall. Predators are more active in darkness.");
        Q.State = EQuest_State::Inactive;
        Q.RewardXP = 100;
        Q.bIsMainQuest = true;

        FQuest_Objective O1;
        O1.ObjectiveID = TEXT("OBJ_GATHER_STICKS");
        O1.Description = TEXT("Gather 5 sticks");
        O1.RequiredCount = 5;
        Q.Objectives.Add(O1);

        FQuest_Objective O2;
        O2.ObjectiveID = TEXT("OBJ_GATHER_ROCKS");
        O2.Description = TEXT("Gather 3 rocks");
        O2.RequiredCount = 3;
        Q.Objectives.Add(O2);

        FQuest_Objective O3;
        O3.ObjectiveID = TEXT("OBJ_BUILD_SHELTER");
        O3.Description = TEXT("Construct the shelter at the marked location");
        O3.RequiredCount = 1;
        Q.Objectives.Add(O3);

        AllQuests.Add(Q);
    }

    // ── Quest 3: Defend the Camp ──────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_DEFEND_CAMP");
        Q.QuestTitle = TEXT("Night Attack");
        Q.QuestDescription = TEXT("A pack of raptors has been spotted circling the camp. "
            "Light fires at the perimeter and drive them off.");
        Q.State = EQuest_State::Inactive;
        Q.RewardXP = 200;
        Q.bIsMainQuest = false;

        FQuest_Objective O1;
        O1.ObjectiveID = TEXT("OBJ_LIGHT_FIRES");
        O1.Description = TEXT("Light 3 perimeter fires");
        O1.RequiredCount = 3;
        Q.Objectives.Add(O1);

        FQuest_Objective O2;
        O2.ObjectiveID = TEXT("OBJ_REPEL_RAPTORS");
        O2.Description = TEXT("Drive off the raptor pack");
        O2.RequiredCount = 4;
        Q.Objectives.Add(O2);

        AllQuests.Add(Q);
    }

    // ── Quest 4: Follow the Migration ────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_MIGRATION");
        Q.QuestTitle = TEXT("Follow the Herd");
        Q.QuestDescription = TEXT("A massive herd of herbivores is migrating north. "
            "Follow them to discover new hunting grounds and water sources.");
        Q.State = EQuest_State::Inactive;
        Q.RewardXP = 175;
        Q.bIsMainQuest = false;

        FQuest_Objective O1;
        O1.ObjectiveID = TEXT("OBJ_FIND_HERD");
        O1.Description = TEXT("Locate the migrating herd");
        O1.RequiredCount = 1;
        Q.Objectives.Add(O1);

        FQuest_Objective O2;
        O2.ObjectiveID = TEXT("OBJ_CROSS_RIVER");
        O2.Description = TEXT("Cross the river with the herd");
        O2.RequiredCount = 1;
        Q.Objectives.Add(O2);

        FQuest_Objective O3;
        O3.ObjectiveID = TEXT("OBJ_DISCOVER_VALLEY");
        O3.Description = TEXT("Discover the northern valley");
        O3.RequiredCount = 1;
        Q.Objectives.Add(O3);

        AllQuests.Add(Q);
    }

    // ── Quest 5: Explore the Cave ─────────────────────────────────────────────
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_CAVE_EXPLORE");
        Q.QuestTitle = TEXT("Into the Dark");
        Q.QuestDescription = TEXT("Scouts report a large cave to the northwest. "
            "Explore it for shelter, flint deposits, and signs of other predators.");
        Q.State = EQuest_State::Inactive;
        Q.RewardXP = 225;
        Q.bIsMainQuest = false;

        FQuest_Objective O1;
        O1.ObjectiveID = TEXT("OBJ_MAKE_TORCH");
        O1.Description = TEXT("Craft a torch before entering");
        O1.RequiredCount = 1;
        Q.Objectives.Add(O1);

        FQuest_Objective O2;
        O2.ObjectiveID = TEXT("OBJ_FIND_FLINT");
        O2.Description = TEXT("Find flint deposits inside the cave");
        O2.RequiredCount = 3;
        Q.Objectives.Add(O2);

        FQuest_Objective O3;
        O3.ObjectiveID = TEXT("OBJ_MAP_CAVE");
        O3.Description = TEXT("Map the cave passages");
        O3.RequiredCount = 1;
        Q.Objectives.Add(O3);

        AllQuests.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("[QuestSystem] Initialized %d default quests"), AllQuests.Num());
}
