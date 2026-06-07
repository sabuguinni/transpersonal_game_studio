#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Quest_PrehistoricSurvivalMissionSystem.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCrowdSimulationManager;
class AQuestTrigger;

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Hunt_SmallPrey,
    Hunt_LargePredator,
    Gather_Resources,
    Craft_Tools,
    Build_Shelter,
    Explore_Territory,
    Escape_Danger,
    Defend_Camp,
    Track_Migration,
    Survive_Weather
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Novice,
    Apprentice,
    Experienced,
    Expert,
    Master
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    NotStarted,
    Active,
    Completed,
    Failed,
    Abandoned
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> RewardItems;

    FQuest_SurvivalMissionData()
    {
        MissionName = TEXT("Unnamed Mission");
        MissionDescription = TEXT("No description available");
        MissionType = EQuest_SurvivalMissionType::Gather_Resources;
        Difficulty = EQuest_MissionDifficulty::Novice;
        Status = EQuest_MissionStatus::NotStarted;
        TargetLocation = FVector::ZeroVector;
        TimeLimit = 300.0f; // 5 minutes default
        ExperienceReward = 100;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PlayerSurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HungerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float ThirstLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    int32 SurvivalLevel;

    FQuest_PlayerSurvivalStats()
    {
        HungerLevel = 100.0f;
        ThirstLevel = 100.0f;
        HealthLevel = 100.0f;
        StaminaLevel = 100.0f;
        FearLevel = 0.0f;
        ExperiencePoints = 0;
        SurvivalLevel = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_PrehistoricSurvivalMissionSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_PrehistoricSurvivalMissionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartMission(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AbandonMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalMissionData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_SurvivalMissionData GenerateRandomMission();

    // Survival Integration
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdatePlayerSurvivalStats(const FQuest_PlayerSurvivalStats& NewStats);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FQuest_PlayerSurvivalStats GetPlayerSurvivalStats() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool CheckSurvivalConditions() const;

    // Environmental Integration
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OnEnvironmentalHazardDetected(const FVector& HazardLocation, float HazardRadius);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OnDinosaurEncounterDetected(const FVector& DinosaurLocation, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void OnResourceDiscovered(const FVector& ResourceLocation, const FString& ResourceType);

    // Crowd Integration
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void OnCrowdDensityChanged(float NewDensity, const FVector& CrowdCenter);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void OnCrowdMovementDetected(const FVector& MovementDirection, float MovementSpeed);

protected:
    // Core mission data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMissionData> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    // Player tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    ATranspersonalCharacter* PlayerCharacter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    FQuest_PlayerSurvivalStats CurrentSurvivalStats;

    // System integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    UCrowdSimulationManager* CrowdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<AQuestTrigger*> QuestTriggers;

    // Mission generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float DifficultyScalingFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateMissions;

    // Timer handles
    FTimerHandle MissionGenerationTimerHandle;
    FTimerHandle SurvivalUpdateTimerHandle;

private:
    // Internal mission management
    void GenerateNewMission();
    void UpdateMissionProgress(float DeltaTime);
    void CheckMissionCompletion();
    void UpdateSurvivalStats(float DeltaTime);
    
    // Mission type generators
    FQuest_SurvivalMissionData GenerateHuntMission(EQuest_MissionDifficulty Difficulty);
    FQuest_SurvivalMissionData GenerateGatherMission(EQuest_MissionDifficulty Difficulty);
    FQuest_SurvivalMissionData GenerateCraftMission(EQuest_MissionDifficulty Difficulty);
    FQuest_SurvivalMissionData GenerateExplorationMission(EQuest_MissionDifficulty Difficulty);
    FQuest_SurvivalMissionData GenerateSurvivalMission(EQuest_MissionDifficulty Difficulty);
    
    // Utility functions
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    EQuest_MissionDifficulty CalculatePlayerAppropriatedifficulty();
    void BroadcastMissionEvent(const FString& EventType, const FQuest_SurvivalMissionData& MissionData);
};