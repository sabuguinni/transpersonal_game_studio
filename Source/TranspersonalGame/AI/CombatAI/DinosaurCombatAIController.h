#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAIController.generated.h"

// ============================================================
// Enums — must be at global scope (UE5 C++ Rule 1)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Charging        UMETA(DisplayName = "Charging"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Recovering      UMETA(DisplayName = "Recovering"),
    PackCoordinate  UMETA(DisplayName = "PackCoordinate")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    None            UMETA(DisplayName = "None"),
    Bite            UMETA(DisplayName = "Bite"),
    Claw            UMETA(DisplayName = "Claw"),
    Charge          UMETA(DisplayName = "Charge"),
    TailSwipe       UMETA(DisplayName = "TailSwipe"),
    Pounce          UMETA(DisplayName = "Pounce"),
    Roar            UMETA(DisplayName = "Roar")
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    Solo            UMETA(DisplayName = "Solo"),
    Alpha           UMETA(DisplayName = "Alpha"),
    LeftFlanker     UMETA(DisplayName = "LeftFlanker"),
    RightFlanker    UMETA(DisplayName = "RightFlanker"),
    Distractor      UMETA(DisplayName = "Distractor"),
    Ambusher        UMETA(DisplayName = "Ambusher")
};

// ============================================================
// Structs — must be at global scope (UE5 C++ Rule 1)
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float WindupTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    float KnockbackForce = 500.0f;

    FCombat_AttackProfile() {}
};

USTRUCT(BlueprintType)
struct FCombat_PackSignal
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_PackRole SenderRole = ECombat_PackRole::Solo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float SignalStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsFlankCommand = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float Timestamp = 0.0f;

    FCombat_PackSignal() {}
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float ThreatScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float DistanceToTarget = 9999.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    bool bTargetIsArmed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    bool bTargetIsGrouped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    int32 AlliedDinosNearby = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float FleeThreshold = 3.0f;

    FCombat_ThreatAssessment() {}
};

// ============================================================
// Main Controller Class
// ============================================================

UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToFlee(FVector SafeLocation);

    // ---- Attack System ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    bool TryExecuteAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void RegisterAttackProfile(FCombat_AttackProfile Profile);

    UFUNCTION(BlueprintPure, Category = "Combat|Attack")
    float GetCurrentAttackDamage() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Attack")
    bool IsAttackOnCooldown() const;

    // ---- Pack Coordination ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetPackRole(ECombat_PackRole Role);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    ECombat_PackRole GetPackRole() const { return CurrentPackRole; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackSignal(FCombat_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(FCombat_PackSignal Signal);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    FVector ComputeFlankPosition(FVector TargetLocation, bool bFlankLeft) const;

    // ---- Threat Assessment ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    FCombat_ThreatAssessment AssessThreat(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    bool ShouldCallPackmates() const;

    // ---- Perception Callbacks ----
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ---- Blackboard Keys ----
    static const FName BB_TargetActor;
    static const FName BB_TargetLocation;
    static const FName BB_CombatState;
    static const FName BB_FlankPosition;
    static const FName BB_ThreatScore;
    static const FName BB_PackRole;

protected:
    // ---- Components ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent_Combat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // ---- State ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentCombatState = ECombat_DinoState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack", meta = (AllowPrivateAccess = "true"))
    ECombat_PackRole CurrentPackRole = ECombat_PackRole::Solo;

    // ---- Attack ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    FCombat_AttackProfile PrimaryAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
    FCombat_AttackProfile SecondaryAttack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Attack", meta = (AllowPrivateAccess = "true"))
    float LastAttackTime = -999.0f;

    // ---- Threat ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    FCombat_ThreatAssessment CurrentThreatAssessment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float LoseSightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float PeripheralVisionAngle = 60.0f;

    // ---- Pack ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankOffset = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackSignalRadius = 1200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Pack", meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_PackSignal> ReceivedPackSignals;

    // ---- Behavior Tree ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|BehaviorTree")
    TObjectPtr<UBehaviorTree> CombatBehaviorTree;

private:
    void TickCombatStateMachine(float DeltaTime);
    void TickPackCoordination(float DeltaTime);
    void UpdateBlackboardValues();
    void InitializePerception();
    void InitializeDefaultAttackProfiles();

    TWeakObjectPtr<AActor> CurrentTarget;
    float StateEntryTime = 0.0f;
    float PackSignalCooldown = 3.0f;
    float LastPackSignalTime = -999.0f;
};
