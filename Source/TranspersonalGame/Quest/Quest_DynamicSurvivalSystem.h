#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Engine/PointLight.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_DynamicSurvivalSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_SurvivalMissionType : uint8
{
    Exploration     UMETA(DisplayName = "Exploration"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Stealth         UMETA(DisplayName = "Stealth"),
    Combat          UMETA(DisplayName = "Combat"),
    Survival        UMETA(DisplayName = "Survival")
};

UENUM(BlueprintType)
enum class EQuest_MissionDifficulty : uint8
{
    Easy            UMETA(DisplayName = "Easy"),
    Medium          UMETA(DisplayName = "Medium"),
    Hard            UMETA(DisplayName = "Hard"),
    Extreme         UMETA(DisplayName = "Extreme")
};

UENUM(BlueprintType)
enum class EQuest_MissionStatus : uint8
{
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Locked          UMETA(DisplayName = "Locked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalMission
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_SurvivalMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_MissionStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FString> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsEmergencyMission;

    FQuest_SurvivalMission()
    {
        MissionName = TEXT("Unknown Mission");
        Description = TEXT("No description available");
        MissionType = EQuest_SurvivalMissionType::Survival;
        Difficulty = EQuest_MissionDifficulty::Easy;
        Status = EQuest_MissionStatus::Available;
        ExperienceReward = 0;
        TimeLimit = 300.0f; // 5 minutes default
        TargetLocation = FVector::ZeroVector;
        bIsEmergencyMission = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MissionObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 RequiredProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector ObjectiveLocation;

    FQuest_MissionObjective()
    {
        ObjectiveName = TEXT("Unknown Objective");
        Description = TEXT("No description");
        bIsCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
        ObjectiveLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DynamicSurvivalSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DynamicSurvivalSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Mission Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMission> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMission> ActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    TArray<FQuest_SurvivalMission> CompletedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission System")
    float MissionGenerationInterval;

    // Player Stats Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Monitoring")
    float PlayerHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Monitoring")
    float PlayerHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Monitoring")
    float PlayerThirstThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Monitoring")
    float PlayerFearThreshold;

    // Mission Generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Generation")
    bool bAutoGenerateMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Generation")
    float EmergencyMissionChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Generation")
    float DifficultyScalingFactor;

    // Visual Elements
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SystemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

public:
    // Mission Management Functions
    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void GenerateRandomMission();

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void StartMission(const FQuest_SurvivalMission& Mission);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void CompleteMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    void FailMission(const FString& MissionName);

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    bool CanStartMission(const FQuest_SurvivalMission& Mission) const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_SurvivalMission> GetAvailableMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Mission System")
    TArray<FQuest_SurvivalMission> GetActiveMissions() const;

    // Emergency Mission System
    UFUNCTION(BlueprintCallable, Category = "Emergency System")
    void TriggerEmergencyMission(EQuest_SurvivalMissionType EmergencyType);

    UFUNCTION(BlueprintCallable, Category = "Emergency System")
    void CheckPlayerConditions();

    // Mission Objective System
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    void UpdateObjectiveProgress(const FString& ObjectiveName, int32 ProgressIncrement);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    bool IsObjectiveCompleted(const FString& ObjectiveName) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetRandomMissionLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    EQuest_MissionDifficulty CalculateMissionDifficulty() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void UpdateMissionTimers(float DeltaTime);

protected:
    // Internal Functions
    void InitializeDefaultMissions();
    void CreateMissionVisualElements();
    FQuest_SurvivalMission CreateWaterMission();
    FQuest_SurvivalMission CreateFoodMission();
    FQuest_SurvivalMission CreateShelterMission();
    FQuest_SurvivalMission CreateStealthMission();
    FQuest_SurvivalMission CreateFireMission();

private:
    float MissionGenerationTimer;
    int32 MissionCounter;
    bool bSystemInitialized;
};