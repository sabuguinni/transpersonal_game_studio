#include "Quest_QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UQuest_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeSurvivalQuests();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized with survival quests"));
}

void UQuest_QuestManager::Deinitialize()
{
    RegisteredQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    ObjectiveProgress.Empty();
    
    Super::Deinitialize();
}

bool UQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest not found: %s"), *QuestID);
        return false;
    }

    if (ActiveQuestIDs.Contains(QuestID) || CompletedQuestIDs.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest already active or completed: %s"), *QuestID);
        return false;
    }

    ActiveQuestIDs.Add(QuestID);
    
    // Initialize objective progress
    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    for (const FString& ObjectiveID : QuestData.ObjectiveIDs)
    {
        ObjectiveProgress.Add(ObjectiveID, 0);
    }

    UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *QuestData.QuestName);
    return true;
}

bool UQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    if (!ActiveQuestIDs.Contains(QuestID))
    {
        return false;
    }

    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.Add(QuestID);

    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *QuestData.QuestName);
    
    return true;
}

bool UQuest_QuestManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool UQuest_QuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (RegisteredQuests.Contains(QuestID))
        {
            ActiveQuests.Add(RegisteredQuests[QuestID]);
        }
    }
    
    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetCompletedQuests() const
{
    TArray<FQuest_QuestData> CompletedQuests;
    
    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (RegisteredQuests.Contains(QuestID))
        {
            CompletedQuests.Add(RegisteredQuests[QuestID]);
        }
    }
    
    return CompletedQuests;
}

bool UQuest_QuestManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 Progress)
{
    if (!ObjectiveProgress.Contains(ObjectiveID))
    {
        return false;
    }

    ObjectiveProgress[ObjectiveID] = Progress;
    
    // Check if any quest should be completed
    for (const FString& QuestID : ActiveQuestIDs)
    {
        CheckQuestCompletion(QuestID);
    }

    UE_LOG(LogTemp, Log, TEXT("Updated objective %s progress to %d"), *ObjectiveID, Progress);
    return true;
}

bool UQuest_QuestManager::CompleteObjective(const FString& ObjectiveID)
{
    return UpdateObjectiveProgress(ObjectiveID, 1);
}

int32 UQuest_QuestManager::GetObjectiveProgress(const FString& ObjectiveID) const
{
    if (ObjectiveProgress.Contains(ObjectiveID))
    {
        return ObjectiveProgress[ObjectiveID];
    }
    return 0;
}

void UQuest_QuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    RegisteredQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("Registered quest: %s"), *QuestData.QuestName);
}

void UQuest_QuestManager::RegisterSurvivalQuests()
{
    InitializeSurvivalQuests();
}

void UQuest_QuestManager::InitializeSurvivalQuests()
{
    // First Hunt Quest
    FQuest_QuestData FirstHunt;
    FirstHunt.QuestID = TEXT("QUEST_FIRST_HUNT");
    FirstHunt.QuestName = TEXT("First Hunt");
    FirstHunt.QuestDescription = TEXT("Prove your worth as a hunter by taking down your first dinosaur. Find a small herbivore and use your primitive weapons to hunt it down.");
    FirstHunt.ObjectiveIDs.Add(TEXT("OBJ_KILL_HERBIVORE"));
    FirstHunt.bIsMainQuest = true;
    FirstHunt.RequiredLevel = 1;
    RegisterQuest(FirstHunt);

    // Gather Resources Quest
    FQuest_QuestData GatherResources;
    GatherResources.QuestID = TEXT("QUEST_GATHER_RESOURCES");
    GatherResources.QuestName = TEXT("Gather Basic Resources");
    GatherResources.QuestDescription = TEXT("Survival requires preparation. Gather stones, sticks, and leaves to craft your first tools.");
    GatherResources.ObjectiveIDs.Add(TEXT("OBJ_COLLECT_STONES"));
    GatherResources.ObjectiveIDs.Add(TEXT("OBJ_COLLECT_STICKS"));
    GatherResources.ObjectiveIDs.Add(TEXT("OBJ_COLLECT_LEAVES"));
    GatherResources.bIsMainQuest = false;
    GatherResources.RequiredLevel = 1;
    RegisterQuest(GatherResources);

    // Explore Territory Quest
    FQuest_QuestData ExploreTerritory;
    ExploreTerritory.QuestID = TEXT("QUEST_EXPLORE_TERRITORY");
    ExploreTerritory.QuestName = TEXT("Explore the Valley");
    ExploreTerritory.QuestDescription = TEXT("Knowledge of the land is survival. Explore the valley and discover key locations: water sources, shelter spots, and hunting grounds.");
    ExploreTerritory.ObjectiveIDs.Add(TEXT("OBJ_FIND_WATER"));
    ExploreTerritory.ObjectiveIDs.Add(TEXT("OBJ_FIND_SHELTER"));
    ExploreTerritory.ObjectiveIDs.Add(TEXT("OBJ_FIND_HUNTING_GROUND"));
    ExploreTerritory.bIsMainQuest = false;
    ExploreTerritory.RequiredLevel = 1;
    RegisterQuest(ExploreTerritory);

    // Avoid Predators Quest
    FQuest_QuestData AvoidPredators;
    AvoidPredators.QuestID = TEXT("QUEST_AVOID_PREDATORS");
    AvoidPredators.QuestName = TEXT("Survive the Predators");
    AvoidPredators.QuestDescription = TEXT("The valley is home to dangerous predators. Learn to avoid T-Rex and Velociraptor territories, or face certain death.");
    AvoidPredators.ObjectiveIDs.Add(TEXT("OBJ_AVOID_TREX"));
    AvoidPredators.ObjectiveIDs.Add(TEXT("OBJ_AVOID_RAPTORS"));
    AvoidPredators.ObjectiveIDs.Add(TEXT("OBJ_SURVIVE_24_HOURS"));
    AvoidPredators.bIsMainQuest = true;
    AvoidPredators.RequiredLevel = 2;
    RegisterQuest(AvoidPredators);

    // Build Shelter Quest
    FQuest_QuestData BuildShelter;
    BuildShelter.QuestID = TEXT("QUEST_BUILD_SHELTER");
    BuildShelter.QuestName = TEXT("Build Your First Shelter");
    BuildShelter.QuestDescription = TEXT("The elements and predators make sleeping in the open deadly. Craft a basic shelter to protect yourself during the night.");
    BuildShelter.ObjectiveIDs.Add(TEXT("OBJ_CRAFT_SHELTER"));
    BuildShelter.ObjectiveIDs.Add(TEXT("OBJ_SLEEP_IN_SHELTER"));
    BuildShelter.bIsMainQuest = false;
    BuildShelter.RequiredLevel = 2;
    RegisterQuest(BuildShelter);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d survival quests"), RegisteredQuests.Num());
}

void UQuest_QuestManager::CheckQuestCompletion(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return;
    }

    if (AreAllObjectivesComplete(QuestID))
    {
        CompleteQuest(QuestID);
    }
}

bool UQuest_QuestManager::AreAllObjectivesComplete(const FString& QuestID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    for (const FString& ObjectiveID : QuestData.ObjectiveIDs)
    {
        if (!ObjectiveProgress.Contains(ObjectiveID) || ObjectiveProgress[ObjectiveID] == 0)
        {
            return false;
        }
    }

    return true;
}