#include "QuestManager.h"
#include "Engine/World.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterDefaultQuests();
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AQuestManager::RegisterDefaultQuests()
{
    AllQuests.Empty();
    InitHuntRaptorQuest();
    InitGatherResourcesQuest();
    InitDefendCampQuest();
}

void AQuestManager::InitHuntRaptorQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = "QUEST_HUNT_RAPTOR_001";
    Quest.QuestName = "Raptor Threat";
    Quest.Description = "A pack of raptors has been spotted near the river crossing. Hunt 3 raptors to secure the area.";
    Quest.QuestType = EQuest_Type::Hunt;
    Quest.Status = EQuest_Status::Inactive;
    Quest.RewardXP = 150;
    Quest.RewardItem = "Raptor_Claw";

    FQuest_Objective Obj;
    Obj.ObjectiveID = "OBJ_KILL_RAPTOR";
    Obj.Description = "Hunt raptors near the river";
    Obj.RequiredCount = 3;
    Obj.CurrentCount = 0;
    Obj.bCompleted = false;
    Quest.Objectives.Add(Obj);

    AllQuests.Add(Quest);
}

void AQuestManager::InitGatherResourcesQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = "QUEST_GATHER_AXEMAT_001";
    Quest.QuestName = "Stone Axe Materials";
    Quest.Description = "Gather 2 sharp rocks and 1 sturdy stick to craft a stone axe.";
    Quest.QuestType = EQuest_Type::Gather;
    Quest.Status = EQuest_Status::Inactive;
    Quest.RewardXP = 75;
    Quest.RewardItem = "Stone_Axe";

    FQuest_Objective ObjRock;
    ObjRock.ObjectiveID = "OBJ_GATHER_ROCK";
    ObjRock.Description = "Collect sharp rocks";
    ObjRock.RequiredCount = 2;
    ObjRock.CurrentCount = 0;
    ObjRock.bCompleted = false;
    Quest.Objectives.Add(ObjRock);

    FQuest_Objective ObjStick;
    ObjStick.ObjectiveID = "OBJ_GATHER_STICK";
    ObjStick.Description = "Collect a sturdy stick";
    ObjStick.RequiredCount = 1;
    ObjStick.CurrentCount = 0;
    ObjStick.bCompleted = false;
    Quest.Objectives.Add(ObjStick);

    AllQuests.Add(Quest);
}

void AQuestManager::InitDefendCampQuest()
{
    FQuest_Data Quest;
    Quest.QuestID = "QUEST_DEFEND_CAMP_001";
    Quest.QuestName = "Night Raid";
    Quest.Description = "Predators are approaching the camp at nightfall. Defend the camp until dawn.";
    Quest.QuestType = EQuest_Type::Defend;
    Quest.Status = EQuest_Status::Inactive;
    Quest.RewardXP = 200;
    Quest.RewardItem = "Bone_Spear";

    FQuest_Objective ObjSurvive;
    ObjSurvive.ObjectiveID = "OBJ_SURVIVE_NIGHT";
    ObjSurvive.Description = "Survive until dawn (300 seconds)";
    ObjSurvive.RequiredCount = 1;
    ObjSurvive.CurrentCount = 0;
    ObjSurvive.bCompleted = false;
    Quest.Objectives.Add(ObjSurvive);

    FQuest_Objective ObjRepel;
    ObjRepel.ObjectiveID = "OBJ_REPEL_PREDATORS";
    ObjRepel.Description = "Repel predator attacks";
    ObjRepel.RequiredCount = 5;
    ObjRepel.CurrentCount = 0;
    ObjRepel.bCompleted = false;
    Quest.Objectives.Add(ObjRepel);

    AllQuests.Add(Quest);
}

void AQuestManager::StartQuest(const FString& QuestID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Status == EQuest_Status::Inactive)
        {
            Quest.Status = EQuest_Status::Active;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest %s"), *Quest.QuestName);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found or already active"), *QuestID);
}

void AQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Status == EQuest_Status::Active)
        {
            for (FQuest_Objective& Obj : Quest.Objectives)
            {
                if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
                {
                    Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
                    if (Obj.CurrentCount >= Obj.RequiredCount)
                    {
                        Obj.bCompleted = true;
                        UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %s completed"), *ObjectiveID);
                    }
                    CheckQuestCompletion(Quest);
                    return;
                }
            }
        }
    }
}

void AQuestManager::CheckQuestCompletion(FQuest_Data& Quest)
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
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest %s COMPLETED! Reward: %d XP + %s"),
            *Quest.QuestName, Quest.RewardXP, *Quest.RewardItem);
    }
}

void AQuestManager::FailQuest(const FString& QuestID)
{
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Status == EQuest_Status::Active)
        {
            Quest.Status = EQuest_Status::Failed;
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s FAILED"), *Quest.QuestName);
            return;
        }
    }
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
    TArray<FQuest_Data> Active;
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.Status == EQuest_Status::Active)
        {
            Active.Add(Quest);
        }
    }
    return Active;
}

bool AQuestManager::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.Status == EQuest_Status::Active;
        }
    }
    return false;
}
