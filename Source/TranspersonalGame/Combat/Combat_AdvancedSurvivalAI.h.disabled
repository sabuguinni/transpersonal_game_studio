#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightPerceptionComponent.h"
#include "Perception/AIHearingPerceptionComponent.h"
#include "SharedTypes.h"
#include "Combat_AdvancedSurvivalAI.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_SurvivalThreatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    float DetectionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat")
    ECombat_ThreatType ThreatType;

    FCombat_SurvivalThreatData()
    {
        ThreatActor = nullptr;
        ThreatLevel = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        DetectionTime = 0.0f;
        ThreatType = ECombat_ThreatType::Unknown;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_SurvivalResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    ECombat_SurvivalBehavior ResponseType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    float ResponsePriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    float ResponseDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    bool bRequiresPackCoordination;

    FCombat_SurvivalResponse()
    {
        ResponseType = ECombat_SurvivalBehavior::Patrol;
        TargetLocation = FVector::ZeroVector;
        ResponsePriority = 0.0f;
        ResponseDuration = 0.0f;
        bRequiresPackCoordination = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AdvancedSurvivalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AdvancedSurvivalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core survival AI functions
    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void InitializeSurvivalAI();

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void UpdateSurvivalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void ProcessThreatDetection();

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void EvaluateEnvironmentalHazards();

    UFUNCTION(BlueprintCallable, Category = "Survival AI")
    void CalculateSurvivalResponse();

    // Threat management
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel, ECombat_ThreatType ThreatType);

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void UpdateThreatData(AActor* ThreatActor, FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    FCombat_SurvivalThreatData GetHighestThreat() const;

    // Environmental awareness
    UFUNCTION(BlueprintCallable, Category = "Environmental AI")
    void ScanEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Environmental AI")
    void EvaluateTerrainAdvantage(FVector Location, float& AdvantageScore);

    UFUNCTION(BlueprintCallable, Category = "Environmental AI")
    void FindCoverPositions();

    UFUNCTION(BlueprintCallable, Category = "Environmental AI")
    void IdentifyEscapeRoutes();

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void InitializePackCoordination();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void UpdatePackFormation();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CommunicateWithPack(ECombat_PackSignal SignalType);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RespondToPackSignal(ECombat_PackSignal SignalType, AActor* Sender);

    // Survival behaviors
    UFUNCTION(BlueprintCallable, Category = "Survival Behavior")
    void ExecuteFlightResponse();

    UFUNCTION(BlueprintCallable, Category = "Survival Behavior")
    void ExecuteFightResponse();

    UFUNCTION(BlueprintCallable, Category = "Survival Behavior")
    void ExecuteHideResponse();

    UFUNCTION(BlueprintCallable, Category = "Survival Behavior")
    void ExecutePatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "Survival Behavior")
    void ExecuteHuntingBehavior();

protected:
    // Survival AI properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival AI", meta = (AllowPrivateAccess = "true"))
    float SurvivalInstinct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival AI", meta = (AllowPrivateAccess = "true"))
    float EnvironmentalAwareness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival AI", meta = (AllowPrivateAccess = "true"))
    float ThreatSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival AI", meta = (AllowPrivateAccess = "true"))
    float PackLoyalty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival AI", meta = (AllowPrivateAccess = "true"))
    float AdaptabilityFactor;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    ECombat_SurvivalBehavior CurrentBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    float CurrentStressLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    FVector SafeLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    bool bIsInDanger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State", meta = (AllowPrivateAccess = "true"))
    bool bHasPackSupport;

    // Threat tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threat Tracking", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_SurvivalThreatData> ActiveThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Tracking", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Threat Tracking", meta = (AllowPrivateAccess = "true"))
    float ThreatMemoryDuration;

    // Environmental data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> CoverPositions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> EscapeRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    float EnvironmentScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (AllowPrivateAccess = "true"))
    float TerrainAnalysisAccuracy;

    // Pack coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    float PackCommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination", meta = (AllowPrivateAccess = "true"))
    bool bIsPackLeader;

    // Response system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Response System", meta = (AllowPrivateAccess = "true"))
    FCombat_SurvivalResponse CurrentResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response System", meta = (AllowPrivateAccess = "true"))
    float ResponseUpdateInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Response System", meta = (AllowPrivateAccess = "true"))
    float LastResponseUpdate;

private:
    // Internal helper functions
    void UpdateThreatMemory(float DeltaTime);
    void CalculateStressLevel();
    void UpdatePackAwareness();
    void EvaluateResponseEffectiveness();
    float CalculateThreatPriority(const FCombat_SurvivalThreatData& ThreatData) const;
    FVector FindOptimalPosition(const TArray<FVector>& Candidates) const;
    bool IsLocationSafe(FVector Location) const;
    void BroadcastPackSignal(ECombat_PackSignal SignalType);
};