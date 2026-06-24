// QuestManager.cpp — Agent #14 Quest & Mission Designer
// Implements the core quest management system for the prehistoric survival game.
// Manages quest state, objectives, rewards, and progression.

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UQuestManager::UQuestManager()
{
    ActiveQuestCount = 0;
    CompletedQuestCount = 0;
    bQuestSystemInitialized = false;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeQuestDatabase();
    bQuestSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized with %d quests in database"), QuestDatabase.Num());
}

void UQuestManager::Deinitialize()
{
    ActiveQuests.Empty();
    QuestDatabase.Empty();
    Super::Deinitialize();
}

void UQuestManager::InitializeQuestDatabase()
{
    // Quest 1: Hunt the Raptor Pack
    FQuest_Data HuntRaptors;
    HuntRaptors.QuestID = FName("QUEST_HUNT_RAPTORS");
    HuntRaptors.QuestTitle = FText::FromString("Hunt the Raptor Pack");
    HuntRaptors.QuestDescription = FText::FromString(
        "A pack of velociraptors has been stalking the eastern riverbank. "
        "Track them down and eliminate the threat before they attack the camp."
    );
    HuntRaptors.QuestType = EQuest_Type::Hunt;
    HuntRaptors.QuestState = EQuest_State::Available;
    HuntRaptors.bIsMainQuest = false;
    HuntRaptors.RewardBones = 15;
    HuntRaptors.RewardHide = 8;

    FQuest_Objective KillRaptors;
    KillRaptors.ObjectiveID = FName("OBJ_KILL_3_RAPTORS");
    KillRaptors.Description = FText::FromString("Eliminate 3 velociraptors near the eastern river");
    KillRaptors.TargetCount = 3;
    KillRaptors.CurrentCount = 0;
    KillRaptors.bIsCompleted = false;
    KillRaptors.ObjectiveType = EQuest_ObjectiveType::Kill;
    HuntRaptors.Objectives.Add(KillRaptors);

    QuestDatabase.Add(HuntRaptors.QuestID, HuntRaptors);

    // Quest 2: Gather Campfire Materials
    FQuest_Data GatherMaterials;
    GatherMaterials.QuestID = FName("QUEST_GATHER_CAMPFIRE");
    GatherMaterials.QuestTitle = FText::FromString("Build a Campfire");
    GatherMaterials.QuestDescription = FText::FromString(
        "Night is approaching and temperatures are dropping. "
        "Gather 3 dry sticks from the forest to build a campfire and survive the cold night."
    );
    GatherMaterials.QuestType = EQuest_Type::Gather;
    GatherMaterials.QuestState = EQuest_State::Available;
    GatherMaterials.bIsMainQuest = true;
    GatherMaterials.RewardBones = 0;
    GatherMaterials.RewardHide = 0;

    FQuest_Objective CollectSticks;
    CollectSticks.ObjectiveID = FName("OBJ_COLLECT_3_STICKS");
    CollectSticks.Description = FText::FromString("Collect 3 dry sticks from the forest");
    CollectSticks.TargetCount = 3;
    CollectSticks.CurrentCount = 0;
    CollectSticks.bIsCompleted = false;
    CollectSticks.ObjectiveType = EQuest_ObjectiveType::Collect;
    GatherMaterials.Objectives.Add(CollectSticks);

    FQuest_Objective BuildCampfire;
    BuildCampfire.ObjectiveID = FName("OBJ_BUILD_CAMPFIRE");
    BuildCampfire.Description = FText::FromString("Use the crafting system to build a campfire");
    BuildCampfire.TargetCount = 1;
    BuildCampfire.CurrentCount = 0;
    BuildCampfire.bIsCompleted = false;
    BuildCampfire.ObjectiveType = EQuest_ObjectiveType::Craft;
    GatherMaterials.Objectives.Add(BuildCampfire);

    QuestDatabase.Add(GatherMaterials.QuestID, GatherMaterials);

    // Quest 3: Defend the Camp
    FQuest_Data DefendCamp;
    DefendCamp.QuestID = FName("QUEST_DEFEND_CAMP");
    DefendCamp.QuestTitle = FText::FromString("Defend the Camp");
    DefendCamp.QuestDescription = FText::FromString(
        "A T-Rex has been drawn by the smell of your campfire. "
        "Survive the attack by driving it away — you cannot kill it alone. Use fire and noise."
    );
    DefendCamp.QuestType = EQuest_Type::Defend;
    DefendCamp.QuestState = EQuest_State::Locked;
    DefendCamp.bIsMainQuest = true;
    DefendCamp.RewardBones = 30;
    DefendCamp.RewardHide = 20;

    FQuest_Objective SurviveTRex;
    SurviveTRex.ObjectiveID = FName("OBJ_SURVIVE_TREX");
    SurviveTRex.Description = FText::FromString("Survive the T-Rex attack for 3 minutes");
    SurviveTRex.TargetCount = 180; // seconds
    SurviveTRex.CurrentCount = 0;
    SurviveTRex.bIsCompleted = false;
    SurviveTRex.ObjectiveType = EQuest_ObjectiveType::Survive;
    DefendCamp.Objectives.Add(SurviveTRex);

    FQuest_Objective DriveTRex;
    DriveTRex.ObjectiveID = FName("OBJ_DRIVE_TREX");
    DriveTRex.Description = FText::FromString("Drive the T-Rex away from camp using fire");
    DriveTRex.TargetCount = 1;
    DriveTRex.CurrentCount = 0;
    DriveTRex.bIsCompleted = false;
    DriveTRex.ObjectiveType = EQuest_ObjectiveType::Interact;
    DefendCamp.Objectives.Add(DriveTRex);

    QuestDatabase.Add(DefendCamp.QuestID, DefendCamp);

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Database initialized with %d quests"), QuestDatabase.Num());
}

