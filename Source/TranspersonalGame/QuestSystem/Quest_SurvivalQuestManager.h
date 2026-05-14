#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalQuestManager.generated.h"

// Quest objective types for survival gameplay
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt_Dinosaur       UMETA(DisplayName = "Hunt Specific Dinosaur"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Explore_Area        UMETA(DisplayName = "Explore Territory"),
    Craft_Tool          UMETA(DisplayName = "Craft Tool/Weapon"),
    Survive_Duration    UMETA(DisplayName = "Survive Time Period"),
    Defend_Location     UMETA(DisplayName = "Defend Camp/Area"),
    Rescue_NPC          UMETA(DisplayName = "Rescue Tribe Member"),
    Track_Migration     UMETA(DisplayName = "Follow Herd Migration"),
    Build_Shelter       UMETA(DisplayName = "Construct Shelter"),
    Tame_Creature       UMETA(DisplayName = "Domesticate Animal")
};

// Quest difficulty and reward tiers
UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Novice      UMETA(DisplayName = "Novice Hunter"),
    Experienced UMETA(DisplayName = "Experienced Survivor"),
    Expert      UMETA(DisplayName = "Master Tracker"),
    Legendary   UMETA(DisplayName = "Apex Predator")
};

// Quest status tracking
UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

// Individual quest objective data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString TargetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    FQuest_ObjectiveData()
    {
        ObjectiveID = TEXT("");
        ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
        Description = TEXT("");
        TargetName = TEXT("");
        RequiredQuantity = 1;
        CurrentProgress = 0;
        TargetLocation = FVector::ZeroVector;
        SearchRadius = 1000.0f;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

// Complete quest definition
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    FQuest_SurvivalQuest()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        Difficulty = EQuest_Difficulty::Novice;
        Status = EQuest_Status::Available;
        QuestGiver = TEXT("");
        TimeLimit = 0.0f;
        ElapsedTime = 0.0f;
        ExperienceReward = 100;
        QuestLocation = FVector::ZeroVector;
        bIsMainQuest = false;
        bIsRepeatable = false;
    }
};

// Quest reward data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_RewardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FString> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Experience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString UnlockedRecipe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString UnlockedArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    bool bUnlocksNewQuests;

    FQuest_RewardData()
    {
        Experience = 0;
        UnlockedRecipe = TEXT("");
        UnlockedArea = TEXT("");
        bUnlocksNewQuests = false;
    }
};

/**
 * Main quest management system for prehistoric survival gameplay
 * Handles quest creation, tracking, completion, and integration with survival mechanics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalQuestManager();

    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CreateQuest(const FQuest_SurvivalQuest& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    // Objective tracking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest queries
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalQuest> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GetQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool HasQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    // Survival integration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnAreaExplored(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCrafted(const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnShelterBuilt(const FVector& Location);

    // Quest generation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GenerateHuntQuest(const FString& DinosaurType, const FVector& HuntingGrounds);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GenerateGatherQuest(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GenerateExplorationQuest(const FVector& TargetArea, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GenerateCraftingQuest(const FString& ItemToCraft);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalQuest GenerateSurvivalQuest(float Duration);

    // Time management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestTimers(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestTimeouts();

    // Reward system
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_RewardData CalculateQuestRewards(const FQuest_SurvivalQuest& Quest) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void GrantQuestRewards(const FQuest_RewardData& Rewards);

    // Debug and testing
    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void CreateTestQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void LogQuestStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void ClearAllQuests();

protected:
    // Quest storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_SurvivalQuest> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DefaultQuestTimeout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float QuestGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> AvailableDinosaurTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FString> AvailableResourceTypes;

    // Internal tracking
    UPROPERTY()
    float LastQuestGenerationTime;

    UPROPERTY()
    int32 NextQuestID;

private:
    // Helper functions
    FString GenerateUniqueQuestID();
    bool ValidateQuestData(const FQuest_SurvivalQuest& Quest) const;
    void NotifyQuestStatusChange(const FString& QuestID, EQuest_Status NewStatus);
    float CalculateQuestDifficulty(const FQuest_SurvivalQuest& Quest) const;
    void UpdateQuestObjectives(FQuest_SurvivalQuest& Quest);
    bool CheckQuestPrerequisites(const FQuest_SurvivalQuest& Quest) const;
};