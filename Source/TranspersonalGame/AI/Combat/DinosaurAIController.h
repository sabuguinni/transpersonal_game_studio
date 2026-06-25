#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Behavior/DinosaurBehaviorTask.h"
#include "DinosaurAIController.generated.h"

// Blackboard key names — used by BT tasks and AI Controller
#define BB_KEY_TARGET_ACTOR     TEXT("TargetActor")
#define BB_KEY_PATROL_ORIGIN    TEXT("PatrolOrigin")
#define BB_KEY_PATROL_RADIUS    TEXT("PatrolRadius")
#define BB_KEY_AI_STATE         TEXT("AIState")
#define BB_KEY_LAST_KNOWN_POS   TEXT("LastKnownPosition")
#define BB_KEY_ATTACK_COOLDOWN  TEXT("AttackCooldown")
#define BB_KEY_IS_HUNGRY        TEXT("IsHungry")

/** Combat state for the AI controller — drives BT selector priorities */
UENUM(BlueprintType)
enum class ECombat_ControllerState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Roaring     UMETA(DisplayName = "Roaring")
};

/** Per-species combat configuration — tuned for realistic predator behavior */
USTRUCT(BlueprintType)
struct FCombat_DinoConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ENPC_DinoSpecies Species = ENPC_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightRadius = 2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightAngle = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HearingRadius = 1500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackDamage = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float MoveSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChaseSpeed = 900.f;

    /** Pack hunt radius — raptors within this range coordinate attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackCoordinationRadius = 800.f;

    /** Whether this species hunts in packs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bPackHunter = false;

    /** Whether this species uses flanking maneuvers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bUsesFlanking = false;
};

/**
 * ADinosaurAIController
 * 
 * AI Controller for all dinosaur species. Owns BehaviorTree + Blackboard + AIPerception.
 * Drives the 4 BT task nodes from DinosaurBehaviorTask.h (Patrol/Chase/Attack/Roar).
 * Supports pack coordination for Velociraptors and flanking maneuvers.
 * 
 * Usage: Assign to any APawn subclass representing a dinosaur.
 * Set DinoConfig.Species to configure per-species behavior.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (DisplayName = "Dinosaur AI Controller"))
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    FCombat_DinoConfig DinoConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|Config")
    class UBehaviorTree* DinosaurBehaviorTree;

    // --- State ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State",
        meta = (AllowPrivateAccess = "true"))
    ECombat_ControllerState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI|State")
    float MaxHealth = 500.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|State")
    AActor* CurrentTarget;

    // --- Pack Coordination ---

    /** Other raptors in the pack — set at spawn or via pack manager */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<ADinosaurAIController*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Pack")
    bool bIsPackLeader = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Pack")
    bool bIsFlankingLeft = false;

    // --- Components ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Components",
        meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Components",
        meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComp;

    // --- Public API ---

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeCombatDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ADinosaurAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(AActor* SharedTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ControllerState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool HasLivingTarget() const;

    /** Returns the flanking offset vector for this pack member */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFlankingOffset(AActor* Target) const;

    /** Apply species-specific config presets */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Combat AI")
    void ApplySpeciesPreset();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    /** Perception callback — fires when sight/hearing detects something */
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    /** Transition to a new AI state, updating Blackboard */
    void TransitionToState(ECombat_ControllerState NewState);

    /** Initialize Blackboard keys from DinoConfig */
    void InitializeBlackboard();

    /** Set up AIPerception sight/hearing configs from DinoConfig */
    void ConfigurePerception();

    /** Scan for pack members within PackCoordinationRadius */
    void ScanForPackMembers();

    float TimeSinceLastPackScan = 0.f;
    static constexpr float PackScanInterval = 3.f;
};
