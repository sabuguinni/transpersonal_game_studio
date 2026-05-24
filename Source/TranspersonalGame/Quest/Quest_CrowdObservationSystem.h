#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "Quest_CrowdObservationSystem.generated.h"

// Forward declarations
class UCrowdSimulationManager;
class UHerdBehaviorManager;
class AQuestTrigger;

UENUM(BlueprintType)
enum class EQuest_ObservationType : uint8
{
    HerbivoreFeedingBehavior,
    PredatorHuntingPattern,
    MigrationMovement,
    TerritorialBehavior,
    PackCoordination,
    FlockingBehavior
};

USTRUCT(BlueprintType)
struct FQuest_ObservationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EQuest_ObservationType ObservationType;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 RequiredObservationTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 MinimumDinosaursInArea;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float ObservationRadius;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bRequiresStealthMode;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString ObservationDescription;

    FQuest_ObservationData()
    {
        ObservationType = EQuest_ObservationType::HerbivoreFeedingBehavior;
        RequiredObservationTime = 30;
        MinimumDinosaursInArea = 3;
        ObservationRadius = 1000.0f;
        bRequiresStealthMode = false;
        ObservationDescription = TEXT("Observe dinosaur behavior");
    }
};

USTRUCT(BlueprintType)
struct FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString MissionName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString MissionDescription;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<FQuest_ObservationData> RequiredObservations;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 RewardExperience;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float MissionTimeLimit;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsRepeatable;

    FQuest_CrowdMissionData()
    {
        MissionName = TEXT("Study Dinosaur Behavior");
        MissionDescription = TEXT("Observe and document dinosaur crowd behavior patterns");
        RewardExperience = 100;
        MissionTimeLimit = 300.0f;
        bIsRepeatable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdObservationSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdObservationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core observation system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    TArray<FQuest_CrowdMissionData> AvailableMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    FQuest_CrowdMissionData CurrentMission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    bool bMissionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    float CurrentObservationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    int32 CurrentObservationIndex;

    // Integration with crowd simulation
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Integration")
    TObjectPtr<UCrowdSimulationManager> CrowdManager;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Integration")
    TObjectPtr<UHerdBehaviorManager> HerdManager;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    TArray<TObjectPtr<AQuestTrigger>> ObservationTriggers;

public:
    // Mission management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartObservationMission(const FQuest_CrowdMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteCurrentMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CancelCurrentMission();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsPlayerInObservationRange(const FQuest_ObservationData& ObservationData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 CountDinosaursInArea(FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObservationProgress(float DeltaTime);

    // Crowd behavior analysis
    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    EQuest_ObservationType AnalyzeCrowdBehavior(FVector ObservationPoint);

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    bool ValidateObservationConditions(const FQuest_ObservationData& ObservationData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    FString GenerateBehaviorReport(EQuest_ObservationType BehaviorType);

    // Audio and feedback
    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Feedback")
    void PlayObservationStartAudio();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Feedback")
    void PlayObservationCompleteAudio();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Feedback")
    void PlayDangerWarningAudio();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Quest System")
    bool IsMissionActive() const { return bMissionActive; }

    UFUNCTION(BlueprintPure, Category = "Quest System")
    float GetMissionProgress() const;

    UFUNCTION(BlueprintPure, Category = "Quest System")
    FString GetCurrentObjectiveText() const;
};