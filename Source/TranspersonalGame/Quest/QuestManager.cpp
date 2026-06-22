// QuestManager.cpp
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260622_005

#include "QuestManager.h"

UQuestManager::UQuestManager()
{
    // CDO safe — no world access here
}

void UQuestManager::InitializeQuestSystem()
{
    AllQuests.Empty();
    RegisterStarterQuests();
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized with %d quests"), AllQuests.Num());
}

void UQuestManager::RegisterStarterQuests()
{
    AllQuests.Add(BuildHuntQuest());
    AllQuests.Add(BuildGatherQuest());
    AllQuests.Add(BuildDefendQuest());
    AllQuests.Add(BuildTrackHerdQuest());
    AllQuests.Add(BuildCraftAxeQuest());
}

FQuest_Data UQuestManager::BuildHuntQuest()
{
    FQuest_Data Q;
    Q.QuestID = FName("QUEST_HUNT_01");
    Q.QuestTitle = TEXT("First Blood");
    Q.QuestDescription = TEXT("The tribe needs food. Hunt a small dinosaur near the river and bring back the meat before nightfall.");
    Q.QuestType = EQuest_Type::Hunt;
    Q.Status = EQuest_Status::Inactive;
    Q.GiverNPCID = FName("NPC_ELDER_01");
    Q.bIsMainQuest = true;
    Q.TimeLimit = 0.0f;

    FQuest_Objective Obj1;
    Obj1.ObjectiveType = EQuest_ObjectiveType::KillTarget;
    Obj1.Description = TEXT("Kill 1 Gallimimus near the river");
    Obj1.RequiredCount = 1;
    Obj1.TargetLocation = FVector(400.0f, 200.0f, 50.0f);
    Obj1.LocationRadius = 500.0f;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    Obj2.Description = TEXT("Collect 2 pieces of raw meat");
    Obj2.RequiredCount = 2;
    Q.Objectives.Add(Obj2);

    Q.Reward.FoodReward = 5;
    Q.Reward.MaterialReward = 1;
    Q.Reward.UnlockedRecipe = TEXT("Cooked Meat");
    return Q;
}

FQuest_Data UQuestManager::BuildGatherQuest()
{
    FQuest_Data Q;
    Q.QuestID = FName("QUEST_GATHER_01");
    Q.QuestTitle = TEXT("Roots and Stones");
    Q.QuestDescription = TEXT("Gather flint stones and plant fibres from the forest clearing. These are needed to craft basic tools.");
    Q.QuestType = EQuest_Type::Gather;
    Q.Status = EQuest_Status::Inactive;
    Q.GiverNPCID = FName("NPC_CRAFTER_01");
    Q.bIsMainQuest = false;
    Q.TimeLimit = 0.0f;

    FQuest_Objective Obj1;
    Obj1.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    Obj1.Description = TEXT("Collect 3 flint stones");
    Obj1.RequiredCount = 3;
    Obj1.TargetLocation = FVector(-300.0f, 400.0f, 50.0f);
    Obj1.LocationRadius = 400.0f;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    Obj2.Description = TEXT("Collect 2 plant fibres");
    Obj2.RequiredCount = 2;
    Q.Objectives.Add(Obj2);

    Q.Reward.MaterialReward = 3;
    Q.Reward.UnlockedRecipe = TEXT("Stone Knife");
    return Q;
}

