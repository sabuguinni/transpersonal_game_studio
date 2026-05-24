#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Quest_DynamicHuntingSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_HuntingMissionType : uint8
{
    TrackSmallPrey    UMETA(DisplayName = "Track Small Prey"),
    ObserveLargePredator UMETA(DisplayName = "Observe Large Predator"),
    StudyHerbivoreHerd UMETA(DisplayName = "Study Herbivore Herd"),
    AvoidPredatorTerritory UMETA(DisplayName = "Avoid Predator Territory"),
    GatherResourcesSafely UMETA(DisplayName = "Gather Resources Safely")
};

UENUM(BlueprintType)
enum class EQuest_HuntingObjectiveStatus : uint8
{
    NotStarted        UMETA(DisplayName = "Not Started"),
    InProgress        UMETA(DisplayName = "In Progress"),
    NearCompletion    UMETA(DisplayName = "Near Completion"),
    Completed         UMETA(DisplayName = "Completed"),
    Failed            UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FQuest_HuntingMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_HuntingMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_HuntingObjectiveStatus Status;

    FQuest_HuntingMissionData()
    {
        MissionType = EQuest_HuntingMissionType::TrackSmallPrey;
        MissionName = TEXT("Default Mission");
        MissionDescription = TEXT("Default mission description");
        TargetLocation = FVector::ZeroVector;
        CompletionRadius = 500.0f;
        TimeLimit = 300.0f;
        ExperienceReward = 100;
        Status = EQuest_HuntingObjectiveStatus::NotStarted;
    }
};

USTRUCT(BlueprintType)
struct FQuest_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsHunting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsFeeding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ObservationDistance;

    FQuest_DinosaurBehaviorData()
    {
        DinosaurSpecies = TEXT("Unknown");
        LastKnownLocation = FVector::ZeroVector;
        AggressionLevel = 0.5f;
        TerritoryRadius = 1000.0f;
        bIsHunting = false;
        bIsFeeding = false;
        ObservationDistance = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DynamicHuntingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DynamicHuntingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Management
    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void StartHuntingMission(EQuest_HuntingMissionType MissionType, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void UpdateMissionProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void FailMission();

    // Dinosaur Behavior Tracking
    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void RegisterDinosaurBehavior(const FString& Species, FVector Location, float Aggression);

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void UpdateDinosaurBehavior(const FString& Species, FVector NewLocation, bool bHunting, bool bFeeding);

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    FQuest_DinosaurBehaviorData GetDinosaurBehaviorData(const FString& Species);

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    bool IsPlayerInDangerZone();

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    float GetDistanceToNearestPredator();

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    void TriggerPredatorWarning();

    // Mission Generation
    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    TArray<FQuest_HuntingMissionData> GenerateAvailableMissions();

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    FQuest_HuntingMissionData CreateTrackingMission(FVector DinosaurLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunting System")
    FQuest_HuntingMissionData CreateObservationMission(const FString& DinosaurSpecies);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    FQuest_HuntingMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Data")
    TArray<FQuest_HuntingMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Data")
    TArray<FQuest_DinosaurBehaviorData> TrackedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MissionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DangerZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxTrackingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActiveMissions;

private:
    FTimerHandle MissionUpdateTimer;
    float MissionStartTime;
    bool bMissionActive;

    void InitializeDefaultMissions();
    void CheckMissionCompletion();
    void UpdateDinosaurTracking();
    bool IsLocationSafe(FVector Location);
    void NotifyMissionUpdate();
};