#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_SurvivalQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float RewardExperience;

    FQuest_SurvivalObjective()
    {
        ObjectiveDescription = TEXT("Survive the wilderness");
        QuestType = EQuestType::Survival;
        TargetValue = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        RewardExperience = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    FString ChallengeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    float CurrentTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    TArray<FQuest_SurvivalObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    FVector ChallengeLocation;

    FQuest_SurvivalChallenge()
    {
        ChallengeName = TEXT("Wilderness Survival");
        TimeLimit = 300.0f; // 5 minutes
        CurrentTime = 0.0f;
        bIsActive = false;
        ChallengeLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Survival Quest Management
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void StartSurvivalChallenge(const FString& ChallengeName, float TimeLimit, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void EndSurvivalChallenge(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void UpdateSurvivalProgress(const FString& ObjectiveType, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void AddSurvivalObjective(const FString& Description, int32 TargetValue, float RewardXP);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    bool CheckSurvivalCompletion();

    // Survival Monitoring
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void MonitorPlayerHealth(float CurrentHealth, float MaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void MonitorPlayerHunger(float CurrentHunger, float MaxHunger);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void MonitorPlayerThirst(float CurrentThirst, float MaxThirst);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void MonitorPlayerTemperature(float CurrentTemp, float OptimalTemp);

    // Quest Rewards
    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void GrantSurvivalRewards(float ExperiencePoints);

    UFUNCTION(BlueprintCallable, Category = "Survival Quest")
    void UnlockSurvivalSkill(const FString& SkillName);

    // Quest State
    UFUNCTION(BlueprintPure, Category = "Survival Quest")
    bool IsAnySurvivalChallengeActive() const;

    UFUNCTION(BlueprintPure, Category = "Survival Quest")
    float GetCurrentChallengeTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "Survival Quest")
    int32 GetCompletedObjectivesCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    TArray<FQuest_SurvivalChallenge> ActiveChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    TArray<FQuest_SurvivalObjective> GlobalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float HealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float HungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float ThirstThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float TemperatureRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    bool bAutoGenerateChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float ChallengeGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Quest")
    float LastChallengeTime;

private:
    void ProcessActiveChallenges(float DeltaTime);
    void CheckAutoGeneration(float DeltaTime);
    void GenerateRandomChallenge();
    bool ValidateSurvivalConditions();
    void UpdateObjectiveProgress(FQuest_SurvivalObjective& Objective, int32 Progress);
};