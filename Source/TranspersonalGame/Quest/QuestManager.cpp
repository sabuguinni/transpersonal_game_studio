// QuestManager.cpp — Quest & Mission Designer Agent #14
// Implements the core quest system for Transpersonal Game (Prehistoric Survival)

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UQuestManager::UQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    ActiveQuestIndex = -1;
    bQuestSystemInitialized = false;
}

void UQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeQuestDatabase();
    bQuestSystemInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized with %d quests"), QuestDatabase.Num());
}

void UQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bQuestSystemInitialized) return;

    // Update active quest objective timers
    for (FQuest_ObjectiveData& Obj : ActiveObjectives)
    {
        if (Obj.ObjectiveState == EQuest_ObjectiveState::InProgress && Obj.TimeLimit > 0.0f)
        {
            Obj.TimeLimit -= DeltaTime;
            if (Obj.TimeLimit <= 0.0f)
            {
                Obj.ObjectiveState = EQuest_ObjectiveState::Failed;
                OnObjectiveFailed(Obj.ObjectiveID);
            }
        }
    }
}

void UQuestManager::InitializeQuestDatabase()
{
    QuestDatabase.Empty();

    // --- QUEST 1: Protect the Camp ---
    {
        FQuest_QuestData Q;
        Q.QuestID = FName("Q_ProtectCamp");
        Q.QuestTitle = FText::FromString("Protect the Camp");
        Q.QuestDescription = FText::FromString("A T-Rex has been spotted approaching the tribe's camp. Drive it away before it destroys your shelter and harms your people.");
        Q.QuestType = EQuest_QuestType::Defense;
        Q.QuestState = EQuest_QuestState::Available;
        Q.bIsMainQuest = false;
        Q.RewardBones = 15;
        Q.RewardHides = 5;

        FQuest_ObjectiveData Obj1;
        Obj1.ObjectiveID = FName("OBJ_SpotTRex");
        Obj1.Description = FText::FromString("Spot the approaching T-Rex");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.TargetCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj1.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj1);

        FQuest_ObjectiveData Obj2;
        Obj2.ObjectiveID = FName("OBJ_DriveTRex");
        Obj2.Description = FText::FromString("Drive the T-Rex away from camp (deal 200 damage or lure it 500m away)");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Kill;
        Obj2.TargetCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj2.TimeLimit = 120.0f; // 2 minutes
        Q.Objectives.Add(Obj2);

        FQuest_ObjectiveData Obj3;
        Obj3.ObjectiveID = FName("OBJ_ReturnToElder");
        Obj3.Description = FText::FromString("Return to the Elder and report");
        Obj3.ObjectiveType = EQuest_ObjectiveType::TalkToNPC;
        Obj3.TargetCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj3.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj3);

        QuestDatabase.Add(Q);
    }

    // --- QUEST 2: The First Hunt ---
    {
        FQuest_QuestData Q;
        Q.QuestID = FName("Q_FirstHunt");
        Q.QuestTitle = FText::FromString("The First Hunt");
        Q.QuestDescription = FText::FromString("The tribe is running low on food. Track down a herd of herbivores and bring back enough meat to last the week.");
        Q.QuestType = EQuest_QuestType::Hunt;
        Q.QuestState = EQuest_QuestState::Available;
        Q.bIsMainQuest = true;
        Q.RewardBones = 8;
        Q.RewardHides = 12;

        FQuest_ObjectiveData Obj1;
        Obj1.ObjectiveID = FName("OBJ_FindHerd");
        Obj1.Description = FText::FromString("Find the herbivore herd (follow the tracks east)");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.TargetCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj1.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj1);

        FQuest_ObjectiveData Obj2;
        Obj2.ObjectiveID = FName("OBJ_HuntDinos");
        Obj2.Description = FText::FromString("Hunt 3 herbivores");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Kill;
        Obj2.TargetCount = 3;
        Obj2.CurrentCount = 0;
        Obj2.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj2.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj2);

        FQuest_ObjectiveData Obj3;
        Obj3.ObjectiveID = FName("OBJ_ReturnMeat");
        Obj3.Description = FText::FromString("Return the meat to camp before it spoils");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Deliver;
        Obj3.TargetCount = 3;
        Obj3.CurrentCount = 0;
        Obj3.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj3.TimeLimit = 300.0f; // 5 minutes real time
        Q.Objectives.Add(Obj3);

        QuestDatabase.Add(Q);
    }

    // --- QUEST 3: Stone Tools ---
    {
        FQuest_QuestData Q;
        Q.QuestID = FName("Q_StoneTools");
        Q.QuestTitle = FText::FromString("Sharper Than Claws");
        Q.QuestDescription = FText::FromString("Craft a stone axe and a spear. Without proper tools, survival in this world is impossible.");
        Q.QuestType = EQuest_QuestType::Crafting;
        Q.QuestState = EQuest_QuestState::Available;
        Q.bIsMainQuest = true;
        Q.RewardBones = 0;
        Q.RewardHides = 0;

        FQuest_ObjectiveData Obj1;
        Obj1.ObjectiveID = FName("OBJ_GatherStones");
        Obj1.Description = FText::FromString("Gather 4 sharp stones from the riverbed");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj1.TargetCount = 4;
        Obj1.CurrentCount = 0;
        Obj1.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj1.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj1);

        FQuest_ObjectiveData Obj2;
        Obj2.ObjectiveID = FName("OBJ_GatherSticks");
        Obj2.Description = FText::FromString("Gather 3 sturdy branches");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Gather;
        Obj2.TargetCount = 3;
        Obj2.CurrentCount = 0;
        Obj2.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj2.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj2);

        FQuest_ObjectiveData Obj3;
        Obj3.ObjectiveID = FName("OBJ_CraftAxe");
        Obj3.Description = FText::FromString("Craft a Stone Axe at the crafting stone");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj3.TargetCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj3.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj3);

        FQuest_ObjectiveData Obj4;
        Obj4.ObjectiveID = FName("OBJ_CraftSpear");
        Obj4.Description = FText::FromString("Craft a Wooden Spear at the crafting stone");
        Obj4.ObjectiveType = EQuest_ObjectiveType::Craft;
        Obj4.TargetCount = 1;
        Obj4.CurrentCount = 0;
        Obj4.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj4.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj4);

        QuestDatabase.Add(Q);
    }

    // --- QUEST 4: Scouting the Territory ---
    {
        FQuest_QuestData Q;
        Q.QuestID = FName("Q_ScoutTerritory");
        Q.QuestTitle = FText::FromString("Know Your Ground");
        Q.QuestDescription = FText::FromString("The Scout needs to map the surrounding territory. Reach 4 high vantage points to survey the land and identify safe routes.");
        Q.QuestType = EQuest_QuestType::Exploration;
        Q.QuestState = EQuest_QuestState::Available;
        Q.bIsMainQuest = false;
        Q.RewardBones = 5;
        Q.RewardHides = 3;

        FQuest_ObjectiveData Obj1;
        Obj1.ObjectiveID = FName("OBJ_VantageNorth");
        Obj1.Description = FText::FromString("Reach the northern ridge");
        Obj1.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj1.TargetCount = 1;
        Obj1.CurrentCount = 0;
        Obj1.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj1.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj1);

        FQuest_ObjectiveData Obj2;
        Obj2.ObjectiveID = FName("OBJ_VantageEast");
        Obj2.Description = FText::FromString("Reach the eastern cliffs");
        Obj2.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj2.TargetCount = 1;
        Obj2.CurrentCount = 0;
        Obj2.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj2.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj2);

        FQuest_ObjectiveData Obj3;
        Obj3.ObjectiveID = FName("OBJ_VantageSouth");
        Obj3.Description = FText::FromString("Reach the southern plateau");
        Obj3.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj3.TargetCount = 1;
        Obj3.CurrentCount = 0;
        Obj3.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj3.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj3);

        FQuest_ObjectiveData Obj4;
        Obj4.ObjectiveID = FName("OBJ_VantageWest");
        Obj4.Description = FText::FromString("Reach the western river crossing");
        Obj4.ObjectiveType = EQuest_ObjectiveType::Reach;
        Obj4.TargetCount = 1;
        Obj4.CurrentCount = 0;
        Obj4.ObjectiveState = EQuest_ObjectiveState::NotStarted;
        Obj4.TimeLimit = -1.0f;
        Q.Objectives.Add(Obj4);

        QuestDatabase.Add(Q);
    }

    UE_LOG(LogTemp, Log, TEXT("QuestManager: Database initialized with %d quests"), QuestDatabase.Num());
}

