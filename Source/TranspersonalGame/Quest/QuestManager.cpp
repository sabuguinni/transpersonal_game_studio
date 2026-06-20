#include "QuestManager.h"

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
    // Quest 1: Hunt the Raptor Pack
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("HUNT_RAPTOR_001");
        Q.QuestName = TEXT("Raptor Threat");
        Q.Description = TEXT("A pack of raptors has been spotted near the camp. Hunt them before they attack.");
        Q.QuestType = EQuest_Type::Hunt;
        Q.Status = EQuest_Status::Inactive;
        Q.RewardDescription = TEXT("Raptor claws — used for crafting sharp tools");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("KILL_RAPTORS");
        Obj1.Description = TEXT("Kill 3 raptors");
        Obj1.RequiredCount = 3;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        RegisterQuest(Q);
    }

    // Quest 2: Gather Flint Stones
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("GATHER_FLINT_001");
        Q.QuestName = TEXT("Stone Tools");
        Q.Description = TEXT("Collect flint stones from the riverbed to craft better weapons.");
        Q.QuestType = EQuest_Type::Gather;
        Q.Status = EQuest_Status::Inactive;
        Q.RewardDescription = TEXT("Stone Axe blueprint unlocked");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("COLLECT_FLINT");
        Obj1.Description = TEXT("Collect 5 flint stones");
        Obj1.RequiredCount = 5;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        RegisterQuest(Q);
    }

    // Quest 3: Survive the T-Rex Encounter
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("SURVIVE_TREX_001");
        Q.QuestName = TEXT("The Great Predator");
        Q.Description = TEXT("A T-Rex has entered the valley. Survive for 3 minutes without being caught.");
        Q.QuestType = EQuest_Type::Survive;
        Q.Status = EQuest_Status::Inactive;
        Q.RewardDescription = TEXT("Survival experience — stamina bonus");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("SURVIVE_TREX");
        Obj1.Description = TEXT("Survive for 3 minutes");
        Obj1.RequiredCount = 180;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        RegisterQuest(Q);
    }

    // Quest 4: Explore the Northern Ridge
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("EXPLORE_RIDGE_001");
        Q.QuestName = TEXT("Beyond the Ridge");
        Q.Description = TEXT("Scout the northern ridge to find new hunting grounds and water sources.");
        Q.QuestType = EQuest_Type::Explore;
        Q.Status = EQuest_Status::Inactive;
        Q.RewardDescription = TEXT("New map area revealed");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("REACH_RIDGE");
        Obj1.Description = TEXT("Reach the northern ridge");
        Obj1.RequiredCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        FQuest_Objective Obj2;
        Obj2.ObjectiveID = TEXT("FIND_WATER");
        Obj2.Description = TEXT("Find a water source");
        Obj2.RequiredCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.bCompleted = false;
        Q.Objectives.Add(Obj2);

        RegisterQuest(Q);
    }

    // Quest 5: Defend the Camp
    {
        FQuest_Data Q;
        Q.QuestID = TEXT("DEFEND_CAMP_001");
        Q.QuestName = TEXT("Night Attack");
        Q.Description = TEXT("Predators approach the camp at night. Defend until dawn.");
        Q.QuestType = EQuest_Type::Defend;
        Q.Status = EQuest_Status::Inactive;
        Q.RewardDescription = TEXT("Camp fortification materials");

        FQuest_Objective Obj1;
        Obj1.ObjectiveID = TEXT("REPEL_ATTACKERS");
        Obj1.Description = TEXT("Repel 5 predator attacks");
        Obj1.RequiredCount = 5;
        Obj1.CurrentCount = 0;
        Obj1.bCompleted = false;
        Q.Objectives.Add(Obj1);

        RegisterQuest(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized %d default quests"), ActiveQuests.Num());
}

void AQuestManager::RegisterQuest(const FQuest_Data& Quest)
{
    ActiveQuests.Add(Quest);
}

bool AQuestManager::StartQuest(const FString& QuestID)
{
    for (FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Inactive)
        {
            Q.Status = EQuest_Status::Active;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest '%s'"), *Q.QuestName);
            return true;
        }
    }
    return false;
}

bool AQuestManager::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    for (FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Active)
        {
            for (FQuest_Objective& Obj : Q.Objectives)
            {
                if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
                {
                    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Progress, Obj.RequiredCount);
                    if (Obj.CurrentCount >= Obj.RequiredCount)
                    {
                        Obj.bCompleted = true;
                        UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective '%s' completed"), *Obj.Description);
                    }
                    CheckQuestCompletion(Q);
                    return true;
                }
            }
        }
    }
    return false;
}

bool AQuestManager::CheckQuestCompletion(FQuest_Data& Quest)
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

    if (bAllDone)
    {
        Quest.Status = EQuest_Status::Completed;
        CompletedQuests.Add(Quest);
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' COMPLETED!"), *Quest.QuestName);
        return true;
    }
    return false;
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); ++i)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].Status = EQuest_Status::Completed;
            CompletedQuests.Add(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool AQuestManager::FailQuest(const FString& QuestID)
{
    for (FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID)
        {
            Q.Status = EQuest_Status::Failed;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest '%s' FAILED"), *Q.QuestName);
            return true;
        }
    }
    return false;
}

FQuest_Data AQuestManager::GetQuestData(const FString& QuestID) const
{
    for (const FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID)
        {
            return Q;
        }
    }
    return FQuest_Data();
}

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_Data& Q : ActiveQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Active)
        {
            return true;
        }
    }
    return false;
}

void AQuestManager::OnHerdFleeTriggered(FVector FleeOrigin)
{
    // When the herd flees (from CrowdHerdManager), activate the survival quest
    StartQuest(TEXT("SURVIVE_TREX_001"));
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Herd flee at %s triggered SURVIVE_TREX_001"), *FleeOrigin.ToString());
}

int32 AQuestManager::GetActiveQuestCount() const
{
    int32 Count = 0;
    for (const FQuest_Data& Q : ActiveQuests)
    {
        if (Q.Status == EQuest_Status::Active)
        {
            Count++;
        }
    }
    return Count;
}
