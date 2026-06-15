#include "Quest_RewardManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UQuest_RewardManager::UQuest_RewardManager()
{
    RewardDataTable = nullptr;
    PlayerExperience = 0;
    PlayerReputation = 0;
}

void UQuest_RewardManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadRewardData();
}

void UQuest_RewardManager::LoadRewardData()
{
    // Load reward data table from content
    RewardDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/TranspersonalGame/Data/DT_QuestRewards"));
    
    if (!RewardDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_RewardManager: Failed to load reward data table"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Reward data table loaded successfully"));
    }
}

bool UQuest_RewardManager::GrantReward(const FString& RewardBundleName, AActor* Player)
{
    if (!RewardDataTable || !Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_RewardManager: Cannot grant reward - missing data table or player"));
        return false;
    }

    // Find reward bundle
    FQuest_RewardBundle* RewardBundle = RewardDataTable->FindRow<FQuest_RewardBundle>(FName(*RewardBundleName), TEXT(""));
    
    if (!RewardBundle)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_RewardManager: Reward bundle not found: %s"), *RewardBundleName);
        return false;
    }

    if (!ValidateReward(*RewardBundle))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_RewardManager: Invalid reward bundle: %s"), *RewardBundleName);
        return false;
    }

    // Grant all items in the bundle
    for (const FQuest_RewardItem& Item : RewardBundle->RewardItems)
    {
        GrantSingleReward(Item, Player);
    }

    // Grant experience
    if (RewardBundle->ExperiencePoints > 0)
    {
        GrantExperience(RewardBundle->ExperiencePoints, Player);
    }

    // Grant reputation
    if (RewardBundle->ReputationPoints > 0)
    {
        GrantReputation(RewardBundle->ReputationPoints, Player);
    }

    // Handle unlockable content
    if (!RewardBundle->UnlockableContent.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Unlocked content: %s"), *RewardBundle->UnlockableContent);
        // This would integrate with progression system
    }

    UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Granted reward bundle: %s"), *RewardBundleName);
    return true;
}

bool UQuest_RewardManager::GrantSingleReward(const FQuest_RewardItem& RewardItem, AActor* Player)
{
    if (!Player)
    {
        return false;
    }

    // Add item to inventory
    AddItemToInventory(RewardItem);

    // Notify about reward
    NotifyRewardGranted(RewardItem);

    UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Granted %s x%d to player"), 
           *RewardItem.ItemName, RewardItem.Quantity);

    return true;
}

void UQuest_RewardManager::GrantExperience(int32 ExperiencePoints, AActor* Player)
{
    if (!Player || ExperiencePoints <= 0)
    {
        return;
    }

    PlayerExperience += ExperiencePoints;
    
    // Broadcast experience gained event
    OnExperienceGained.Broadcast(ExperiencePoints);

    UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Granted %d experience points. Total: %d"), 
           ExperiencePoints, PlayerExperience);
}

void UQuest_RewardManager::GrantReputation(int32 ReputationPoints, AActor* Player)
{
    if (!Player || ReputationPoints == 0)
    {
        return;
    }

    PlayerReputation += ReputationPoints;

    UE_LOG(LogTemp, Log, TEXT("Quest_RewardManager: Granted %d reputation points. Total: %d"), 
           ReputationPoints, PlayerReputation);
}

TArray<FQuest_RewardItem> UQuest_RewardManager::GetRewardPreview(const FString& RewardBundleName)
{
    TArray<FQuest_RewardItem> PreviewItems;

    if (!RewardDataTable)
    {
        return PreviewItems;
    }

    FQuest_RewardBundle* RewardBundle = RewardDataTable->FindRow<FQuest_RewardBundle>(FName(*RewardBundleName), TEXT(""));
    
    if (RewardBundle)
    {
        PreviewItems = RewardBundle->RewardItems;
    }

    return PreviewItems;
}

bool UQuest_RewardManager::HasRewardBundle(const FString& RewardBundleName) const
{
    if (!RewardDataTable)
    {
        return false;
    }

    return RewardDataTable->FindRow<FQuest_RewardBundle>(FName(*RewardBundleName), TEXT("")) != nullptr;
}

void UQuest_RewardManager::AddItemToInventory(const FQuest_RewardItem& Item)
{
    if (PlayerInventory.Contains(Item.ItemName))
    {
        PlayerInventory[Item.ItemName] += Item.Quantity;
    }
    else
    {
        PlayerInventory.Add(Item.ItemName, Item.Quantity);
    }
}

void UQuest_RewardManager::NotifyRewardGranted(const FQuest_RewardItem& Item)
{
    OnRewardGranted.Broadcast(Item.ItemName, Item.Quantity);
}

bool UQuest_RewardManager::ValidateReward(const FQuest_RewardBundle& RewardBundle) const
{
    // Basic validation
    if (RewardBundle.BundleName.IsEmpty())
    {
        return false;
    }

    // Check for valid reward items
    for (const FQuest_RewardItem& Item : RewardBundle.RewardItems)
    {
        if (Item.ItemName.IsEmpty() || Item.Quantity <= 0)
        {
            return false;
        }
    }

    return true;
}