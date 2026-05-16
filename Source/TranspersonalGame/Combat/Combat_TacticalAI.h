#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

class UBlackboardComponent;
class UAIPerceptionComponent;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol         UMETA(DisplayName = "Patrol"),
    Hunt           UMETA(DisplayName = "Hunt"),
    Engage         UMETA(DisplayName = "Engage"),
    Retreat        UMETA(DisplayName = "Retreat"),
    Ambush         UMETA(DisplayName = "Ambush"),
    PackCoordinate UMETA(DisplayName = "Pack Coordinate"),
    Territorial    UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector ThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPlayerThreat;

    FCombat_ThreatAssessment()
    {
        ThreatLevel = 0.0f;
        ThreatLocation = FVector::ZeroVector;
        ThreatActor = nullptr;
        LastSeenTime = 0.0f;
        bIsPlayerThreat = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<APawn*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    APawn* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector RallyPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_TacticalState PackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float PackRadius;

    FCombat_PackCoordination()
    {
        PackLeader = nullptr;
        RallyPoint = FVector::ZeroVector;
        PackState = ECombat_TacticalState::Patrol;
        PackRadius = 1000.0f;
    }
};

/**
 * Advanced tactical AI component for dinosaur combat behavior
 * Handles threat assessment, pack coordination, and tactical decision making
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core tactical AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_TacticalState GetTacticalState() const { return CurrentTacticalState; }

    // Pack coordination functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(APawn* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    bool IsPackLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void BroadcastPackSignal(ECombat_TacticalState Signal);

protected:
    // Tactical state management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TacticalUpdateInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastTacticalUpdate;

    // Threat assessment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_ThreatAssessment> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatMemoryDuration;

    // Pack coordination
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    bool bCanJoinPack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    int32 MaxPackSize;

    // Combat parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AmbushDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bPreferAmbushTactics;

    // AI References
    UPROPERTY()
    UBlackboardComponent* BlackboardComp;

    UPROPERTY()
    UAIPerceptionComponent* PerceptionComp;

private:
    void UpdateThreatAssessment(float DeltaTime);
    void CleanupOldThreats(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    FVector FindCoverPosition(FVector FromLocation);
    bool IsPositionSafe(FVector Position);
    void ExecuteFlankingManeuver(AActor* Target);
    void ExecuteAmbushTactic(AActor* Target);
};