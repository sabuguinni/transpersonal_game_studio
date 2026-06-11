#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_RewardSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RewardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    FString RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    int32 RewardAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    FString RewardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    bool bIsRare;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward")
    float ExperienceValue;

    FQuest_RewardData()
    {
        RewardType = EQuest_RewardType::Experience;
        RewardID = TEXT("");
        RewardAmount = 1;
        RewardDescription = TEXT("");
        bIsRare = false;
        ExperienceValue = 10.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RewardBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward Bundle")
    FString BundleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward Bundle")
    TArray<FQuest_RewardData> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward Bundle")
    bool bRequiresAllObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Reward Bundle")
    float BonusMultiplier;

    FQuest_RewardBundle()
    {
        BundleID = TEXT("");
        bRequiresAllObjectives = true;
        BonusMultiplier = 1.0f;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_RewardSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_RewardSystem();

protected:
    virtual void BeginPlay() override;

public:
    // Reward Management
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void AddRewardBundle(const FQuest_RewardBundle& NewBundle);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void RemoveRewardBundle(const FString& BundleID);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle GetRewardBundle(const FString& BundleID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    TArray<FQuest_RewardBundle> GetAllRewardBundles() const;

    // Reward Processing
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void ProcessQuestRewards(const FString& QuestID, const FString& BundleID);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantReward(const FQuest_RewardData& Reward);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantRewardBundle(const FQuest_RewardBundle& Bundle);

    // Experience and Skills
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantExperience(float ExperienceAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantSkillPoints(const FString& SkillType, int32 Points);

    // Items and Resources
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantItem(const FString& ItemID, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantResource(const FString& ResourceType, int32 Amount);

    // Knowledge and Recipes
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void UnlockRecipe(const FString& RecipeID);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void UnlockKnowledge(const FString& KnowledgeID);

    // Territory and Access
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void UnlockArea(const FString& AreaID);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantSafeZone(const FVector& Location, float Radius);

    // Reward Validation
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool CanGrantReward(const FQuest_RewardData& Reward) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool HasInventorySpace(const FString& ItemID, int32 Quantity) const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardGranted, const FQuest_RewardData&, Reward);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnRewardGranted OnRewardGranted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExperienceGranted, float, ExperienceAmount);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnExperienceGranted OnExperienceGranted;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKnowledgeUnlocked, const FString&, KnowledgeID);
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnKnowledgeUnlocked OnKnowledgeUnlocked;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Rewards")
    TArray<FQuest_RewardBundle> RewardBundles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Rewards")
    TArray<FString> UnlockedRecipes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Rewards")
    TArray<FString> UnlockedKnowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Rewards")
    TArray<FString> UnlockedAreas;

    // Reward settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float BaseExperienceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float RareRewardBonusMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bAutoGrantRewards;

private:
    void ProcessRewardByType(const FQuest_RewardData& Reward);
    void BroadcastRewardEvents(const FQuest_RewardData& Reward);
    float CalculateRewardMultiplier(const FQuest_RewardData& Reward) const;
};