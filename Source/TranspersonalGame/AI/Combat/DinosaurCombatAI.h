// DinosaurCombatAI.h
// Agent #12 — Combat & Enemy AI
// Dinosaur combat AI controller — tactical behavior for predator encounters
// CYCLE: PROD_CYCLE_AUTO_20260628_009

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "DinosaurCombatAI.generated.h"

// Combat state for dinosaur AI
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding")
};

// Dinosaur species type for behavior differentiation
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops")
};

// Combat threat assessment result
USTRUCT(BlueprintType)
struct FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bTargetIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bTargetIsArmed = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bPackSupport = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    int32 PackMemberCount = 0;
};

// Patrol waypoint data
USTRUCT(BlueprintType)
struct FCombat_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    float WaitTime = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    bool bIsAlertPoint = false;
};

UCLASS(ClassGroup = (CombatAI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat state machine
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_ThreatAssessment AssessThreat(AActor* Target);

    // Patrol system
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddPatrolPoint(FVector Location, float WaitTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartPatrol();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StopPatrol();

    // Combat actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertNearbyPackMembers(AActor* Threat);

    // Species configuration
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ConfigureForSpecies(ECombat_DinoSpecies Species);

    // Blackboard keys
    static const FName BB_TargetActor;
    static const FName BB_PatrolLocation;
    static const FName BB_CombatState;
    static const FName BB_ThreatLevel;
    static const FName BB_HomeLocation;

protected:
    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* PerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComp;

    // Species and behavior config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::Carnotaurus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    // Perception ranges
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float SightAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChargeRange = 800.0f;

    // Combat stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ChargeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AlertSpeed = 600.0f;

    // Territory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritoryRadius = 3000.0f;

    // Patrol data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<FCombat_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    int32 CurrentPatrolIndex = 0;

    // Current target
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget = nullptr;

    // State timers
    float StateTimer = 0.0f;
    float AlertCooldown = 0.0f;
    float AttackCooldown = 0.0f;

    // Internal state machine
    void UpdateIdleState(float DeltaTime);
    void UpdatePatrolState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateStalkingState(float DeltaTime);
    void UpdateChargingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateFleeingState(float DeltaTime);

    // Species-specific behavior multipliers
    float AggressionMultiplier = 1.0f;
    float PackBehaviorWeight = 0.0f;
    bool bIsPackPredator = false;
    bool bIsAmbushPredator = false;
};
