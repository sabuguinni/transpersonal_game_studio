#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Quest_RewardManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Experience = 0,
    Items,
    Tools,
    Weapons,
    Food,
    Materials,
    Knowledge,
    Reputation
};

UENUM(BlueprintType)
enum class EQuest_ItemRarity : uint8
{
    Common = 0,
    Uncommon,
    Rare,
    Epic,
    Legendary
};

USTRUCT(BlueprintType)
struct FQuest_RewardItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    EQuest_ItemRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString IconPath;

    FQuest_RewardItem()
    {
        ItemName = TEXT("");
        RewardType = EQuest_RewardType::Items;
        Rarity = EQuest_ItemRarity::Common;
        Quantity = 1;
        Description = TEXT("");
        IconPath = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FQuest_RewardBundle : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    FString BundleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    TArray<FQuest_RewardItem> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    int32 ReputationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    FString UnlockableContent;

    FQuest_RewardBundle()
    {
        BundleName = TEXT("");
        ExperiencePoints = 0;
        ReputationPoints = 0;
        UnlockableContent = TEXT("");
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardGranted, const FString&, RewardName, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceGained, int32, ExperiencePoints);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuest_RewardManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_RewardManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantReward(const FString& RewardBundleName, class AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantSingleReward(const FQuest_RewardItem& RewardItem, class AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantExperience(int32 ExperiencePoints, class AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantReputation(int32 ReputationPoints, class AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    TArray<FQuest_RewardItem> GetRewardPreview(const FString& RewardBundleName);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool HasRewardBundle(const FString& RewardBundleName) const;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnRewardGranted OnRewardGranted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnExperienceGained OnExperienceGained;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Data")
    class UDataTable* RewardDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Reward State")
    TMap<FString, int32> PlayerInventory;

    UPROPERTY(BlueprintReadOnly, Category = "Reward State")
    int32 PlayerExperience;

    UPROPERTY(BlueprintReadOnly, Category = "Reward State")
    int32 PlayerReputation;

private:
    void LoadRewardData();
    void AddItemToInventory(const FQuest_RewardItem& Item);
    void NotifyRewardGranted(const FQuest_RewardItem& Item);
    bool ValidateReward(const FQuest_RewardBundle& RewardBundle) const;
};