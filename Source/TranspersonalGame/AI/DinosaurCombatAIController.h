#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DinosaurCombatAIController.generated.h"

// ─── Combat State Enum ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Stalking     UMETA(DisplayName = "Stalking"),
    Charging     UMETA(DisplayName = "Charging"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Retreating   UMETA(DisplayName = "Retreating"),
    Feeding      UMETA(DisplayName = "Feeding")
};

// ─── Species Type Enum ─────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pterodactyl  UMETA(DisplayName = "Pterodactyl")
};

// ─── Combat Parameters Struct ──────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_DinoParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float ChaseActivationRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Detection")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed_Patrol = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MoveSpeed_Chase = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float PackSignalRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    float TerritoryReturnRange = 8000.0f;
};

// ─── Threat Memory Entry ───────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCombat_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    bool bIsActive = false;
};

// ─── Main Controller Class ─────────────────────────────────────────────────
UCLASS(ClassGroup = "TranspersonalGame|CombatAI", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // ── State Machine ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void SetCombatState(ECombat_DinoState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat|State")
    ECombat_DinoState GetCombatState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToChase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void TransitionToPatrol();

    // ── Perception ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Perception")
    void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintPure, Category = "Combat|Perception")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Perception")
    bool IsTargetInAttackRange() const;

    // ── Pack Hunting ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void SignalPackMembers(AActor* SharedTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat|Pack")
    void ReceivePackSignal(AActor* SharedTarget, FVector FlankPosition);

    UFUNCTION(BlueprintPure, Category = "Combat|Pack")
    FVector CalculateFlankPosition(AActor* Target, int32 FlankIndex) const;

    // ── Attack ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat|Attack")
    void ApplyDamageToTarget(AActor* Target, float Damage);

    // ── Patrol ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Patrol")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Combat|Patrol")
    void ReturnToTerritory();

    // ── Species Config ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ConfigureForSpecies(ECombat_DinoSpecies Species);

    // ── Properties ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies DinoSpecies = ECombat_DinoSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoParams CombatParams;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Target")
    TObjectPtr<AActor> CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Patrol")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|Memory")
    FCombat_ThreatMemory ThreatMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Behavior")
    UBehaviorTree* BehaviorTreeAsset = nullptr;

private:
    UPROPERTY()
    TObjectPtr<UAIPerceptionComponent> PerceptionComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UBlackboardComponent> BlackboardComp = nullptr;

    float LastAttackTime = 0.0f;
    float StateEntryTime = 0.0f;
    int32 CurrentPatrolIndex = 0;

    void UpdateStateMachine(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrolState(float DeltaTime);
    void HandleChaseState(float DeltaTime);
    void HandleAttackState(float DeltaTime);
    void HandleRetreatState(float DeltaTime);
    void RecordThreatMemory(AActor* ThreatActor);
};
