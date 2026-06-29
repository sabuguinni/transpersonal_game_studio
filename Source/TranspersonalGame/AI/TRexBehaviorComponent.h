#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ENUMS — global scope (UHT requirement)
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Roar        UMETA(DisplayName = "Roar"),
    Resting     UMETA(DisplayName = "Resting")
};

// ─────────────────────────────────────────────────────────────────────────────
// STRUCTS — global scope
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FNPC_TRexPatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float WaitDuration = 3.0f;
};

USTRUCT(BlueprintType)
struct FNPC_TRexSensoryData
{
    GENERATED_BODY()

    /** Last known location of the detected target */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Sensory")
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    /** Time since target was last seen (seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Sensory")
    float TimeSinceLastSeen = 0.0f;

    /** Whether target is currently visible */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Sensory")
    bool bTargetVisible = false;

    /** Whether ground vibration detected (simulates T-Rex sensitivity to footsteps) */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Sensory")
    bool bVibrationsDetected = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UTRexBehaviorComponent
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UTRexBehaviorComponent
 *
 * Drives T-Rex AI behaviour: patrol → alert → chase → attack loop.
 * Designed for realistic predator behaviour:
 *   - T-Rex detects by sight (cone) and ground vibration (proximity)
 *   - Chases until target escapes or T-Rex loses interest after 15s
 *   - Roars when entering chase to intimidate (triggers FearAlpha on player)
 *   - Rests after a kill or after sustained chase with no contact
 *
 * Attach to any APawn that represents a T-Rex.
 */
UCLASS(ClassGroup = "TranspersonalGame|AI", meta = (BlueprintSpawnableComponent),
       DisplayName = "TRex Behavior Component")
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    // ── UActorComponent overrides ─────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── State accessors ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void ForceState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Sensory")
    const FNPC_TRexSensoryData& GetSensoryData() const { return SensoryData; }

    // ── Patrol configuration ──────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void SetPatrolPoints(const TArray<FNPC_TRexPatrolPoint>& Points);

    UFUNCTION(BlueprintCallable, Category = "TRex|Patrol")
    void GenerateRandomPatrolPoints(float Radius, int32 NumPoints);

    // ── Combat events (called by Combat AI Agent #12) ─────────────────────

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void OnTargetEnteredAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void OnTargetLeftAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void OnKillConfirmed();

    // ── Delegates ─────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_TRexStateChanged,
                                                ENPC_TRexState, NewState);

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexStateChanged OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNPC_TRexRoarEvent, float, FearIntensity);

    UPROPERTY(BlueprintAssignable, Category = "TRex|Events")
    FNPC_TRexRoarEvent OnRoar;

    // ── Tuning parameters ─────────────────────────────────────────────────

    /** Radius within which T-Rex patrols (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "1000.0", ClampMax = "20000.0"))
    float PatrolRadius = 500000.0f; // 5000 UU = 50m

    /** Distance at which T-Rex detects player by sight (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float SightRange = 300000.0f; // 3000 UU = 30m

    /** Horizontal half-angle of sight cone (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "10.0", ClampMax = "90.0"))
    float SightConeHalfAngle = 60.0f;

    /** Distance at which ground vibrations are detected (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float VibrationDetectionRange = 100000.0f; // 1000 UU = 10m

    /** Distance at which T-Rex can melee attack (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float AttackRange = 30000.0f; // 300 UU = 3m

    /** How long T-Rex chases without seeing target before giving up (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "5.0", ClampMax = "60.0"))
    float ChaseGiveUpTime = 15.0f;

    /** Fear intensity broadcast when T-Rex roars (0-1, drives PlayerAnimInstance.FearAlpha) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RoarFearIntensity = 0.85f;

    /** Movement speed during patrol (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning")
    float PatrolSpeed = 250.0f;

    /** Movement speed during chase (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Tuning")
    float ChaseSpeed = 800.0f;

private:
    // ── Internal state ────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
              meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState = ENPC_TRexState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Sensory",
              meta = (AllowPrivateAccess = "true"))
    FNPC_TRexSensoryData SensoryData;

    UPROPERTY()
    TArray<FNPC_TRexPatrolPoint> PatrolPoints;

    UPROPERTY()
    int32 CurrentPatrolIndex = 0;

    UPROPERTY()
    float PatrolWaitTimer = 0.0f;

    UPROPERTY()
    float ChaseTimer = 0.0f;

    UPROPERTY()
    float RoarCooldown = 0.0f;

    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentTarget;

    // ── Internal helpers ──────────────────────────────────────────────────

    void TransitionToState(ENPC_TRexState NewState);
    void TickIdle(float DeltaTime);
    void TickPatrol(float DeltaTime);
    void TickAlert(float DeltaTime);
    void TickChase(float DeltaTime);
    void TickAttack(float DeltaTime);
    void TickRoar(float DeltaTime);
    void TickResting(float DeltaTime);

    bool CanSeeTarget(AActor* Target) const;
    bool CanDetectVibrations(AActor* Target) const;
    void UpdateSensoryData(float DeltaTime);
    void MoveToLocation(const FVector& TargetLocation, float Speed);
    void TriggerRoar();
    AActor* FindNearestPlayer() const;
};
