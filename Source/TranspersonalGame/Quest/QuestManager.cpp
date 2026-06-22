// QuestManager.cpp
// Quest & Mission Designer — Agent #14
// Full implementation of quest lifecycle world subsystem

#include "QuestManager.h"
#include "Engine/World.h"

UQuestManager::UQuestManager()
    : ElapsedTime(0.0f)
{
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultQuests();
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized with %d quests"), QuestRegistry.Num());
}

void UQuestManager::Deinitialize()
{
    QuestRegistry.Empty();
    Super::Deinitialize();
}

TStatId UQuestManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UQuestManager, STATGROUP_Tickables);
}

void UQuestManager::RegisterDefaultQuests()
{
    // -------------------------------------------------------
    // QUEST 1: Hunt the Predator
    // Giver: CrowdAgent_Elder_01 at tribal camp (800, 400)
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_HUNT_PREDATOR");
        Q.QuestName = TEXT("Hunt the Predator");
        Q.QuestDescription = TEXT("A large predator has been stalking the camp and taken two hunters. Track it and drive it away from our lands.");
        Q.QuestType = EQuest_Type::Hunt;
        Q.Status = EQuest_Status::Available;
        Q.GiverActorLabel = TEXT("CrowdAgent_Elder_01");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_FIND_PREDATOR");
        Obj1.Description = TEXT("Track the predator to its territory");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_KILL_PREDATOR");
        Obj2.Description = TEXT("Defeat the predator");
        Obj2.ObjectiveType = EQuest_ObjectiveType::KillTarget;
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        Q.Reward.ResourceBonus = 5;
        Q.Reward.HealthRestorePercent = 50.0f;
        Q.Reward.UnlockedRecipeID = TEXT("RECIPE_BONE_SPEAR");

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // -------------------------------------------------------
    // QUEST 2: Escort the Migrants
    // Giver: CrowdAgent_Scout_01 at migration column (-250, -250)
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_ESCORT_MIGRANTS");
        Q.QuestName = TEXT("Escort the Migrants");
        Q.QuestDescription = TEXT("A group of travelers is moving through raptor territory. Escort them safely to the tribal camp.");
        Q.QuestType = EQuest_Type::Escort;
        Q.Status = EQuest_Status::Available;
        Q.GiverActorLabel = TEXT("CrowdAgent_Scout_01");
        Q.bHasTimeLimit = true;
        Q.TimeLimit = 300.0f; // 5 minutes

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_MEET_MIGRANTS");
        Obj1.Description = TEXT("Find the migration column");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_ESCORT_SAFE");
        Obj2.Description = TEXT("Escort all 5 migrants to the camp");
        Obj2.ObjectiveType = EQuest_ObjectiveType::ProtectTarget;
        Obj2.RequiredCount = 5;
        Q.Objectives.Add(Obj2);

        Q.Reward.ResourceBonus = 8;
        Q.Reward.HealthRestorePercent = 25.0f;
        Q.Reward.UnlockedRecipeID = TEXT("RECIPE_CAMPFIRE");

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // -------------------------------------------------------
    // QUEST 3: Gather Resources
    // Zone: QuestZone_GatherResources at (200, -600)
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_GATHER_RESOURCES");
        Q.QuestName = TEXT("Gather Resources");
        Q.QuestDescription = TEXT("The camp needs materials to survive the coming cold. Gather rocks, sticks, and leaves from the valley.");
        Q.QuestType = EQuest_Type::Gather;
        Q.Status = EQuest_Status::Available;
        Q.GiverActorLabel = TEXT("CrowdAgent_Gatherer_01");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_COLLECT_ROCKS");
        Obj1.Description = TEXT("Collect 3 rocks");
        Obj1.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj1.RequiredCount = 3;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_COLLECT_STICKS");
        Obj2.Description = TEXT("Collect 3 sticks");
        Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_COLLECT_LEAVES");
        Obj3.Description = TEXT("Collect 2 large leaves");
        Obj3.ObjectiveType = EQuest_ObjectiveType::CollectItem;
        Obj3.RequiredCount = 2;
        Q.Objectives.Add(Obj3);

        Q.Reward.ResourceBonus = 3;
        Q.Reward.HealthRestorePercent = 20.0f;
        Q.Reward.UnlockedRecipeID = TEXT("RECIPE_STONE_AXE");

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // -------------------------------------------------------
    // QUEST 4: Defend the Camp
    // Zone: QuestZone_DefendCamp at (900, 600)
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_DEFEND_CAMP");
        Q.QuestName = TEXT("Defend the Camp");
        Q.QuestDescription = TEXT("Raptors have been circling the camp at night. Survive the night and repel the attack.");
        Q.QuestType = EQuest_Type::Defend;
        Q.Status = EQuest_Status::Locked;
        Q.GiverActorLabel = TEXT("CrowdAgent_Hunter_01");
        Q.bHasTimeLimit = true;
        Q.TimeLimit = 180.0f; // 3 minutes

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_SURVIVE_NIGHT");
        Obj1.Description = TEXT("Survive until dawn");
        Obj1.ObjectiveType = EQuest_ObjectiveType::SurviveTime;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_REPEL_RAPTORS");
        Obj2.Description = TEXT("Repel raptor attacks (kill 3)");
        Obj2.ObjectiveType = EQuest_ObjectiveType::KillTarget;
        Obj2.RequiredCount = 3;
        Q.Objectives.Add(Obj2);

        Q.Reward.ResourceBonus = 10;
        Q.Reward.HealthRestorePercent = 100.0f;
        Q.Reward.UnlockedRecipeID = TEXT("RECIPE_BONE_SHIELD");

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // -------------------------------------------------------
    // QUEST 5: Scout the Valley
    // Zone: QuestZone_ScoutValley at (-250, -250)
    // -------------------------------------------------------
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_SCOUT_VALLEY");
        Q.QuestName = TEXT("Scout the Valley");
        Q.QuestDescription = TEXT("We need to know what lies beyond the ridge. Scout three key locations and return safely.");
        Q.QuestType = EQuest_Type::Scout;
        Q.Status = EQuest_Status::Available;
        Q.GiverActorLabel = TEXT("CrowdAgent_Scout_01");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_SCOUT_RIVER");
        Obj1.Description = TEXT("Reach the river crossing");
        Obj1.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj1.RequiredCount = 1;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_SCOUT_RIDGE");
        Obj2.Description = TEXT("Reach the high ridge");
        Obj2.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj2.RequiredCount = 1;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_SCOUT_DINO_TERRITORY");
        Obj3.Description = TEXT("Observe the dinosaur territory from a safe distance");
        Obj3.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
        Obj3.RequiredCount = 1;
        Q.Objectives.Add(Obj3);

        Q.Reward.ResourceBonus = 4;
        Q.Reward.HealthRestorePercent = 30.0f;
        Q.Reward.UnlockedRecipeID = TEXT("RECIPE_WATER_CONTAINER");

        QuestRegistry.Add(Q.QuestID, Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered %d default quests"), QuestRegistry.Num());
}

void UQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot register quest with empty ID"));
        return;
    }
    QuestRegistry.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered quest '%s'"), *QuestData.QuestName);
}

