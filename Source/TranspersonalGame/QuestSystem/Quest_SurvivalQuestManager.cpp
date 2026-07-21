#include "Quest_SurvivalQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalCharacter.h"

UQuest_SurvivalQuestManager::UQuest_SurvivalQuestManager()
{
    TotalSurvivalTime = 0.0f;
    PlayerCharacter = nullptr;
}

void UQuest_SurvivalQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestManager initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
    
    // Clear counters
    DinosaurKillCounts.Empty();
    ResourceCounts.Empty();
    TotalSurvivalTime = 0.0f;
}

void UQuest_SurvivalQuestManager::Deinitialize()
{
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    DinosaurKillCounts.Empty();
    ResourceCounts.Empty();
    PlayerCharacter = nullptr;
    
    Super::Deinitialize();
}

void UQuest_SurvivalQuestManager::StartQuest(const FString& QuestID)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.bIsActive = true;
            UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *QuestID);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest not found: %s"), *QuestID);
}

void UQuest_SurvivalQuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].bIsCompleted = true;
            ActiveQuests[i].bIsActive = false;
            
            // Move to completed quests
            CompletedQuests.Add(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            
            BroadcastQuestCompletion(QuestID);
            UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *QuestID);
            return;
        }
    }
}

void UQuest_SurvivalQuestManager::FailQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].bIsActive = false;
            UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s"), *QuestID);
            return;
        }
    }
}

bool UQuest_SurvivalQuestManager::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.bIsActive;
        }
    }
    return false;
}

bool UQuest_SurvivalQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    for (const FQuest_SurvivalQuest& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest.bIsCompleted;
        }
    }
    return false;
}

FQuest_SurvivalQuest UQuest_SurvivalQuestManager::GetQuest(const FString& QuestID) const
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    for (const FQuest_SurvivalQuest& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    return FQuest_SurvivalQuest();
}

TArray<FQuest_SurvivalQuest> UQuest_SurvivalQuestManager::GetActiveQuests() const
{
    TArray<FQuest_SurvivalQuest> Result;
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.bIsActive)
        {
            Result.Add(Quest);
        }
    }
    return Result;
}

void UQuest_SurvivalQuestManager::UpdateSurvivalProgress(float DeltaTime)
{
    // Update total survival time
    TotalSurvivalTime += DeltaTime;
    
    // Get player character if not cached
    if (!PlayerCharacter)
    {
        if (UWorld* World = GetWorld())
        {
            PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
        }
    }
    
    // Check quest completion every few seconds
    static float CheckTimer = 0.0f;
    CheckTimer += DeltaTime;
    if (CheckTimer >= 5.0f) // Check every 5 seconds
    {
        CheckQuestCompletion();
        CheckTimer = 0.0f;
    }
}

