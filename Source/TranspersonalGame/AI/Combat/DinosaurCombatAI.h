// DinosaurCombatAI.h
// Agent #12 — Combat & Enemy AI Agent
// PROD_CYCLE_AUTO_20260628_003
// Implements tactical combat AI for dinosaur enemies in the prehistoric survival game.
// Uses UE5 ACharacter + UCharacterMovementComponent as base — no custom movement.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// ENUMS — Combat AI state machine
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
    Retreating      UMETA(DisplayName = "Retreating"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Fleeing         UMETA(DisplayName = "Fleeing")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ECombat_AttackType : uint8
{
    Bite            UMETA(DisplayName = "Bite"),
    Charge          UMETA(DisplayName = "Charge"),
    Swipe           UMETA(DisplayName = "Swipe"),
    TailWhip        UMETA(DisplayName = "Tail Whip"),
    Gore            UMETA(DisplayName = "Gore"),
    Stomp           UMETA(DisplayName = "Stomp"),
    Pounce          UMETA(DisplayName = "Pounce")
};

// ============================================================
// STRUCTS — Combat data
// ============================================================

USTRUCT(BlueprintType)
struct FCombat_AttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AttackType AttackType = ECombat_AttackType::Bite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WindupTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanInterrupt = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoSpeciesTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float MaxHealth = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float SightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bDetectsByMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bDetectsBySmell = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    int32 MaxPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    TArray<FCombat_AttackData> Attacks;
};

USTRUCT(BlueprintType)
struct FCombat_ThreatEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsVisible = false;
};

// ============================================================
// ADinosaurCombatAIController — Main AI Controller
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame", meta = (DisplayName = "Dinosaur Combat AI Controller"))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ---- Perception ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ---- State Machine ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // ---- Threat Assessment ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatList(AActor* ThreatActor, float ThreatLevel, bool bVisible, FVector LastLocation);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetHighestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearThreat(AActor* ThreatActor);

    // ---- Combat Actions ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(ECombat_AttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BeginCharge(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AbortCharge();

    // ---- Pack Coordination ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SignalPackMembers(ECombat_DinoState SignalState, FVector SignalLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ReceivePackSignal(ECombat_DinoState SignalState, FVector SignalLocation, ADinosaurCombatAIController* Sender);

    // ---- Species Config ----
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ApplySpeciesTraits(const FCombat_DinoSpeciesTraits& Traits);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    FCombat_DinoSpeciesTraits SpeciesTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTreeAsset;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception",
        meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    TArray<FCombat_ThreatEntry> ThreatList;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCharging;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    FVector ChargeTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI",
        meta = (AllowPrivateAccess = "true"))
    TArray<ADinosaurCombatAIController*> PackMembers;

private:
    void UpdateStateMachine(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleInvestigateState(float DeltaTime);
    void HandleStalkState(float DeltaTime);
    void HandleChargeState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void HandleRetreatState(float DeltaTime);

    float StateEntryTime;
    FVector PatrolOrigin;
    FVector InvestigateTarget;
    int32 CurrentPatrolIndex;
    TArray<FVector> PatrolWaypoints;
};
