#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check quest state every second
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeQuestRegistry();
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Future: proximity checks against QuestZoneLocation
}

void AQuestManager::InitializeQuestRegistry()
{
    AllQuests.Empty();
    AllQuests.Add(BuildHuntRexQuest());
    AllQuests.Add(BuildGatherResourcesQuest());
    AllQuests.Add(BuildDefendCampQuest());
    AllQuests.Add(BuildTrackRaptorsQuest());
    AllQuests.Add(BuildWaterSourceQuest());

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered %d quests"), AllQuests.Num());
}

FQuest_Definition AQuestManager::BuildHuntRexQuest() const
{
    FQuest_Definition Q;
    Q.QuestID = TEXT("QUEST_HUNT_REX");
    Q.Title = TEXT("Hunt the Rex");
    Q.Description = TEXT("A Tyrannosaurus Rex has been spotted near the eastern ridge. It has already killed two hunters. Track it down and eliminate the threat before it reaches the camp.");
    Q.QuestType = EQuest_Type::Hunt;
    Q.Status = EQuest_Status::Inactive;
    Q.QuestZoneLocation = FVector(1200.f, 800.f, 100.f);
    Q.QuestZoneRadius = 1500.f;
    Q.RewardDescription = TEXT("Rex hide (crafting material), 3 large bones, camp safety restored");

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("FIND_REX_TRACKS");
    Obj1.Description = TEXT("Find the T-Rex tracks near the river");
    Obj1.RequiredCount = 1;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("CRAFT_SPEAR");
    Obj2.Description = TEXT("Craft a stone spear before engaging");
    Obj2.RequiredCount = 1;
    Q.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("KILL_REX");
    Obj3.Description = TEXT("Kill the Tyrannosaurus Rex");
    Obj3.RequiredCount = 1;
    Q.Objectives.Add(Obj3);

    return Q;
}

FQuest_Definition AQuestManager::BuildGatherResourcesQuest() const
{
    FQuest_Definition Q;
    Q.QuestID = TEXT("QUEST_GATHER_RESOURCES");
    Q.Title = TEXT("Gather Flint and Bone");
    Q.Description = TEXT("The tribe needs materials to craft better tools. Collect flint stones from the eastern ridge and bones from the raptor kill site to the north.");
    Q.QuestType = EQuest_Type::Gather;
    Q.Status = EQuest_Status::Inactive;
    Q.QuestZoneLocation = FVector(-600.f, 1400.f, 100.f);
    Q.QuestZoneRadius = 800.f;
    Q.RewardDescription = TEXT("Crafted stone axe, 2 bone needles");

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("COLLECT_FLINT");
    Obj1.Description = TEXT("Collect flint stones (0/5)");
    Obj1.RequiredCount = 5;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("COLLECT_BONES");
    Obj2.Description = TEXT("Collect animal bones (0/3)");
    Obj2.RequiredCount = 3;
    Q.Objectives.Add(Obj2);

    return Q;
}

FQuest_Definition AQuestManager::BuildDefendCampQuest() const
{
    FQuest_Definition Q;
    Q.QuestID = TEXT("QUEST_DEFEND_CAMP");
    Q.Title = TEXT("Defend the Camp");
    Q.Description = TEXT("A pack of raptors has been circling the camp perimeter. Set traps and drive them away before nightfall or the tribe will be forced to abandon this location.");
    Q.QuestType = EQuest_Type::Defend;
    Q.Status = EQuest_Status::Inactive;
    Q.QuestZoneLocation = FVector(0.f, 0.f, 100.f);
    Q.QuestZoneRadius = 600.f;
    Q.RewardDescription = TEXT("Camp upgraded to tier 2, 5 wooden stakes");

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("SET_TRAPS");
    Obj1.Description = TEXT("Set spike traps around the camp perimeter (0/4)");
    Obj1.RequiredCount = 4;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("REPEL_RAPTORS");
    Obj2.Description = TEXT("Repel the raptor attack wave");
    Obj2.RequiredCount = 1;
    Q.Objectives.Add(Obj2);

    return Q;
}

