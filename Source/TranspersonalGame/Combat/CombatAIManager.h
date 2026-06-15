#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "CombatAIManager.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha Leader"),
    Hunter      UMETA(DisplayName = "Pack Hunter"),
    Ambusher    UMETA(DisplayName = "Ambush Specialist"),
    Defender    UMETA(DisplayName = "Territory Defender"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Patrol      UMETA(DisplayName = "Patrolling"),
    Hunt        UMETA(DisplayName = "Hunting"),
    Attack      UMETA(DisplayName = "Attacking"),
    Defend      UMETA(DisplayName = "Defending"),
    Retreat     UMETA(DisplayName = "Retreating"),
    Coordinated UMETA(DisplayName = "Pack Coordination")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_TacticalRole Role = ECombat_TacticalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolRadius = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 PackID = 0;

    FCombat_TacticalBehavior()
    {
        Role = ECombat_TacticalRole::Hunter;
        AggressionLevel = 0.7f;
        AttackRange = 400.0f;
        PatrolRadius = 250.0f;
        bIsPackLeader = false;
        PackID = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    AActor* PackLeader = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    FVector RallyPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    ECombat_AIState PackState = ECombat_AIState::Patrol;

    FCombat_PackCoordination()
    {
        PackMembers.Empty();
        PackLeader = nullptr;
        CurrentTarget = nullptr;
        RallyPoint = FVector::ZeroVector;
        PackState = ECombat_AIState::Patrol;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalRole(ECombat_TacticalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCurrentTarget() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(int32 PackID, AActor* Leader = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void ExecuteFlankingManeuver(FVector TargetLocation);

    // Tactical Behaviors
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteHuntingPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAmbushTactic();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteDefensivePosture();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* PotentialThreat) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> DetectNearbyThreats(float DetectionRadius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalAttackPosition(AActor* Target) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    FCombat_TacticalBehavior TacticalBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    ECombat_AIState CurrentAIState = ECombat_AIState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float DetectionRange = 800.0f;

private:
    // Pack coordination data
    FCombat_PackCoordination PackData;
    
    // Internal state tracking
    float StateChangeTimer = 0.0f;
    FVector LastKnownTargetLocation = FVector::ZeroVector;
    bool bIsInCombat = false;
    
    // Helper functions
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntBehavior(float DeltaTime);
    void UpdateAttackBehavior(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    bool IsTargetInRange(AActor* Target, float Range) const;
    FVector GetNextPatrolPoint() const;
    void BroadcastPackCommand(const FString& Command) const;
};

#include "CombatAIManager.generated.h"