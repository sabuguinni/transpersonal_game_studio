#include "QuestSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UQuestSystem::UQuestSystem()
{
    // Initialize default values
}

void UQuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest System Initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
}

void UQuestSystem::StartQuest(const FString& QuestID)
{
    // Find quest in all quests
    FQuest_Data* QuestToStart = nullptr;
    for (FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            QuestToStart = &Quest;
            break;
        }
    }

    if (!QuestToStart)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest not found: %s"), *QuestID);
        return;
    }

    // Check if already active
    for (const FQuest_Data& ActiveQuest : ActiveQuests)
    {
        if (ActiveQuest.QuestID == QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest already active: %s"), *QuestID);
            return;
        }
    }

    // Start the quest
    QuestToStart->QuestStatus = EQuest_Status::InProgress;
    ActiveQuests.Add(*QuestToStart);

    UE_LOG(LogTemp, Warning, TEXT("Quest Started: %s - %s"), *QuestToStart->QuestName, *QuestToStart->QuestDescription);
}

void UQuestSystem::CompleteQuest(const FString& QuestID)
{
    // Find and remove from active quests
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].QuestStatus = EQuest_Status::Completed;
            CompletedQuests.Add(ActiveQuests[i]);
            
            OnQuestCompleted(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest Completed: %s"), *QuestID);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Active quest not found for completion: %s"), *QuestID);
}

void UQuestSystem::UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress)
{
    // Find active quest
    for (FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Quest.Objectives.Num())
            {
                Quest.Objectives[ObjectiveIndex].CurrentCount += Progress;
                
                // Clamp to target count
                if (Quest.Objectives[ObjectiveIndex].CurrentCount >= Quest.Objectives[ObjectiveIndex].TargetCount)
                {
                    Quest.Objectives[ObjectiveIndex].CurrentCount = Quest.Objectives[ObjectiveIndex].TargetCount;
                    Quest.Objectives[ObjectiveIndex].bIsCompleted = true;
                }

                CheckObjectiveCompletion(Quest);
                
                UE_LOG(LogTemp, Warning, TEXT("Objective Updated: %s [%d] - Progress: %d/%d"), 
                    *QuestID, ObjectiveIndex, 
                    Quest.Objectives[ObjectiveIndex].CurrentCount,
                    Quest.Objectives[ObjectiveIndex].TargetCount);
            }
            return;
        }
    }
}

bool UQuestSystem::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

TArray<FQuest_Data> UQuestSystem::GetActiveQuests() const
{
    return ActiveQuests;
}

FQuest_Data UQuestSystem::GetQuestData(const FString& QuestID) const
{
    // Check active quests first
    for (const FQuest_Data& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }

    // Check all quests
    for (const FQuest_Data& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }

    // Return empty quest data if not found
    return FQuest_Data();
}

void UQuestSystem::RegisterQuestGiver(AActor* QuestGiver, const FString& QuestID)
{
    if (QuestGiver)
    {
        QuestGivers.Add(QuestGiver, QuestID);
        UE_LOG(LogTemp, Warning, TEXT("Quest Giver Registered: %s for Quest: %s"), 
            *QuestGiver->GetName(), *QuestID);
    }
}