FQuest_Data UQuestManager::BuildDefendQuest()
{
    FQuest_Data Q;
    Q.QuestID = FName("QUEST_DEFEND_01");
    Q.QuestTitle = TEXT("Night Watch");
    Q.QuestDescription = TEXT("A pack of raptors was spotted circling the camp. Defend the camp perimeter until dawn.");
    Q.QuestType = EQuest_Type::Defend;
    Q.Status = EQuest_Status::Inactive;
    Q.GiverNPCID = FName("NPC_SCOUT_01");
    Q.bIsMainQuest = false;
    Q.TimeLimit = 180.0f;  // 3 minutes

    FQuest_Objective Obj1;
    Obj1.ObjectiveType = EQuest_ObjectiveType::SurviveTime;
    Obj1.Description = TEXT("Survive until dawn (3 minutes)");
    Obj1.RequiredCount = 1;
    Obj1.TargetLocation = FVector(100.0f, -500.0f, 50.0f);
    Obj1.LocationRadius = 600.0f;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveType = EQuest_ObjectiveType::KillTarget;
    Obj2.Description = TEXT("Repel 3 raptor attacks");
    Obj2.RequiredCount = 3;
    Q.Objectives.Add(Obj2);

    Q.Reward.FoodReward = 2;
    Q.Reward.MaterialReward = 2;
    Q.Reward.UnlockedArea = TEXT("Eastern Forest");
    return Q;
}

FQuest_Data UQuestManager::BuildTrackHerdQuest()
{
    FQuest_Data Q;
    Q.QuestID = FName("QUEST_TRACK_01");
    Q.QuestTitle = TEXT("Follow the Herd");
    Q.QuestDescription = TEXT("The Parasaurolophus herd migrates south each season. Follow them to discover the southern valley — a new territory rich in resources.");
    Q.QuestType = EQuest_Type::Track;
    Q.Status = EQuest_Status::Inactive;
    Q.GiverNPCID = FName("NPC_ELDER_01");
    Q.bIsMainQuest = true;
    Q.TimeLimit = 0.0f;

    FQuest_Objective Obj1;
    Obj1.ObjectiveType = EQuest_ObjectiveType::ObserveEvent;
    Obj1.Description = TEXT("Observe the herd migration start");
    Obj1.RequiredCount = 1;
    Obj1.TargetLocation = FVector(700.0f, 100.0f, 80.0f);
    Obj1.LocationRadius = 300.0f;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveType = EQuest_ObjectiveType::ReachLocation;
    Obj2.Description = TEXT("Follow the herd to the southern valley");
    Obj2.RequiredCount = 1;
    Obj2.TargetLocation = FVector(900.0f, -200.0f, 50.0f);
    Obj2.LocationRadius = 400.0f;
    Q.Objectives.Add(Obj2);

    Q.Reward.FoodReward = 3;
    Q.Reward.UnlockedArea = TEXT("Southern Valley");
    return Q;
}

FQuest_Data UQuestManager::BuildCraftAxeQuest()
{
    FQuest_Data Q;
    Q.QuestID = FName("QUEST_CRAFT_01");
    Q.QuestTitle = TEXT("Stone and Sinew");
    Q.QuestDescription = TEXT("A stone axe will let you chop wood and fight larger prey. Gather the materials and craft one at the workbench.");
    Q.QuestType = EQuest_Type::Craft;
    Q.Status = EQuest_Status::Inactive;
    Q.GiverNPCID = FName("NPC_CRAFTER_01");
    Q.bIsMainQuest = false;
    Q.TimeLimit = 0.0f;

    FQuest_Objective Obj1;
    Obj1.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    Obj1.Description = TEXT("Collect 2 sharp rocks");
    Obj1.RequiredCount = 2;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveType = EQuest_ObjectiveType::CollectItem;
    Obj2.Description = TEXT("Collect 1 sturdy stick");
    Obj2.RequiredCount = 1;
    Q.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveType = EQuest_ObjectiveType::CraftItem;
    Obj3.Description = TEXT("Craft a Stone Axe at the workbench");
    Obj3.RequiredCount = 1;
    Obj3.TargetLocation = FVector(-50.0f, 150.0f, 50.0f);
    Obj3.LocationRadius = 200.0f;
    Q.Objectives.Add(Obj3);

    Q.Reward.MaterialReward = 2;
    Q.Reward.UnlockedRecipe = TEXT("Stone Spear");
    return Q;
}

