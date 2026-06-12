#include "Quest_ResourceQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuest_ResourceQuestManager::UQuest_ResourceQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    SearchRadius = 1500.0f;
    bShowResourceMarkers = true;
    MaxActiveGathering = 5;
    GatheringTimeLimit = 900.0f; // 15 minutes
}

void UQuest_ResourceQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultResourceQuests();
}

void UQuest_ResourceQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bShowResourceMarkers)
    {
        UpdateResourceMarkers();
    }
    
    CheckGatheringTimeouts();
}

void UQuest_ResourceQuestManager::StartResourceQuest(const FString& ResourceType, int32 RequiredAmount, const FString& Location)
{
    if (ActiveResourceTargets.Num() >= MaxActiveGathering)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active resource quests reached. Cannot start gathering %s"), *ResourceType);
        return;
    }

    // Check if quest already exists
    if (FindResourceTarget(ResourceType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Resource quest for %s already active"), *ResourceType);
        return;
    }

    FQuest_ResourceTarget NewTarget;
    NewTarget.ResourceType = ResourceType;
    NewTarget.RequiredAmount = RequiredAmount;
    NewTarget.CurrentAmount = 0;
    NewTarget.SourceLocation = Location;
    
    // Set quality threshold based on resource type
    if (ResourceType == TEXT("Stone"))
    {
        NewTarget.QualityThreshold = 0.3f;
    }
    else if (ResourceType == TEXT("Wood"))
    {
        NewTarget.QualityThreshold = 0.4f;
    }
    else if (ResourceType == TEXT("Fiber"))
    {
        NewTarget.QualityThreshold = 0.2f;
    }
    else
    {
        NewTarget.QualityThreshold = 0.5f;
    }

    ActiveResourceTargets.Add(NewTarget);
    
    UE_LOG(LogTemp, Log, TEXT("Started resource quest for %s - Required: %d at %s"), 
           *ResourceType, RequiredAmount, *Location);
}

void UQuest_ResourceQuestManager::RegisterResourceGathered(const FString& ResourceType, int32 Amount, float Quality)
{
    FQuest_ResourceTarget* ResourceTarget = FindResourceTarget(ResourceType);
    if (!ResourceTarget)
    {
        return;
    }

    // Check quality requirement
    if (!ValidateResourceQuality(Quality, ResourceTarget->QualityThreshold))
    {
        UE_LOG(LogTemp, Warning, TEXT("Gathered %s quality too low (%.2f < %.2f)"), 
               *ResourceType, Quality, ResourceTarget->QualityThreshold);
        return;
    }

    ResourceTarget->CurrentAmount += Amount;
    UE_LOG(LogTemp, Log, TEXT("Gathered %s: +%d (%d/%d), Quality: %.2f"), 
           *ResourceType, Amount, ResourceTarget->CurrentAmount, ResourceTarget->RequiredAmount, Quality);

    if (CheckResourceQuestCompletion(ResourceType))
    {
        CompleteResourceQuest(ResourceType);
    }
}

bool UQuest_ResourceQuestManager::CheckResourceQuestCompletion(const FString& ResourceType)
{
    FQuest_ResourceTarget* ResourceTarget = FindResourceTarget(ResourceType);
    if (!ResourceTarget)
    {
        return false;
    }

    return ResourceTarget->CurrentAmount >= ResourceTarget->RequiredAmount;
}

void UQuest_ResourceQuestManager::CompleteResourceQuest(const FString& ResourceType)
{
    FQuest_ResourceTarget* ResourceTarget = FindResourceTarget(ResourceType);
    if (!ResourceTarget)
    {
        return;
    }

    // Award rewards
    for (const FQuest_ResourceReward& Reward : QuestRewards)
    {
        UE_LOG(LogTemp, Log, TEXT("Awarded: %s x%d, Crafting XP: %.1f"), 
               *Reward.RewardType, Reward.Amount, Reward.CraftingExperience);
    }

    // Remove completed quest
    ActiveResourceTargets.RemoveAll([ResourceType](const FQuest_ResourceTarget& Target)
    {
        return Target.ResourceType == ResourceType;
    });

    UE_LOG(LogTemp, Log, TEXT("Completed resource quest for %s"), *ResourceType);
}

TArray<FString> UQuest_ResourceQuestManager::GetActiveResourceTargets()
{
    TArray<FString> Targets;
    for (const FQuest_ResourceTarget& Target : ActiveResourceTargets)
    {
        Targets.Add(Target.ResourceType);
    }
    return Targets;
}

float UQuest_ResourceQuestManager::GetResourceProgress(const FString& ResourceType)
{
    FQuest_ResourceTarget* ResourceTarget = FindResourceTarget(ResourceType);
    if (!ResourceTarget || ResourceTarget->RequiredAmount == 0)
    {
        return 0.0f;
    }

    return static_cast<float>(ResourceTarget->CurrentAmount) / static_cast<float>(ResourceTarget->RequiredAmount);
}

void UQuest_ResourceQuestManager::UpdateResourceMarkers()
{
    // This would update UI markers showing resource locations on the map
    // Implementation would depend on UI system
}

void UQuest_ResourceQuestManager::CancelResourceQuest(const FString& ResourceType)
{
    ActiveResourceTargets.RemoveAll([ResourceType](const FQuest_ResourceTarget& Target)
    {
        return Target.ResourceType == ResourceType;
    });

    UE_LOG(LogTemp, Log, TEXT("Cancelled resource quest for %s"), *ResourceType);
}

TArray<FString> UQuest_ResourceQuestManager::GetNearbyResources(float Range)
{
    TArray<FString> NearbyResources;
    
    // This would scan for resource nodes within range
    // For now, return common resources
    NearbyResources.Add(TEXT("Stone"));
    NearbyResources.Add(TEXT("Wood"));
    NearbyResources.Add(TEXT("Fiber"));
    
    return NearbyResources;
}

void UQuest_ResourceQuestManager::InitializeDefaultResourceQuests()
{
    // Setup default reward structure
    FQuest_ResourceReward ToolReward;
    ToolReward.RewardType = TEXT("Stone Tool Blueprint");
    ToolReward.Amount = 1;
    ToolReward.CraftingExperience = 50.0f;

    FQuest_ResourceReward MaterialReward;
    MaterialReward.RewardType = TEXT("Refined Materials");
    MaterialReward.Amount = 5;
    MaterialReward.CraftingExperience = 25.0f;

    QuestRewards.Add(ToolReward);
    QuestRewards.Add(MaterialReward);
}

void UQuest_ResourceQuestManager::CheckGatheringTimeouts()
{
    // Implementation for time-limited gathering
    // Would check elapsed time and cancel expired quests
}

FQuest_ResourceTarget* UQuest_ResourceQuestManager::FindResourceTarget(const FString& ResourceType)
{
    for (FQuest_ResourceTarget& Target : ActiveResourceTargets)
    {
        if (Target.ResourceType == ResourceType)
        {
            return &Target;
        }
    }
    return nullptr;
}

bool UQuest_ResourceQuestManager::ValidateResourceQuality(float Quality, float Threshold)
{
    return Quality >= Threshold;
}