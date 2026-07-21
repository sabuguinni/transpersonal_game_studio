#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "TRexAIController.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_TRexState : uint8
{
    Patrol       UMETA(DisplayName = "Patrol"),
    Alert        UMETA(DisplayName = "Alert"),
    Investigate  UMETA(DisplayName = "Investigate"),
    Chase        UMETA(DisplayName = "Chase"),
    Attack       UMETA(DisplayName = "Attack"),
    Roar         UMETA(DisplayName = "Roar"),
    Retreat      UMETA(DisplayName = "Retreat"),
    Dead         UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_TRexAttackType : uint8
{
    Bite         UMETA(DisplayName = "Bite"),
    TailSwipe    UMETA(DisplayName = "TailSwipe"),
    Stomp        UMETA(DisplayName = "Stomp"),
    Charge       UMETA(DisplayName = "Charge")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_TRexSenses
{
    GENERATED_BODY()

    /** Sight radius for detecting player (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightRadius = 3000.0f;

    /** Peripheral sight radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float PeripheralSightRadius = 1500.0f;

    /** Hearing radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float HearingRadius = 2000.0f;

    /** Sight half-angle (degrees) — T-Rex has poor forward vision, good lateral */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightHalfAngle = 75.0f;

    /** Time before losing track of player (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float LostTargetTime = 8.0f;

    /** Motion sensitivity — stationary player reduces detection chance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float MotionSensitivity = 1.0f;
};

USTRUCT(BlueprintType)
struct FCombat_TRexAttackData
{
    GENERATED_BODY()

    /** Bite damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float BiteDamage = 85.0f;

    /** Tail swipe damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float TailSwipeDamage = 45.0f;

    /** Stomp damage (AoE) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float StompDamage = 60.0f;

    /** Charge damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float ChargeDamage = 100.0f;

    /** Bite range (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float BiteRange = 350.0f;

    /** Tail swipe range (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float TailSwipeRange = 500.0f;

    /** Stomp AoE radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float StompRadius = 300.0f;

    /** Cooldown between attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float AttackCooldown = 2.5f;

    /** Charge minimum distance to trigger (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    float ChargeMinDistance = 1500.0f;
};

// ============================================================
// ATRexAIController
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Combat|AI")
class TRANSPERSONALGAME_API ATRexAIController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexAIController();

    // ---- Behavior Tree ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|BehaviorTree")
    TObjectPtr<UBehaviorTree> TRexBehaviorTree;

    // ---- Senses configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    FCombat_TRexSenses SensesConfig;

    // ---- Attack configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Attack")
    FCombat_TRexAttackData AttackConfig;

    // ---- Current AI state ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    ECombat_TRexState CurrentState;

    // ---- Blackboard keys ----
    static const FName BB_TargetActor;
    static const FName BB_TargetLocation;
    static const FName BB_AIState;
    static const FName BB_PatrolIndex;
    static const FName BB_LastKnownLocation;
    static const FName BB_bCanAttack;
    static const FName BB_bIsRoaring;

    // ---- State machine ----
    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    void SetAIState(ECombat_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|State")
    ECombat_TRexState GetAIState() const { return CurrentState; }

    // ---- Combat ----
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ExecuteAttack(ECombat_TRexAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    bool IsPlayerInChargeRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    ECombat_TRexAttackType SelectBestAttack() const;

    // ---- Detection ----
    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    bool IsPlayerMoving() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Detection")
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ---- Roar ----
    UFUNCTION(BlueprintCallable, Category = "TRex|Roar")
    void TriggerRoar();

    /** Roar fear radius — nearby players get fear debuff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarFearRadius = 1500.0f;

    /** Fear intensity applied to player on roar (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarFearIntensity = 0.65f;

    /** Roar cooldown (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarCooldown = 15.0f;

    // ---- Head tracking ----
    UFUNCTION(BlueprintCallable, Category = "TRex|Animation")
    void UpdateHeadTracking();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaTime) override;

private:
    // Perception component
    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> PerceptionComp;

    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "TRex|Perception",
              meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // Cached references
    UPROPERTY(Transient)
    TObjectPtr<AActor> TrackedTarget;

    // Timers
    float AttackCooldownRemaining;
    float RoarCooldownRemaining;
    float LostTargetTimer;

    // Internal helpers
    void UpdateStateMachine(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleAlertState(float DeltaTime);
    void HandleChaseState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void ApplyRoarFear(float Radius, float Intensity);
    void SetBlackboardTarget(AActor* Target);
};