bool UQuestManager::StartQuest(FName QuestID)
{
    for (FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EQuest_QuestState::Available)
        {
            Quest.QuestState = EQuest_QuestState::Active;
            ActiveQuestIndex = QuestDatabase.IndexOfByKey(Quest);

            // Activate first objective
            if (Quest.Objectives.Num() > 0)
            {
                Quest.Objectives[0].ObjectiveState = EQuest_ObjectiveState::InProgress;
                ActiveObjectives.Add(Quest.Objectives[0]);
            }

            OnQuestStarted.Broadcast(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest started — %s"), *QuestID.ToString());
            return true;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest %s — not found or not available"), *QuestID.ToString());
    return false;
}

bool UQuestManager::UpdateObjectiveProgress(FName ObjectiveID, int32 ProgressAmount)
{
    for (FQuest_ObjectiveData& Obj : ActiveObjectives)
    {
        if (Obj.ObjectiveID == ObjectiveID && Obj.ObjectiveState == EQuest_ObjectiveState::InProgress)
        {
            Obj.CurrentCount = FMath::Min(Obj.CurrentCount + ProgressAmount, Obj.TargetCount);

            if (Obj.CurrentCount >= Obj.TargetCount)
            {
                Obj.ObjectiveState = EQuest_ObjectiveState::Completed;
                OnObjectiveCompleted.Broadcast(ObjectiveID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective completed — %s"), *ObjectiveID.ToString());
                AdvanceToNextObjective(ObjectiveID);
            }
            return true;
        }
    }
    return false;
}

void UQuestManager::AdvanceToNextObjective(FName CompletedObjectiveID)
{
    if (ActiveQuestIndex < 0 || ActiveQuestIndex >= QuestDatabase.Num()) return;

    FQuest_QuestData& Quest = QuestDatabase[ActiveQuestIndex];
    bool bFoundCompleted = false;

    for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
    {
        if (Quest.Objectives[i].ObjectiveID == CompletedObjectiveID)
        {
            bFoundCompleted = true;
            // Activate next objective if exists
            if (i + 1 < Quest.Objectives.Num())
            {
                Quest.Objectives[i + 1].ObjectiveState = EQuest_ObjectiveState::InProgress;
                ActiveObjectives.Add(Quest.Objectives[i + 1]);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Next objective activated — %s"), *Quest.Objectives[i + 1].ObjectiveID.ToString());
            }
            else
            {
                // All objectives done — complete quest
                CompleteQuest(Quest.QuestID);
            }
            break;
        }
    }
}

void UQuestManager::CompleteQuest(FName QuestID)
{
    for (FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.QuestState = EQuest_QuestState::Completed;
            CompletedQuestIDs.Add(QuestID);
            ActiveQuestIndex = -1;
            ActiveObjectives.Empty();
            OnQuestCompleted.Broadcast(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s"), *QuestID.ToString());
            return;
        }
    }
}

void UQuestManager::FailQuest(FName QuestID)
{
    for (FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID && Quest.QuestState == EQuest_QuestState::Active)
        {
            Quest.QuestState = EQuest_QuestState::Failed;
            ActiveQuestIndex = -1;
            ActiveObjectives.Empty();
            OnQuestFailed.Broadcast(QuestID);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest FAILED — %s"), *QuestID.ToString());
            return;
        }
    }
}

void UQuestManager::OnObjectiveFailed(FName ObjectiveID)
{
    // Find parent quest and fail it
    if (ActiveQuestIndex >= 0 && ActiveQuestIndex < QuestDatabase.Num())
    {
        FailQuest(QuestDatabase[ActiveQuestIndex].QuestID);
    }
}

FQuest_QuestData UQuestManager::GetQuestData(FName QuestID) const
{
    for (const FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_QuestData> Available;
    for (const FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestState == EQuest_QuestState::Available)
        {
            Available.Add(Quest);
        }
    }
    return Available;
}

TArray<FQuest_QuestData> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> Active;
    for (const FQuest_QuestData& Quest : QuestDatabase)
    {
        if (Quest.QuestState == EQuest_QuestState::Active)
        {
            Active.Add(Quest);
        }
    }
    return Active;
}

bool UQuestManager::IsQuestCompleted(FName QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

int32 UQuestManager::GetTotalQuestCount() const
{
    return QuestDatabase.Num();
}

int32 UQuestManager::GetCompletedQuestCount() const
{
    return CompletedQuestIDs.Num();
}