bool UQuestManager::StartQuest(FName QuestID)
{
    if (!QuestDatabase.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found in database"), *QuestID.ToString());
        return false;
    }

    FQuest_Data& Quest = QuestDatabase[QuestID];

    if (Quest.QuestState == EQuest_State::Locked)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is locked"), *QuestID.ToString());
        return false;
    }

    if (Quest.QuestState == EQuest_State::Active || Quest.QuestState == EQuest_State::Completed)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already active or completed"), *QuestID.ToString());
        return false;
    }

    Quest.QuestState = EQuest_State::Active;
    ActiveQuests.Add(QuestID, Quest);
    ActiveQuestCount = ActiveQuests.Num();

    OnQuestStarted.Broadcast(QuestID, Quest.QuestTitle);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest STARTED — %s"), *Quest.QuestTitle.ToString());
    return true;
}

bool UQuestManager::UpdateObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_Data& Quest = ActiveQuests[QuestID];

    for (FQuest_Objective& Obj : Quest.Objectives)
    {
        if (Obj.ObjectiveID == ObjectiveID)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + ProgressAmount, Obj.TargetCount);

            if (Obj.CurrentCount >= Obj.TargetCount && !Obj.bIsCompleted)
            {
                Obj.bIsCompleted = true;
                OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective COMPLETED — %s in quest %s"),
                    *ObjectiveID.ToString(), *QuestID.ToString());
            }

            // Check if all objectives are complete
            CheckQuestCompletion(QuestID);
            return true;
        }
    }

    return false;
}

void UQuestManager::CheckQuestCompletion(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID)) return;

    FQuest_Data& Quest = ActiveQuests[QuestID];
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
        CompleteQuest(QuestID);
    }
}

bool UQuestManager::CompleteQuest(FName QuestID)
{
    if (!ActiveQuests.Contains(QuestID)) return false;

    FQuest_Data& Quest = ActiveQuests[QuestID];
    Quest.QuestState = EQuest_State::Completed;

    // Update database
    if (QuestDatabase.Contains(QuestID))
    {
        QuestDatabase[QuestID].QuestState = EQuest_State::Completed;
    }

    OnQuestCompleted.Broadcast(QuestID, Quest.RewardBones, Quest.RewardHide);
    ActiveQuests.Remove(QuestID);
    ActiveQuestCount = ActiveQuests.Num();
    CompletedQuestCount++;

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s | Reward: %d bones, %d hide"),
        *Quest.QuestTitle.ToString(), Quest.RewardBones, Quest.RewardHide);

    // Unlock follow-up quests
    UnlockFollowUpQuests(QuestID);
    return true;
}

void UQuestManager::UnlockFollowUpQuests(FName CompletedQuestID)
{
    // Completing GATHER unlocks DEFEND
    if (CompletedQuestID == FName("QUEST_GATHER_CAMPFIRE"))
    {
        if (QuestDatabase.Contains(FName("QUEST_DEFEND_CAMP")))
        {
            QuestDatabase[FName("QUEST_DEFEND_CAMP")].QuestState = EQuest_State::Available;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: QUEST_DEFEND_CAMP unlocked"));
        }
    }
}

TArray<FQuest_Data> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : ActiveQuests)
    {
        Result.Add(Pair.Value);
    }
    return Result;
}

TArray<FQuest_Data> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_Data> Result;
    for (const auto& Pair : QuestDatabase)
    {
        if (Pair.Value.QuestState == EQuest_State::Available)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

FQuest_Data UQuestManager::GetQuestData(FName QuestID) const
{
    if (QuestDatabase.Contains(QuestID))
    {
        return QuestDatabase[QuestID];
    }
    return FQuest_Data();
}
