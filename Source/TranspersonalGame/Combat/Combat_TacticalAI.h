#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Hunting,
    Flanking,
    Attacking,
    Retreating,
    Regrouping
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Alpha,      // Pack leader, makes decisions
    Beta,       // Flanking specialists
    Gamma,      // Support and harassment
    Scout,      // Early warning and tracking
    Ambusher    // Hidden attack specialist
};

USTRUCT(BlueprintType)
struct FCombat_TacticalWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_TacticalState RequiredState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOccupied;

    FCombat_TacticalWaypoint()
    {
        Position = FVector::ZeroVector;
        RequiredState = ECombat_TacticalState::Patrol;
        Priority = 1.0f;
        bIsOccupied = false;
    }
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<APawn*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APawn* AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    APawn* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombat_TacticalState PackState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CoordinationRadius;

    FCombat_PackCoordination()
    {
        AlphaLeader = nullptr;
        CurrentTarget = nullptr;
        PackState = ECombat_TacticalState::Patrol;
        CoordinationRadius = 2000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical AI properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    ECombat_PackRole PackRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FlankingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    TArray<FCombat_TacticalWaypoint> TacticalWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Cunning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Patience;

    // Tactical AI functions
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    APawn* FindBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    FVector CalculateFlankingPosition(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UCombat_TacticalAI* AlphaAI);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void SignalPackMembers(ECombat_TacticalState NewPackState);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    FCombat_TacticalWaypoint FindNearestWaypoint(ECombat_TacticalState RequiredState);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    void OccupyWaypoint(int32 WaypointIndex);

    UFUNCTION(BlueprintCallable, Category = "Waypoints")
    void ReleaseWaypoint(int32 WaypointIndex);

private:
    // Internal tactical calculations
    void UpdateTacticalAssessment();
    void ExecuteCurrentTactic();
    bool CanSeeTarget(APawn* Target);
    float CalculateThreatLevel(APawn* Threat);
    
    // Pack coordination internals
    void UpdatePackCoordination();
    void AssignPackRoles();
    
    // Timers and state tracking
    float StateChangeTimer;
    float LastTargetScanTime;
    APawn* LastKnownTarget;
    FVector LastKnownTargetLocation;
    
    // AI decision making
    float DecisionCooldown;
    bool bInCombat;
    float CombatStartTime;
};