#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DinosaurCombatAIController.generated.h"

// Forward declarations
class UNPCBehaviorComponent;
class UBlackboardComponent;
class UBehaviorTreeComponent;

// ============================================================
// ENUMS — global scope (UHT requirement)
// ============================================================

UENUM(BlueprintType)
enum class ECombat_ControllerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    PackHunting UMETA(DisplayName = "PackHunting")
};

UENUM(BlueprintType)
enum class ECombat_PackRole : uint8
{
    None        UMETA(DisplayName = "None"),
    Leader      UMETA(DisplayName = "Leader"),
    LeftFlanker UMETA(DisplayName = "LeftFlanker"),
    RightFlanker UMETA(DisplayName = "RightFlanker"),
    Ambusher    UMETA(DisplayName = "Ambusher")
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    ECombat_PackRole PackRole = ECombat_PackRole::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float FlankAngleDegrees = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float CoordinationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    bool bIsPackHuntActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    float PackSignalCooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Pack")
    FVector LastKnownTargetLocation = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float ThreatLevel = 0.0f;  // 0.0 = no threat, 1.0 = maximum threat

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float DistanceToTarget = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    bool bTargetIsArmed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    bool bTargetIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    int32 AlliesNearby = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Threat")
    float LastDamageTaken = 0.0f;
};

// ============================================================
// DELEGATES
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombat_OnPackSignal,
    ECombat_ControllerState, NewState,
    FVector, TargetLocation);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombat_OnThreatLevelChanged,
    float, NewThreatLevel);

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    // ---- Lifecycle ----
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- Pack Coordination ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void BroadcastPackSignal(ECombat_ControllerState NewState, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(ECombat_ControllerState SignalState, FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SetPackRole(ECombat_PackRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    ECombat_PackRole GetPackRole() const { return PackCoordination.PackRole; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void InitiatePackHunt(APawn* Target);

    // ---- Threat Assessment ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Threat")
    float CalculateThreatLevel(APawn* Target);

    UFUNCTION(BlueprintPure, Category = "Combat|Threat")
    FCombat_ThreatAssessment GetCurrentThreatAssessment() const { return CurrentThreat; }

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetControllerState(ECombat_ControllerState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_ControllerState GetControllerState() const { return CurrentState; }

    // ---- Combat Actions ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ExecuteFlankingManeuver(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ExecuteAmbushApproach(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ExecuteChargeAttack(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|Actions")
    void ApplyCombatDamage(APawn* Target, float BaseDamage);

    // ---- Hit Reactions ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Reactions")
    void OnDamageTaken(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat|Reactions")
    void EscalateToAttackState();

    UFUNCTION(BlueprintCallable, Category = "Combat|Reactions")
    void TriggerFleeIfHerbivore();

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnPackSignal OnPackSignal;

    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FCombat_OnThreatLevelChanged OnThreatLevelChanged;

    // ---- Config ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_PackCoordination PackCoordination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    bool bIsCarnivore = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float ChaseSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float PackSignalRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float FleeHealthThreshold = 0.25f;  // Flee when HP < 25%

private:
    UPROPERTY()
    ECombat_ControllerState CurrentState = ECombat_ControllerState::Idle;

    UPROPERTY()
    FCombat_ThreatAssessment CurrentThreat;

    UPROPERTY()
    APawn* CurrentTarget = nullptr;

    float AttackCooldownTimer = 0.0f;
    float PackSignalCooldownTimer = 0.0f;
    float CurrentHealth = 100.0f;
    float MaxHealth = 100.0f;

    void UpdateCooldownTimers(float DeltaTime);
    void UpdateThreatAssessment();
    void ProcessStateLogic(float DeltaTime);
    FVector CalculateFlankPosition(FVector TargetLocation, float AngleDegrees, float Distance);
};
