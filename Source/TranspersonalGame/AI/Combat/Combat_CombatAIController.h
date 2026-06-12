#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat_CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_CombatStyle : uint8
{
    AmbushPredator UMETA(DisplayName = "Ambush Predator"),
    PackHunter UMETA(DisplayName = "Pack Hunter"),
    Defensive UMETA(DisplayName = "Defensive"),
    Flee UMETA(DisplayName = "Flee"),
    Tank UMETA(DisplayName = "Tank")
};

UENUM(BlueprintType)
enum class ECombat_CombatState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Stalking UMETA(DisplayName = "Stalking"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Defending UMETA(DisplayName = "Defending")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    ECombat_CombatStyle CombatStyle = ECombat_CombatStyle::AmbushPredator;

    FCombat_CombatStats()
    {
        Aggression = 0.5f;
        AttackRange = 500.0f;
        DetectionRange = 800.0f;
        AttackDamage = 25.0f;
        AttackCooldown = 2.0f;
        MovementSpeed = 400.0f;
        CombatStyle = ECombat_CombatStyle::AmbushPredator;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_CombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_CombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_CombatStats CombatStats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_CombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat;

public:
    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeCombatStats();

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Behavior Functions
    void HandleIdleState();
    void HandlePatrollingState();
    void HandleInvestigatingState();
    void HandleStalkingState();
    void HandleAttackingState();
    void HandleFleeingState();
    void HandleDefendingState();

    // Utility Functions
    bool ShouldEngageTarget(AActor* PotentialTarget) const;
    bool ShouldFleeFromTarget(AActor* PotentialTarget) const;
    FVector GetRandomPatrolPoint() const;
    void UpdateBlackboardValues();

private:
    FVector HomeLocation;
    float PatrolRadius;
    FTimerHandle CombatUpdateTimer;
    TArray<AActor*> DetectedEnemies;
    float StateChangeTime;
    int32 AttackComboCount;
};