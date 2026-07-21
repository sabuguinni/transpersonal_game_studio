#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "../Core/SharedTypes.h"
#include "Quest_SurvivalChallengeManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_ChallengeType : uint8
{
    None            UMETA(DisplayName = "None"),
    HungerSurvival  UMETA(DisplayName = "Hunger Survival"),
    ThirstSurvival  UMETA(DisplayName = "Thirst Survival"),
    ColdSurvival    UMETA(DisplayName = "Cold Survival"),
    PredatorEscape  UMETA(DisplayName = "Predator Escape"),
    ResourceGather  UMETA(DisplayName = "Resource Gathering"),
    ShelterBuild    UMETA(DisplayName = "Shelter Building"),
    FireMaintain    UMETA(DisplayName = "Fire Maintenance"),
    WaterFind       UMETA(DisplayName = "Water Finding"),
    FoodHunt        UMETA(DisplayName = "Food Hunting"),
    ToolCraft       UMETA(DisplayName = "Tool Crafting")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Beginner    UMETA(DisplayName = "Beginner"),
    Novice      UMETA(DisplayName = "Novice"),
    Experienced UMETA(DisplayName = "Experienced"),
    Expert      UMETA(DisplayName = "Expert"),
    Master      UMETA(DisplayName = "Master"),
    Legendary   UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    FString ChallengeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    EQuest_ChallengeType ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
    int32 CurrentProgress;

    FQuest_SurvivalChallenge()
    {
        ChallengeName = TEXT("Unnamed Challenge");
        Description = TEXT("Survive the prehistoric wilderness");
        ChallengeType = EQuest_ChallengeType::None;
        Difficulty = EQuest_Difficulty::Beginner;
        TimeLimit = 300.0f;
        RequiredAmount = 1;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 1000.0f;
        bIsActive = false;
        bIsCompleted = false;
        StartTime = 0.0f;
        CurrentProgress = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeStarted, const FQuest_SurvivalChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FQuest_SurvivalChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeFailed, const FQuest_SurvivalChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChallengeProgress, const FQuest_SurvivalChallenge&, Challenge, float, ProgressPercent);

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalChallengeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalChallengeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Challenge Management
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void StartChallenge(EQuest_ChallengeType ChallengeType, EQuest_Difficulty Difficulty = EQuest_Difficulty::Beginner);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void CompleteCurrentChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void FailCurrentChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void CancelCurrentChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    bool HasActiveChallenge() const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge GetCurrentChallenge() const;

    // Progress Tracking
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void UpdateChallengeProgress(int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    float GetChallengeProgressPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    float GetRemainingTime() const;

    // Challenge Creation
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge CreateHungerChallenge(EQuest_Difficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge CreateThirstChallenge(EQuest_Difficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge CreateResourceGatherChallenge(EQuest_Difficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge CreateShelterBuildChallenge(EQuest_Difficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    FQuest_SurvivalChallenge CreatePredatorEscapeChallenge(EQuest_Difficulty Difficulty);

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void CheckLocationObjective(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void RegisterResourceCollection(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void RegisterCraftingActivity(const FString& ItemCrafted);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void RegisterShelterBuilding();

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void RegisterPredatorEncounter();

    // Biome-Specific Challenges
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void StartBiomeChallenge(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    TArray<FQuest_SurvivalChallenge> GetAvailableChallengesForBiome(EBiomeType BiomeType);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnChallengeStarted OnChallengeStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnChallengeCompleted OnChallengeCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnChallengeFailed OnChallengeFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnChallengeProgress OnChallengeProgress;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    FQuest_SurvivalChallenge CurrentChallenge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    TArray<FQuest_SurvivalChallenge> CompletedChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    bool bAutoStartNextChallenge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float ChallengeCheckInterval;

    // Timer Handles
    FTimerHandle ChallengeTimerHandle;
    FTimerHandle ProgressCheckHandle;

    // Internal Methods
    void CheckChallengeCompletion();
    void CheckChallengeTimeout();
    void InitializeChallengeTimer();
    void CleanupChallengeTimer();
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);
    float GetDifficultyMultiplier(EQuest_Difficulty Difficulty);
};