bool UQuestManager::ActivateQuest(FName QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;
    if (AllQuests[Idx].Status != EQuest_Status::Inactive) return false;

    AllQuests[Idx].Status = EQuest_Status::Active;
    AllQuests[Idx].ElapsedTime = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Activated quest '%s'"), *AllQuests[Idx].QuestTitle);
    return true;
}

void UQuestManager::UpdateObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Amount)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return;
    if (AllQuests[Idx].Status != EQuest_Status::Active) return;
    if (!AllQuests[Idx].Objectives.IsValidIndex(ObjectiveIndex)) return;

    FQuest_Objective& Obj = AllQuests[Idx].Objectives[ObjectiveIndex];
    if (Obj.bCompleted) return;

    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Amount, Obj.RequiredCount);
    if (Obj.CurrentCount >= Obj.RequiredCount)
    {
        Obj.bCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %d completed for quest '%s'"),
               ObjectiveIndex, *AllQuests[Idx].QuestTitle);
    }

    CheckQuestCompletion(QuestID);
}

bool UQuestManager::CheckQuestCompletion(FName QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;
    if (AllQuests[Idx].Status != EQuest_Status::Active) return false;

    bool bAllDone = true;
    for (const FQuest_Objective& Obj : AllQuests[Idx].Objectives)
    {
        if (!Obj.bCompleted) { bAllDone = false; break; }
    }

    if (bAllDone)
    {
        AllQuests[Idx].Status = EQuest_Status::Completed;
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — '%s'"), *AllQuests[Idx].QuestTitle);
        return true;
    }
    return false;
}

void UQuestManager::FailQuest(FName QuestID)
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return;
    AllQuests[Idx].Status = EQuest_Status::Failed;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest FAILED — '%s'"), *AllQuests[Idx].QuestTitle);
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Active;
    for (const FQuest_Data& Q : AllQuests)
    {
        if (Q.Status == EQuest_Status::Active) Active.Add(Q);
    }
    return Active;
}

bool UQuestManager::GetQuestData(FName QuestID, FQuest_Data& OutData) const
{
    int32 Idx = FindQuestIndex(QuestID);
    if (Idx == INDEX_NONE) return false;
    OutData = AllQuests[Idx];
    return true;
}

void UQuestManager::TickQuestTimers(float DeltaTime)
{
    for (FQuest_Data& Q : AllQuests)
    {
        if (Q.Status != EQuest_Status::Active) continue;
        if (Q.TimeLimit <= 0.0f) continue;

        Q.ElapsedTime += DeltaTime;
        if (Q.ElapsedTime >= Q.TimeLimit)
        {
            FailQuest(Q.QuestID);
        }
    }
}

void UQuestManager::OnHerdFleeTriggered(FVector FleeOrigin, float Radius)
{
    // Check if any active quest has an ObserveEvent objective near the flee origin
    for (FQuest_Data& Q : AllQuests)
    {
        if (Q.Status != EQuest_Status::Active) continue;
        for (int32 i = 0; i < Q.Objectives.Num(); ++i)
        {
            FQuest_Objective& Obj = Q.Objectives[i];
            if (Obj.ObjectiveType != EQuest_ObjectiveType::ObserveEvent) continue;
            if (Obj.bCompleted) continue;

            float Dist = FVector::Dist(FleeOrigin, Obj.TargetLocation);
            if (Dist <= (Radius + Obj.LocationRadius))
            {
                UpdateObjectiveProgress(Q.QuestID, i, 1);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Herd flee triggered objective '%s'"), *Obj.Description);
            }
        }
    }
}

int32 UQuestManager::GetCompletedQuestCount() const
{
    int32 Count = 0;
    for (const FQuest_Data& Q : AllQuests)
    {
        if (Q.Status == EQuest_Status::Completed) ++Count;
    }
    return Count;
}

int32 UQuestManager::FindQuestIndex(FName QuestID) const
{
    for (int32 i = 0; i < AllQuests.Num(); ++i)
    {
        if (AllQuests[i].QuestID == QuestID) return i;
    }
    return INDEX_NONE;
}
