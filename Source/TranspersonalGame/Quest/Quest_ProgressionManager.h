#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_ProgressionManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ProgressionTier : uint8
{
    Tier1_Beginner     UMETA(DisplayName = "Tier 1: Beginner"),
    Tier2_Novice       UMETA(DisplayName = "Tier 2: Novice"),
    Tier3_Intermediate UMETA(DisplayName = "Tier 3: Intermediate"),
    Tier4_Advanced     UMETA(DisplayName = "Tier 4: Advanced"),
    Tier5_Expert       UMETA(DisplayName = "Tier 5: Expert")
};

UENUM(BlueprintType)
enum class EQuest_CompletionStatus : uint8
{
    NotStarted    UMETA(DisplayName = "Not Started"),
    InProgress    UMETA(DisplayName = "In Progress"),
    Completed     UMETA(DisplayName = "Completed"),
    Failed        UMETA(DisplayName = "Failed"),
    Abandoned     UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ProgressionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    EQuest_ProgressionTier CurrentTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    EQuest_CompletionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    float ProgressPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    int32 CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    int32 TotalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    float TimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FString> UnlockedRewards;

    FQuest_ProgressionData()
    {
        QuestID = TEXT("");
        CurrentTier = EQuest_ProgressionTier::Tier1_Beginner;
        Status = EQuest_CompletionStatus::NotStarted;
        ProgressPercentage = 0.0f;
        CompletedObjectives = 0;
        TotalObjectives = 1;
        TimeRemaining = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TierRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier Requirements")
    EQuest_ProgressionTier RequiredTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier Requirements")
    int32 MinimumLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier Requirements")
    TArray<FString> PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier Requirements")
    float MinimumSurvivalRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier Requirements")
    TArray<FString> RequiredSkills;

    FQuest_TierRequirements()
    {
        RequiredTier = EQuest_ProgressionTier::Tier1_Beginner;
        MinimumLevel = 1;
        MinimumSurvivalRating = 0.0f;
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ProgressionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ProgressionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Progression Management
    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool StartQuest(const FString& QuestID, EQuest_ProgressionTier StartingTier);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool AdvanceToNextTier(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool CanAccessTier(EQuest_ProgressionTier TargetTier) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    float CalculateQuestDifficulty(EQuest_ProgressionTier Tier, const FString& QuestType) const;

    // Quest State Queries
    UFUNCTION(BlueprintPure, Category = "Quest Progression")
    FQuest_ProgressionData GetQuestProgress(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest Progression")
    TArray<FString> GetAvailableQuests(EQuest_ProgressionTier Tier) const;

    UFUNCTION(BlueprintPure, Category = "Quest Progression")
    EQuest_ProgressionTier GetPlayerTier() const { return CurrentPlayerTier; }

    UFUNCTION(BlueprintPure, Category = "Quest Progression")
    int32 GetCompletedQuestCount() const;

    // Tier Management
    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void UnlockNextTier();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool ValidateTierRequirements(EQuest_ProgressionTier TargetTier) const;

    // Reward System
    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void GrantQuestRewards(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    TArray<FString> GetTierRewards(EQuest_ProgressionTier Tier) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    EQuest_ProgressionTier CurrentPlayerTier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TMap<FString, FQuest_ProgressionData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TMap<EQuest_ProgressionTier, FQuest_TierRequirements> TierRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    float PlayerSurvivalRating;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    int32 PlayerLevel;

    // Progression Tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    float TierProgressionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    bool bAutoAdvanceTiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    float QuestTimeoutDuration;

private:
    void InitializeTierRequirements();
    void UpdatePlayerRating();
    bool CheckPrerequisites(const FString& QuestID) const;
    void ProcessTierAdvancement();
    void HandleQuestTimeout(const FString& QuestID);
};