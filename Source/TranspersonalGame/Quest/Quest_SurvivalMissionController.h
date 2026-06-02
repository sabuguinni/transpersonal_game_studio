#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_SurvivalMissionController.generated.h"

// Forward declarations
class UQuestManager;
class ATranspersonalCharacter;

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    BasicSurvival       UMETA(DisplayName = "Basic Survival"),
    ResourceGathering   UMETA(DisplayName = "Resource Gathering"),
    ShelterBuilding     UMETA(DisplayName = "Shelter Building"),
    FoodHunting         UMETA(DisplayName = "Food Hunting"),
    WaterCollection     UMETA(DisplayName = "Water Collection"),
    ToolCrafting        UMETA(DisplayName = "Tool Crafting"),
    TerritoryDefense    UMETA(DisplayName = "Territory Defense"),
    WeatherSurvival     UMETA(DisplayName = "Weather Survival")
};

UENUM(BlueprintType)
enum class EQuest_SurvivalDifficulty : uint8
{
    Beginner    UMETA(DisplayName = "Beginner"),
    Novice      UMETA(DisplayName = "Novice"),
    Experienced UMETA(DisplayName = "Experienced"),
    Expert      UMETA(DisplayName = "Expert"),
    Master      UMETA(DisplayName = "Master")
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Objective")
    FVector TargetLocation;

    FQuest_SurvivalObjective()
    {
        ObjectiveDescription = TEXT("Survive the wilderness");
        MissionType = EQuest_SurvivalMissionType::BasicSurvival;
        RequiredAmount = 1;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FQuest_SurvivalReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Reward")
    int32 SurvivalSkillBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Reward")
    float HealthRestore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Reward")
    float StaminaBonus;

    FQuest_SurvivalReward()
    {
        ExperiencePoints = 100;
        SurvivalSkillBonus = 10;
        HealthRestore = 25.0f;
        StaminaBonus = 15.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_SurvivalMissionController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_SurvivalMissionController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core survival mission properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    EQuest_SurvivalMissionType CurrentMissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    EQuest_SurvivalDifficulty MissionDifficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    TArray<FQuest_SurvivalObjective> SurvivalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    FQuest_SurvivalReward MissionReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    float MissionTimeRemaining;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsMissionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Mission")
    bool bIsMissionCompleted;

    // Player survival tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float PlayerHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float PlayerHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float PlayerThirstThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Tracking")
    float PlayerStaminaThreshold;

    // Mission generation and management
    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void GenerateSurvivalMission(EQuest_SurvivalMissionType MissionType, EQuest_SurvivalDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void StartSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CompleteSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void FailSurvivalMission();

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void UpdateObjectiveProgress(int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    bool CheckObjectiveCompletion(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Survival Mission")
    void CheckAllObjectivesCompletion();

    // Player survival state monitoring
    UFUNCTION(BlueprintCallable, Category = "Player Monitoring")
    void MonitorPlayerSurvivalState();

    UFUNCTION(BlueprintCallable, Category = "Player Monitoring")
    bool IsPlayerInDanger();

    UFUNCTION(BlueprintCallable, Category = "Player Monitoring")
    void TriggerEmergencySurvivalMission();

    // Mission type specific generators
    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateResourceGatheringMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateShelterBuildingMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateFoodHuntingMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateWaterCollectionMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateToolCraftingMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateTerritoryDefenseMission();

    UFUNCTION(BlueprintCallable, Category = "Mission Generation")
    void GenerateWeatherSurvivalMission();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetNearbyResourceLocation(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float CalculateMissionDifficultyMultiplier();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void AdjustRewardsForDifficulty();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetMissionDescription();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintMissionStatus();

protected:
    // Internal helper functions
    void InitializeMissionDefaults();
    void SetupObjectiveLocations();
    void CalculateTimeRequirements();
    bool ValidatePlayerState();
    void NotifyPlayerOfMissionUpdate();

private:
    // Internal state tracking
    float LastPlayerHealthCheck;
    float LastObjectiveCheck;
    int32 CompletedObjectivesCount;
    bool bPlayerInEmergencyState;

    // Mission generation helpers
    TArray<FVector> GetResourceSpawnLocations();
    TArray<FVector> GetSafeZoneLocations();
    TArray<FVector> GetDangerousAreaLocations();
};