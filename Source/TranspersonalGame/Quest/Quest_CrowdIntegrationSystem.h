#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_CrowdIntegrationSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    HerdObservation     UMETA(DisplayName = "Herd Observation"),
    PredatorTracking    UMETA(DisplayName = "Predator Tracking"),
    MigrationMapping    UMETA(DisplayName = "Migration Mapping"),
    TerritorialStudy    UMETA(DisplayName = "Territorial Study"),
    PackBehaviorAnalysis UMETA(DisplayName = "Pack Behavior Analysis"),
    CrowdAvoidance      UMETA(DisplayName = "Crowd Avoidance"),
    MassEncounter       UMETA(DisplayName = "Mass Encounter")
};

UENUM(BlueprintType)
enum class EQuest_ObservationStatus : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    InProgress          UMETA(DisplayName = "In Progress"),
    DataCollected       UMETA(DisplayName = "Data Collected"),
    AnalysisComplete    UMETA(DisplayName = "Analysis Complete"),
    ReportSubmitted     UMETA(DisplayName = "Report Submitted"),
    Completed           UMETA(DisplayName = "Completed"),
    Failed              UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdObservationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    EQuest_CrowdMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    FVector ObservationLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    float ObservationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    int32 RequiredObservationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    int32 MinimumCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    TArray<FString> BehaviorPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    EQuest_ObservationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Observation")
    float ProgressPercentage;

    FQuest_CrowdObservationData()
    {
        MissionType = EQuest_CrowdMissionType::HerdObservation;
        ObservationLocation = FVector::ZeroVector;
        ObservationRadius = 1000.0f;
        RequiredObservationTime = 300;
        MinimumCrowdSize = 10;
        Status = EQuest_ObservationStatus::NotStarted;
        ProgressPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdMissionReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Reward")
    TArray<FString> UnlockedKnowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Reward")
    TArray<FString> NewQuestLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Reward")
    bool bUnlocksNewArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Reward")
    FString RewardDescription;

    FQuest_CrowdMissionReward()
    {
        ExperiencePoints = 100;
        bUnlocksNewArea = false;
        RewardDescription = TEXT("Knowledge gained about prehistoric creatures");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdObservationMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdObservationMarker();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FQuest_CrowdObservationData ObservationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    FQuest_CrowdMissionReward MissionReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Marker")
    float DetectionRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void ActivateObservationMission();

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void UpdateObservationProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    bool IsPlayerInObservationRange(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Marker")
    void CompleteMission();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnMissionStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnMissionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnProgressUpdated(float NewProgress);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_CrowdIntegrationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_CrowdIntegrationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    TArray<FQuest_CrowdObservationData> ActiveObservationMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    TArray<class AQuest_CrowdObservationMarker*> ObservationMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    int32 MaxSimultaneousMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    float MissionGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Integration")
    float LastMissionGenerationTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    void InitializeCrowdMissions();

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    void GenerateNewObservationMission();

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    void UpdateActiveMissions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    bool StartObservationMission(EQuest_CrowdMissionType MissionType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    void CompleteObservationMission(int32 MissionIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    TArray<FQuest_CrowdObservationData> GetActiveMissions() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Integration")
    int32 GetCompletedMissionCount() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnNewMissionGenerated(const FQuest_CrowdObservationData& NewMission);

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Events")
    void OnMissionCompleted(const FQuest_CrowdObservationData& CompletedMission);

private:
    UPROPERTY()
    int32 CompletedMissionCount;

    UPROPERTY()
    class UWorld* CachedWorld;

    void SpawnObservationMarker(const FQuest_CrowdObservationData& MissionData);
    FVector FindSuitableObservationLocation(EQuest_CrowdMissionType MissionType);
    bool ValidateObservationLocation(FVector Location, float Radius);
};