FQuest_Definition AQuestManager::BuildTrackRaptorsQuest() const
{
    FQuest_Definition Q;
    Q.QuestID = TEXT("QUEST_TRACK_RAPTORS");
    Q.Title = TEXT("Track the Raptor Pack");
    Q.Description = TEXT("Two tribe members are missing. Follow the raptor tracks south and determine where the pack has made its den. Do not engage — just locate and report back.");
    Q.QuestType = EQuest_Type::Track;
    Q.Status = EQuest_Status::Inactive;
    Q.QuestZoneLocation = FVector(500.f, -900.f, 100.f);
    Q.QuestZoneRadius = 1200.f;
    Q.RewardDescription = TEXT("Raptor den location revealed on map, 2 raptor claws");

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("FOLLOW_TRACKS");
    Obj1.Description = TEXT("Follow the blood trail south");
    Obj1.RequiredCount = 3;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("FIND_DEN");
    Obj2.Description = TEXT("Locate the raptor den");
    Obj2.RequiredCount = 1;
    Q.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("RETURN_ALIVE");
    Obj3.Description = TEXT("Return to camp without being detected");
    Obj3.RequiredCount = 1;
    Q.Objectives.Add(Obj3);

    return Q;
}

FQuest_Definition AQuestManager::BuildWaterSourceQuest() const
{
    FQuest_Definition Q;
    Q.QuestID = TEXT("QUEST_WATER_SOURCE");
    Q.Title = TEXT("Find Clean Water");
    Q.Description = TEXT("The river near camp has been fouled by a dead carcass upstream. Find a clean water source before the tribe's water supply runs out in two days.");
    Q.QuestType = EQuest_Type::Explore;
    Q.Status = EQuest_Status::Inactive;
    Q.QuestZoneLocation = FVector(-1200.f, -400.f, 100.f);
    Q.QuestZoneRadius = 1000.f;
    Q.RewardDescription = TEXT("New camp water source discovered, clay water vessel crafted");

    FQuest_Objective Obj1;
    Obj1.ObjectiveID = TEXT("EXPLORE_WEST");
    Obj1.Description = TEXT("Explore the western valley");
    Obj1.RequiredCount = 1;
    Q.Objectives.Add(Obj1);

    FQuest_Objective Obj2;
    Obj2.ObjectiveID = TEXT("FIND_SPRING");
    Obj2.Description = TEXT("Find a fresh water spring");
    Obj2.RequiredCount = 1;
    Q.Objectives.Add(Obj2);

    FQuest_Objective Obj3;
    Obj3.ObjectiveID = TEXT("COLLECT_WATER");
    Obj3.Description = TEXT("Collect water in a clay vessel");
    Obj3.RequiredCount = 1;
    Q.Objectives.Add(Obj3);

    return Q;
}

bool AQuestManager::ActivateQuest(const FString& QuestID)
{
    for (FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Inactive)
        {
            Q.Status = EQuest_Status::Active;
            ActiveQuestIDs.AddUnique(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest ACTIVATED — %s"), *Q.Title);
            return true;
        }
    }
    return false;
}

bool AQuestManager::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count)
{
    for (FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Active)
        {
            for (FQuest_Objective& Obj : Q.Objectives)
            {
                if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
                {
                    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
                    if (Obj.CurrentCount >= Obj.RequiredCount)
                    {
                        Obj.bCompleted = true;
                        UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective COMPLETE — %s"), *Obj.Description);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    for (FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID && Q.Status == EQuest_Status::Active)
        {
            // Check all objectives done
            bool bAllDone = true;
            for (const FQuest_Objective& Obj : Q.Objectives)
            {
                if (!Obj.bCompleted) { bAllDone = false; break; }
            }
            if (bAllDone)
            {
                Q.Status = EQuest_Status::Completed;
                ActiveQuestIDs.Remove(QuestID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s | Reward: %s"), *Q.Title, *Q.RewardDescription);
                return true;
            }
        }
    }
    return false;
}

FQuest_Definition AQuestManager::GetQuestByID(const FString& QuestID) const
{
    for (const FQuest_Definition& Q : AllQuests)
    {
        if (Q.QuestID == QuestID) return Q;
    }
    return FQuest_Definition();
}

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

TArray<FQuest_Definition> AQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Definition> Active;
    for (const FQuest_Definition& Q : AllQuests)
    {
        if (Q.Status == EQuest_Status::Active)
        {
            Active.Add(Q);
        }
    }
    return Active;
}

void AQuestManager::RegisterDefaultQuests()
{
    InitializeQuestRegistry();
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Default quests registered via editor call — %d quests"), AllQuests.Num());
}
