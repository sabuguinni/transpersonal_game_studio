#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Quest_PrehistoricQuestRewardSystem.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UInventoryComponent;

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    None = 0,
    Experience,
    Resources,
    Tools,
    Knowledge,
    Territory,
    Reputation,
    Skills,
    Crafting_Recipe
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RewardDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    EQuest_RewardType RewardType = EQuest_RewardType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float ExperiencePoints = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    bool bIsRare = false;

    FQuest_RewardDefinition()
    {
        RewardType = EQuest_RewardType::None;
        RewardID = "";
        Quantity = 1;
        ExperiencePoints = 0.0f;
        Description = "";
        bIsRare = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RewardBundle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    TArray<FQuest_RewardDefinition> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    float TotalExperience = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    FString BundleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Bundle")
    bool bAutoGrant = true;

    FQuest_RewardBundle()
    {
        TotalExperience = 0.0f;
        BundleName = "";
        bAutoGrant = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestRewardGranted, const FQuest_RewardBundle&, RewardBundle, ATranspersonalCharacter*, Player);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_PrehistoricQuestRewardSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_PrehistoricQuestRewardSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Reward granting system
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantReward(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantRewardBundle(ATranspersonalCharacter* Player, const FQuest_RewardBundle& Bundle);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle CreateBasicSurvivalReward(int32 ExperienceAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle CreateHuntingReward(const FString& PreyType, int32 PreySize);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle CreateCraftingReward(const FString& ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle CreateExplorationReward(const FString& LocationDiscovered);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    FQuest_RewardBundle CreateTribalReward(int32 TribeMembers);

    // Reward validation
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool CanGrantReward(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool ValidateRewardBundle(const FQuest_RewardBundle& Bundle);

    // Experience system
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void GrantExperience(ATranspersonalCharacter* Player, float Amount, const FString& Source);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    float CalculateExperienceBonus(ATranspersonalCharacter* Player, const FString& ActivityType);

    // Resource rewards
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantResourceReward(ATranspersonalCharacter* Player, const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantToolReward(ATranspersonalCharacter* Player, const FString& ToolType, int32 Quality);

    // Knowledge and skills
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantKnowledgeReward(ATranspersonalCharacter* Player, const FString& KnowledgeType);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantSkillImprovement(ATranspersonalCharacter* Player, const FString& SkillName, float Improvement);

    // Territory and reputation
    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    bool GrantTerritoryAccess(ATranspersonalCharacter* Player, const FString& TerritoryName);

    UFUNCTION(BlueprintCallable, Category = "Quest Rewards")
    void ModifyReputation(ATranspersonalCharacter* Player, const FString& Faction, float Change);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Rewards")
    FOnQuestRewardGranted OnRewardGranted;

protected:
    // Reward templates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Templates")
    TMap<FString, FQuest_RewardBundle> RewardTemplates;

    // Experience multipliers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
    TMap<FString, float> ExperienceMultipliers;

    // Resource limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<FString, int32> MaxResourceAmounts;

    // Skill caps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
    TMap<FString, float> SkillCaps;

    // Internal methods
    void InitializeRewardTemplates();
    void InitializeExperienceMultipliers();
    bool HasInventorySpace(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward);
    void LogRewardGrant(ATranspersonalCharacter* Player, const FQuest_RewardDefinition& Reward);
};