bool UQuestManager::ActivateQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' not found"), *QuestID);
        return false;
    }
    if (Quest->Status != EQuest_Status::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' is not available (status=%d)"), *QuestID, (int32)Quest->Status);
        return false;
    }
    Quest->Status = EQuest_Status::Active;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Activated quest '%s'"), *Quest->QuestName);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest || Quest->Status != EQuest_Status::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : Quest->Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + ProgressAmount, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' completed in quest '%s'"), *ObjectiveID, *QuestID);
            }
            CheckQuestCompletion(QuestID);
            return true;
        }
    }
    return false;
}

void UQuestManager::CheckQuestCompletion(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return;

    bool bAllDone = true;
    for (const FQuest_Objective& Obj : Quest->Objectives)
    {
        if (!Obj.bCompleted)
        {
            bAllDone = false;
            break;
        }
    }

    if (bAllDone)
    {
        CompleteQuest(QuestID);
    }
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Completed;
    GrantReward(Quest->Reward);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' COMPLETED. Reward granted."), *Quest->QuestName);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (!Quest) return false;

    Quest->Status = EQuest_Status::Failed;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' FAILED."), *Quest->QuestName);
    return true;
}

void UQuestManager::GrantReward(const FQuest_Reward& Reward)
{
    // Reward integration point — CraftingSystem and CharacterStats will hook here
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Granting reward — Resources:%d Health:%.0f%% Recipe:%s"),
        Reward.ResourceBonus, Reward.HealthRestorePercent, *Reward.UnlockedRecipeID);
}

EQuest_Status UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    return Quest ? Quest->Status : EQuest_Status::Locked;
}

bool UQuestManager::GetQuestData(const FString& QuestID, FQuest_Data& OutData) const
{
    const FQuest_Data* Quest = QuestRegistry.Find(QuestID);
    if (Quest)
    {
        OutData = *Quest;
        return true;
    }
    return false;
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Active)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FQuest_Data> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Available)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

int32 UQuestManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const auto& Pair : QuestRegistry)
    {
        if (Pair.Value.Status == EQuest_Status::Active) Count++;
    }
    return Count;
}

void UQuestManager::OnDinoThreatDetected(const FString& DinoActorLabel, const FVector& ThreatLocation)
{
    // When a dino threat is detected near camp, unlock the Defend quest
    if (DinoActorLabel.Contains(TEXT("Raptor")) || DinoActorLabel.Contains(TEXT("Rex")))
    {
        FQuest_Data* DefendQuest = QuestRegistry.Find(TEXT("QUEST_DEFEND_CAMP"));
        if (DefendQuest && DefendQuest->Status == EQuest_Status::Locked)
        {
            DefendQuest->Status = EQuest_Status::Available;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: QUEST_DEFEND_CAMP unlocked by dino threat from '%s'"), *DinoActorLabel);
        }

        // Progress Hunt quest if active
        FQuest_Data* HuntQuest = QuestRegistry.Find(TEXT("QUEST_HUNT_PREDATOR"));
        if (HuntQuest && HuntQuest->Status == EQuest_Status::Active)
        {
            CompleteObjective(TEXT("QUEST_HUNT_PREDATOR"), TEXT("OBJ_FIND_PREDATOR"), 1);
        }
    }
}

void UQuestManager::Tick(float DeltaTime)
{
    ElapsedTime += DeltaTime;

    // Check time-limited quests
    for (auto& Pair : QuestRegistry)
    {
        FQuest_Data& Quest = Pair.Value;
        if (Quest.Status == EQuest_Status::Active && Quest.bHasTimeLimit)
        {
            Quest.TimeLimit -= DeltaTime;
            if (Quest.TimeLimit <= 0.0f)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}
