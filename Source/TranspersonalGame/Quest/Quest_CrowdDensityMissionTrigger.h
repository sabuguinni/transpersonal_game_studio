#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "Components/BoxComponent.h"
#include "Quest_CrowdDensityMissionTrigger.generated.h"

class UCrowdSimulationManager;
class UQuestManager;

UENUM(BlueprintType)
enum class EQuest_CrowdDensityTriggerType : uint8
{
    HighDensity     UMETA(DisplayName = "High Density"),
    LowDensity      UMETA(DisplayName = "Low Density"), 
    FlowChange      UMETA(DisplayName = "Flow Change"),
    Stampede        UMETA(DisplayName = "Stampede"),
    Gathering       UMETA(DisplayName = "Gathering")
};

USTRUCT(BlueprintType)
struct FQuest_CrowdDensityMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdDensityTriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float DensityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeRequirement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RewardPoints;

    FQuest_CrowdDensityMissionData()
        : MissionName(TEXT("Crowd Mission"))
        , MissionDescription(TEXT("Navigate crowd challenges"))
        , TriggerType(EQuest_CrowdDensityTriggerType::HighDensity)
        , DensityThreshold(100.0f)
        , TimeRequirement(60.0f)
        , RewardPoints(50)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdDensityMissionTrigger : public ATriggerVolume
{
    GENERATED_BODY()

public:
    AQuest_CrowdDensityMissionTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // Mission Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    FQuest_CrowdDensityMissionData MissionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    float MonitoringRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Mission")
    float UpdateInterval;

    // Crowd Density Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    float CurrentCrowdDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    int32 EntitiesInRange;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    float AverageMovementSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd State")
    bool bStampedeDetected;

    // Mission Progress
    UPROPERTY(BlueprintReadOnly, Category = "Mission Progress")
    bool bMissionTriggered;

    UPROPERTY(BlueprintReadOnly, Category = "Mission Progress")
    float MissionTimer;

    UPROPERTY(BlueprintReadOnly, Category = "Mission Progress")
    bool bPlayerInTrigger;

    UPROPERTY(BlueprintReadOnly, Category = "Mission Progress")
    float PlayerSurvivalTime;

    // Quest Integration
    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void StartMission();

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Quest Mission")
    void ResetMission();

    // Crowd Analysis
    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    float CalculateCrowdDensity();

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    bool DetectStampede();

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    FVector GetCrowdFlowDirection();

    UFUNCTION(BlueprintCallable, Category = "Crowd Analysis")
    bool IsSafeNavigationPath(const FVector& StartLocation, const FVector& EndLocation);

private:
    // Component References
    UPROPERTY()
    UCrowdSimulationManager* CrowdManager;

    UPROPERTY()
    UQuestManager* QuestManager;

    // Internal State
    float LastUpdateTime;
    TArray<AActor*> NearbyEntities;
    FVector LastCrowdFlowDirection;
    bool bStampedeWarningIssued;

    // Mission Logic
    void UpdateCrowdMonitoring();
    void CheckMissionConditions();
    void ProcessPlayerInTrigger();
    void HandleStampedeEvent();
    void UpdateMissionProgress(float DeltaTime);
    void BroadcastMissionUpdate();
};