void UQuestSystem::InitializeDefaultQuests()
{
    // Quest 1: First Hunt
    FQuest_Data FirstHunt;
    FirstHunt.QuestID = TEXT("QUEST_001");
    FirstHunt.QuestName = TEXT("First Hunt");
    FirstHunt.QuestDescription = TEXT("Hunt a small dinosaur to prove your survival skills. Target: Compsognathus or similar small herbivore.");
    FirstHunt.QuestStatus = EQuest_Status::NotStarted;
    FirstHunt.ExperienceReward = 150;
    FirstHunt.ItemRewards.Add(TEXT("Stone Spear"));

    FQuest_ObjectiveData HuntObjective;
    HuntObjective.ObjectiveDescription = TEXT("Hunt 1 small dinosaur");
    HuntObjective.ObjectiveType = EQuest_ObjectiveType::Hunt;
    HuntObjective.TargetCount = 1;
    FirstHunt.Objectives.Add(HuntObjective);

    AllQuests.Add(FirstHunt);

    // Quest 2: Gather Resources
    FQuest_Data GatherResources;
    GatherResources.QuestID = TEXT("QUEST_002");
    GatherResources.QuestName = TEXT("Resource Gathering");
    GatherResources.QuestDescription = TEXT("Collect basic materials needed for crafting tools and shelter.");
    GatherResources.QuestStatus = EQuest_Status::NotStarted;
    GatherResources.ExperienceReward = 100;
    GatherResources.ItemRewards.Add(TEXT("Basic Toolkit"));

    FQuest_ObjectiveData GatherStones;
    GatherStones.ObjectiveDescription = TEXT("Collect 5 stones");
    GatherStones.ObjectiveType = EQuest_ObjectiveType::Collect;
    GatherStones.TargetCount = 5;
    GatherResources.Objectives.Add(GatherStones);

    FQuest_ObjectiveData GatherSticks;
    GatherSticks.ObjectiveDescription = TEXT("Collect 3 sticks");
    GatherSticks.ObjectiveType = EQuest_ObjectiveType::Collect;
    GatherSticks.TargetCount = 3;
    GatherResources.Objectives.Add(GatherSticks);

    AllQuests.Add(GatherResources);

    // Quest 3: Explore Territory
    FQuest_Data ExploreTerritory;
    ExploreTerritory.QuestID = TEXT("QUEST_003");
    ExploreTerritory.QuestName = TEXT("Territory Exploration");
    ExploreTerritory.QuestDescription = TEXT("Explore the surrounding area to locate key landmarks and resources.");
    ExploreTerritory.QuestStatus = EQuest_Status::NotStarted;
    ExploreTerritory.ExperienceReward = 200;
    ExploreTerritory.ItemRewards.Add(TEXT("Map Fragment"));

    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveDescription = TEXT("Discover 3 new locations");
    ExploreObjective.ObjectiveType = EQuest_ObjectiveType::Explore;
    ExploreObjective.TargetCount = 3;
    ExploreTerritory.Objectives.Add(ExploreObjective);

    AllQuests.Add(ExploreTerritory);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default quests"), AllQuests.Num());
}

void UQuestSystem::CheckObjectiveCompletion(FQuest_Data& Quest)
{
    bool bAllObjectivesComplete = true;
    for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (!Objective.bIsCompleted)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }

    if (bAllObjectivesComplete)
    {
        CompleteQuest(Quest.QuestID);
    }
}

void UQuestSystem::OnQuestCompleted(const FQuest_Data& Quest)
{
    // Award experience and items
    UE_LOG(LogTemp, Warning, TEXT("Quest Rewards: %d XP"), Quest.ExperienceReward);
    
    for (const FString& ItemReward : Quest.ItemRewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item Reward: %s"), *ItemReward);
    }

    // Here you would integrate with inventory/character progression systems
    // For now, just log the rewards
}

// Quest Component Implementation
UQuestComponent::UQuestComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    QuestID = TEXT("");
    bAutoStartOnInteraction = true;
    InteractionRange = 300.0f;
    QuestSystem = nullptr;
}

void UQuestComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get quest system reference
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        QuestSystem = GameInstance->GetSubsystem<UQuestSystem>();
    }

    if (QuestSystem && !QuestID.IsEmpty())
    {
        QuestSystem->RegisterQuestGiver(GetOwner(), QuestID);
    }
}

void UQuestComponent::TriggerQuestInteraction()
{
    if (!QuestSystem || QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest Component: Invalid quest system or quest ID"));
        return;
    }

    if (bAutoStartOnInteraction)
    {
        if (!QuestSystem->IsQuestActive(QuestID))
        {
            QuestSystem->StartQuest(QuestID);
            UE_LOG(LogTemp, Warning, TEXT("Quest triggered via interaction: %s"), *QuestID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest already active: %s"), *QuestID);
        }
    }
}

void UQuestComponent::SetQuestID(const FString& NewQuestID)
{
    QuestID = NewQuestID;
    
    if (QuestSystem && GetOwner())
    {
        QuestSystem->RegisterQuestGiver(GetOwner(), QuestID);
    }
}