void UQuest_SurvivalQuestManager::OnDinosaurKilled(const FString& DinosaurType)
{
    int32* CurrentCount = DinosaurKillCounts.Find(DinosaurType);
    if (CurrentCount)
    {
        (*CurrentCount)++;
    }
    else
    {
        DinosaurKillCounts.Add(DinosaurType, 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur killed: %s (Total: %d)"), *DinosaurType, DinosaurKillCounts[DinosaurType]);
    
    CheckQuestCompletion();
}

void UQuest_SurvivalQuestManager::OnResourceGathered(const FString& ResourceType)
{
    int32* CurrentCount = ResourceCounts.Find(ResourceType);
    if (CurrentCount)
    {
        (*CurrentCount)++;
    }
    else
    {
        ResourceCounts.Add(ResourceType, 1);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Resource gathered: %s (Total: %d)"), *ResourceType, ResourceCounts[ResourceType]);
    
    CheckQuestCompletion();
}

void UQuest_SurvivalQuestManager::CheckQuestCompletion()
{
    TArray<FString> QuestsToComplete;
    
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (!Quest.bIsActive || Quest.bIsCompleted)
        {
            continue;
        }
        
        bool bCanComplete = true;
        
        // Check survival requirements
        if (!CheckSurvivalRequirements(Quest))
        {
            bCanComplete = false;
        }
        
        // Check kill requirements
        if (!CheckKillRequirements(Quest))
        {
            bCanComplete = false;
        }
        
        // Check resource requirements
        if (!CheckResourceRequirements(Quest))
        {
            bCanComplete = false;
        }
        
        if (bCanComplete)
        {
            QuestsToComplete.Add(Quest.QuestID);
        }
    }
    
    // Complete eligible quests
    for (const FString& QuestID : QuestsToComplete)
    {
        CompleteQuest(QuestID);
    }
}

void UQuest_SurvivalQuestManager::CreateSurvivalQuest(const FString& QuestID, const FString& Name, const FString& Description, float SurvivalTime)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestName = Name;
    NewQuest.QuestDescription = Description;
    NewQuest.RequiredSurvivalTime = SurvivalTime;
    NewQuest.bIsActive = false;
    NewQuest.bIsCompleted = false;
    
    ActiveQuests.Add(NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created survival quest: %s"), *QuestID);
}

void UQuest_SurvivalQuestManager::CreateHuntingQuest(const FString& QuestID, const FString& Name, const FString& Description, const FString& DinosaurType, int32 KillCount)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestName = Name;
    NewQuest.QuestDescription = Description;
    NewQuest.RequiredDinosaurKills = KillCount;
    NewQuest.bIsActive = false;
    NewQuest.bIsCompleted = false;
    
    ActiveQuests.Add(NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created hunting quest: %s"), *QuestID);
}

void UQuest_SurvivalQuestManager::CreateGatheringQuest(const FString& QuestID, const FString& Name, const FString& Description, const TArray<FString>& Resources)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestName = Name;
    NewQuest.QuestDescription = Description;
    NewQuest.RequiredResources = Resources;
    NewQuest.bIsActive = false;
    NewQuest.bIsCompleted = false;
    
    ActiveQuests.Add(NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created gathering quest: %s"), *QuestID);
}

void UQuest_SurvivalQuestManager::InitializeDefaultQuests()
{
    // Survival Quest: Stay alive for 10 minutes
    CreateSurvivalQuest(
        "SURVIVE_10MIN",
        "First Survival",
        "Survive in the prehistoric world for 10 minutes while maintaining health above 50%",
        600.0f // 10 minutes
    );
    
    // Hunting Quest: Kill 3 Raptors
    CreateHuntingQuest(
        "HUNT_RAPTORS",
        "Raptor Hunter",
        "Hunt and kill 3 Velociraptors to prove your hunting skills",
        "Raptor",
        3
    );
    
    // Gathering Quest: Collect basic resources
    TArray<FString> BasicResources;
    BasicResources.Add("Stone");
    BasicResources.Add("Wood");
    BasicResources.Add("Fiber");
    
    CreateGatheringQuest(
        "GATHER_BASICS",
        "Resource Gatherer",
        "Collect basic survival resources: Stone, Wood, and Fiber",
        BasicResources
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default quests"), ActiveQuests.Num());
}

bool UQuest_SurvivalQuestManager::CheckSurvivalRequirements(const FQuest_SurvivalQuest& Quest) const
{
    // Check survival time
    if (Quest.RequiredSurvivalTime > 0.0f && TotalSurvivalTime < Quest.RequiredSurvivalTime)
    {
        return false;
    }
    
    // Check player health/stats if we have a player reference
    if (PlayerCharacter)
    {
        if (Quest.MinHealthThreshold > 0.0f && PlayerCharacter->GetHealth() < Quest.MinHealthThreshold)
        {
            return false;
        }
        
        if (Quest.MinHungerThreshold > 0.0f && PlayerCharacter->GetHunger() < Quest.MinHungerThreshold)
        {
            return false;
        }
        
        if (Quest.MinThirstThreshold > 0.0f && PlayerCharacter->GetThirst() < Quest.MinThirstThreshold)
        {
            return false;
        }
    }
    
    return true;
}

bool UQuest_SurvivalQuestManager::CheckKillRequirements(const FQuest_SurvivalQuest& Quest) const
{
    if (Quest.RequiredDinosaurKills <= 0)
    {
        return true; // No kill requirement
    }
    
    // For now, check total kills across all dinosaur types
    int32 TotalKills = 0;
    for (const auto& KillPair : DinosaurKillCounts)
    {
        TotalKills += KillPair.Value;
    }
    
    return TotalKills >= Quest.RequiredDinosaurKills;
}

bool UQuest_SurvivalQuestManager::CheckResourceRequirements(const FQuest_SurvivalQuest& Quest) const
{
    if (Quest.RequiredResources.Num() == 0)
    {
        return true; // No resource requirement
    }
    
    for (const FString& RequiredResource : Quest.RequiredResources)
    {
        const int32* ResourceCount = ResourceCounts.Find(RequiredResource);
        if (!ResourceCount || *ResourceCount <= 0)
        {
            return false; // Missing required resource
        }
    }
    
    return true;
}

void UQuest_SurvivalQuestManager::BroadcastQuestCompletion(const FString& QuestID)
{
    // In a full implementation, this would broadcast to UI, achievement system, etc.
    UE_LOG(LogTemp, Warning, TEXT("QUEST COMPLETED: %s"), *QuestID);
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Quest Completed: %s"), *QuestID);
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, Message);
    }
}