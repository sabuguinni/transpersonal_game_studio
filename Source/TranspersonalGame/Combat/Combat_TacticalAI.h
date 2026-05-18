#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_TacticalAI.generated.h"

// Forward declarations
class UCombat_DamageSystem;
class ANPC_DinosaurAI;

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Circling    UMETA(DisplayName = "Circling"),
    Ambush      UMETA(DisplayName = "Ambush")
};

UENUM(BlueprintType)
enum class ECombat_AttackPattern : uint8
{
    Direct      UMETA(DisplayName = "Direct Assault"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Ambush      UMETA(DisplayName = "Ambush"),
    PackHunt    UMETA(DisplayName = "Pack Hunt"),
    Retreat     UMETA(DisplayName = "Tactical Retreat"),
    Circle      UMETA(DisplayName = "Circle Strafe")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float CautiousLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float PackCoordination = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float FleeHealthThreshold = 0.25f;

    FCombat_TacticalData()
    {
        AggressionLevel = 0.5f;
        CautiousLevel = 0.3f;
        PackCoordination = 0.7f;
        TerritorialRadius = 2000.0f;
        DetectionRange = 1500.0f;
        AttackRange = 300.0f;
        FleeHealthThreshold = 0.25f;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tactical AI functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateTacticalState();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttackPattern(ECombat_AttackPattern Pattern);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* PotentialTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateWithPack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat();

    // Pack behavior functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(UCombat_TacticalAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    bool IsPackLeader() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_TacticalState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetDistanceToTarget() const;

protected:
    // Core state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_TacticalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    ECombat_AttackPattern CurrentPattern;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    // Tactical configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_TacticalData TacticalConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsApexPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsHerbivore;

    // Pack system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    TArray<UCombat_TacticalAI*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    UCombat_TacticalAI* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    bool bIsInPack;

    // Timers and cooldowns
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float StateTimer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float AttackCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    float LastAttackTime;

    // Internal functions
    void UpdateIdleState();
    void UpdatePatrolState();
    void UpdateAlertState();
    void UpdateHuntingState();
    void UpdateAttackingState();
    void UpdateFleeingState();
    void UpdateCirclingState();
    void UpdateAmbushState();

    bool CanSeeTarget(AActor* Target);
    bool IsTargetInRange(AActor* Target, float Range);
    void MoveTowardsTarget(float DeltaTime);
    void ExecuteDirectAttack();
    void ExecuteFlankingAttack();
    void ExecuteAmbushAttack();
    void ExecutePackHunt();
    void ExecuteTacticalRetreat();
    void ExecuteCircleStrafe();

    FVector GetRandomPatrolPoint();
    void BroadcastToPackMembers(const FString& Message);
};