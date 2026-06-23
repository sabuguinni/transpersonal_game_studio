#include "QuestManager.h"
#include "Engine/World.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxActiveQuests = 5;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQuestManager::InitializeDefaultQuests()
{
    // Quest 1: Clear the Raptor Pass
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_RAPTOR_PASS");
        Q.QuestName = TEXT("Clear the Raptor Pass");
        Q.Description = TEXT("The raptor pack has taken the eastern pass. Eliminate the threat to restore safe travel.");
        Q.QuestType = EQuest_Type::Hunt;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Elder_01");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_KILL_RAPTORS");
        Obj1.Description = TEXT("Kill 3 raptors in the eastern pass");
        Obj1.RequiredCount = 3;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_COLLECT_CLAWS");
        Obj2.Description = TEXT("Collect 3 raptor claws as proof");
        Obj2.RequiredCount = 3;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward1;
        Reward1.ItemID = TEXT("ITEM_IRON_SPEAR");
        Reward1.Quantity = 2;
        Reward1.Description = TEXT("Iron-tipped spears");
        Q.Rewards.Add(Reward1);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // Quest 2: Investigate the Valley
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_BRACH_VALLEY");
        Q.QuestName = TEXT("Investigate the Valley");
        Q.Description = TEXT("The Brachiosaurus herd has been disturbed. Find out what is driving them north.");
        Q.QuestType = EQuest_Type::Explore;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Scout_01");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_REACH_VALLEY");
        Obj1.Description = TEXT("Reach the Brachiosaurus feeding ground");
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_FIND_CAUSE");
        Obj2.Description = TEXT("Identify the cause of the herd disturbance");
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward1;
        Reward1.ItemID = TEXT("ITEM_MEAT_DRIED");
        Reward1.Quantity = 5;
        Reward1.Description = TEXT("Dried meat rations");
        Q.Rewards.Add(Reward1);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // Quest 3: First Hunt — Stone Axe Materials
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_FIRST_HUNT");
        Q.QuestName = TEXT("First Hunt");
        Q.Description = TEXT("Gather materials to craft a stone axe before attempting to hunt larger prey.");
        Q.QuestType = EQuest_Type::Gather;
        Q.Status = EQuest_Status::Active;
        Q.GiverNPCID = TEXT("NPC_Craftsman_01");
        Q.bIsMainQuest = true;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_GATHER_ROCKS");
        Obj1.Description = TEXT("Collect 2 sharp rocks");
        Obj1.RequiredCount = 2;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_GATHER_STICKS");
        Obj2.Description = TEXT("Collect 1 sturdy stick");
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Objective Obj3;
        Obj3.ObjectiveID = TEXT("OBJ_CRAFT_AXE");
        Obj3.Description = TEXT("Craft the stone axe at a workbench");
        Obj3.RequiredCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.bCompleted = false;
        Q.Objectives.Add(Obj3);

        FQuest_Reward Reward1;
        Reward1.ItemID = TEXT("ITEM_STONE_AXE");
        Reward1.Quantity = 1;
        Reward1.Description = TEXT("Stone Axe — basic hunting tool");
        Q.Rewards.Add(Reward1);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    // Quest 4: Defend the Camp
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("QUEST_DEFEND_CAMP");
        Q.QuestName = TEXT("Night Defense");
        Q.Description = TEXT("A predator has been circling the camp at night. Set traps and defend the perimeter before dark.");
        Q.QuestType = EQuest_Type::Defend;
        Q.Status = EQuest_Status::Inactive;
        Q.GiverNPCID = TEXT("NPC_Guard_01");
        Q.bIsMainQuest = false;

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("OBJ_SET_TRAPS");
        Obj1.Description = TEXT("Place 3 spike traps around the camp perimeter");
        Obj1.RequiredCount = 3;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("OBJ_SURVIVE_NIGHT");
        Obj2.Description = TEXT("Survive until dawn");
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        FQuest_Reward Reward1;
        Reward1.ItemID = TEXT("ITEM_HIDE_ARMOR");
        Reward1.Quantity = 1;
        Reward1.Description = TEXT("Hide armor — basic protection");
        Q.Rewards.Add(Reward1);

        QuestRegistry.Add(Q.QuestID, Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized %d quests"), QuestRegistry.Num());
}

bool AQuestManager::ActivateQuest(const FString& QuestID)
{
    if (!QuestRegistry.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }

    int32 ActiveCount = GetActiveQuestCount();
    if (ActiveCount >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Max active quests (%d) reached"), MaxActiveQuests);
        return false;
    }

    FQuest_Data& Quest = QuestRegistry[QuestID];
    if (Quest.Status != EQuest_Status::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is not inactive (status=%d)"), *QuestID, (int32)Quest.Status);
        return false;
    }

    Quest.Status = EQuest_Status::Active;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Activated quest '%s'"), *Quest.QuestName);
    return true;
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!QuestRegistry.Contains(QuestID))
    {
        return false;
    }

    FQuest_Data& Quest = QuestRegistry[QuestID];
    if (Quest.Status != EQuest_Status::Active)
    {
        return false;
    }

    if (!AreAllObjectivesComplete(Quest))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot complete quest '%s' — objectives not all done"), *Quest.QuestName);
        return false;
    }

    Quest.Status = EQuest_Status::Completed;
    GrantRewards(Quest);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed quest '%s'"), *Quest.QuestName);
    return true;
}

bool AQuestManager::FailQuest(const FString& QuestID)
{
    if (!QuestRegistry.Contains(QuestID))
    {
        return false;
    }

    FQuest_Data& Quest = QuestRegistry[QuestID];
    Quest.Status = EQuest_Status::Failed;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Failed quest '%s'"), *Quest.QuestName);
    return true;
}

bool AQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    if (!QuestRegistry.Contains(QuestID))
    {
        return false;
    }

    FQuest_Data& Quest = QuestRegistry[QuestID];
    if (Quest.Status != EQuest_Status::Active)
    {
        return false;
    }

    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + Amount, 0, Obj.RequiredCount);
            if (Obj.CurrentCount >= Obj.RequiredCount)
            {
                Obj.bCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' completed in quest '%s'"), *ObjectiveID, *Quest.QuestName);
            }
            return true;
        }
    }

    return false;
}

FQuest_Data AQuestManager::GetQuestData(const FString& QuestID) const
{
    if (QuestRegistry.Contains(QuestID))
    {
        return QuestRegistry[QuestID];
    }
    return FQuest_Data();
}

TArray<FQuest_Data> AQuestManager::GetActiveQuests() const
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

void AQuestManager::RegisterQuest(const FQuest_Data& QuestData)
{
    if (!QuestData.QuestID.IsEmpty())
    {
        QuestRegistry.Add(QuestData.QuestID, QuestData);
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered quest '%s'"), *QuestData.QuestName);
    }
}

int32 AQuestManager::GetActiveQuestCount() const
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

bool AQuestManager::HasQuest(const FString& QuestID) const
{
    return QuestRegistry.Contains(QuestID);
}

bool AQuestManager::AreAllObjectivesComplete(const FQuest_Data& Quest) const
{
    for (const FQuest_Objective& Obj : Quest.Objectives)
    {
        if (!Obj.bCompleted)
        {
            return false;
        }
    }
    return true;
}

void AQuestManager::GrantRewards(const FQuest_Data& Quest)
{
    for (const FQuest_Reward& Reward : Quest.Rewards)
    {
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Granting reward '%s' x%d"), *Reward.ItemID, Reward.Quantity);
        // Integration point: call InventorySystem to add items
    }
}
