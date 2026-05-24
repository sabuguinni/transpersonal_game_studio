#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Combat_AdvancedThreatEscalation.generated.h"

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None = 0,
    Low = 1,
    Medium = 2,
    High = 3,
    Critical = 4,
    Extreme = 5
};

UENUM(BlueprintType)
enum class ECombat_EscalationTrigger : uint8
{
    TimeBasedEscalation,
    PlayerProximity,
    NoiseLevel,
    BloodScent,
    TerritorialInvasion,
    PackDistress,
    AlphaChallenge
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEscalationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    float EscalationTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    float MaxEscalationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    TArray<ECombat_EscalationTrigger> ActiveTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    float ThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    int32 PredatorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Escalation")
    bool bPackCoordination;

    FCombat_ThreatEscalationData()
    {
        CurrentThreatLevel = ECombat_ThreatLevel::None;
        EscalationTimer = 0.0f;
        MaxEscalationTime = 300.0f; // 5 minutes max escalation
        ThreatRadius = 2000.0f;
        PredatorCount = 0;
        bPackCoordination = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_EscalationResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    ECombat_ThreatLevel TriggerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    int32 AdditionalPredators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    float AggressionMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    float HuntingRangeIncrease;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    bool bEnablePackTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Response")
    bool bEnableAmbushBehavior;

    FCombat_EscalationResponse()
    {
        TriggerLevel = ECombat_ThreatLevel::Low;
        AdditionalPredators = 0;
        AggressionMultiplier = 1.0f;
        HuntingRangeIncrease = 0.0f;
        bEnablePackTactics = false;
        bEnableAmbushBehavior = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedThreatEscalation : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedThreatEscalation();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core escalation system
    UFUNCTION(BlueprintCallable, Category = "Threat Escalation")
    void InitializeEscalationSystem();

    UFUNCTION(BlueprintCallable, Category = "Threat Escalation")
    void UpdateThreatLevel(ECombat_EscalationTrigger Trigger, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Threat Escalation")
    ECombat_ThreatLevel GetCurrentThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Threat Escalation")
    void EscalateThreat(ECombat_EscalationTrigger Trigger);

    UFUNCTION(BlueprintCallable, Category = "Threat Escalation")
    void DeescalateThreat(float DeescalationRate = 1.0f);

    // Advanced escalation mechanics
    UFUNCTION(BlueprintCallable, Category = "Advanced Escalation")
    void TriggerPackCoordination();

    UFUNCTION(BlueprintCallable, Category = "Advanced Escalation")
    void InitiateAmbushProtocol();

    UFUNCTION(BlueprintCallable, Category = "Advanced Escalation")
    void ActivateTerritorialDefense();

    UFUNCTION(BlueprintCallable, Category = "Advanced Escalation")
    void ProcessPlayerActions(const FVector& PlayerLocation, float NoiseLevel);

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    float CalculateThreatIntensity(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    bool ShouldEscalate(ECombat_EscalationTrigger Trigger, float CurrentIntensity);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    TArray<AActor*> GetNearbyPredators(float SearchRadius);

    // Response coordination
    UFUNCTION(BlueprintCallable, Category = "Response Coordination")
    void CoordinateEscalationResponse(ECombat_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Response Coordination")
    void NotifyPredatorsOfEscalation(ECombat_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Response Coordination")
    void UpdatePackBehavior(bool bEnableCoordination);

protected:
    // Escalation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Settings")
    FCombat_ThreatEscalationData EscalationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Settings")
    TArray<FCombat_EscalationResponse> EscalationResponses;

    // Escalation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Parameters")
    float BaseEscalationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Parameters")
    float ProximityEscalationMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Parameters")
    float NoiseEscalationMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Parameters")
    float TimeBasedEscalationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escalation Parameters")
    float DeescalationRate;

    // Advanced settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
    bool bEnableTimeBasedEscalation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
    bool bEnableProximityEscalation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
    bool bEnablePackCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
    float MaxThreatRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
    int32 MaxSimultaneousPredators;

private:
    // Internal state
    FTimerHandle EscalationTimerHandle;
    float LastEscalationTime;
    TArray<TWeakObjectPtr<AActor>> TrackedPredators;
    
    // Internal methods
    void ProcessEscalationTimer();
    void UpdateEscalationData(float DeltaTime);
    FCombat_EscalationResponse GetEscalationResponse(ECombat_ThreatLevel Level);
    void ApplyEscalationEffects(const FCombat_EscalationResponse& Response);
};