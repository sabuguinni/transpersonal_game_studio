#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Guarding    UMETA(DisplayName = "Guarding"),
    Patrolling  UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurAggressionLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Defensive   UMETA(DisplayName = "Defensive"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Enraged     UMETA(DisplayName = "Enraged")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurCombatStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;
};

USTRUCT(BlueprintType)
struct FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<class ACombat_DinosaurCombatAI*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    class ACombat_DinosaurCombatAI* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCohesionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* SharedTarget = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* PerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    FCombat_DinosaurCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    ECombat_DinosaurCombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    ECombat_DinosaurAggressionLevel AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    EDinosaurSpecies DinosaurSpecies;

    // Pack Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackAnimal = false;

    // Targets and Threats
    UPROPERTY(BlueprintReadOnly, Category = "Combat Targets")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Targets")
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Targets")
    TArray<AActor*> NearbyAllies;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_DinosaurCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAggressionLevel(ECombat_DinosaurAggressionLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ClearTarget();

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    bool CanAttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void StartHunting(AActor* Prey);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombat_DinosaurCombatAI* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    TArray<ACombat_DinosaurCombatAI*> GetNearbyPackMembers();

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Utility")
    float GetDistanceToTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat Utility")
    bool IsTargetInAttackRange();

    UFUNCTION(BlueprintCallable, Category = "Combat Utility")
    FVector GetOptimalAttackPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat Utility")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat Utility")
    void UpdateThreatAssessment();

protected:
    // Internal Combat Logic
    void ProcessCombatBehavior(float DeltaTime);
    void UpdatePackCoordination();
    void EvaluateThreats();
    void SelectBestTarget();
    void ExecuteCombatState();

    // Timers
    float LastAttackTime = 0.0f;
    float ThreatUpdateTimer = 0.0f;
    float PackCoordinationTimer = 0.0f;

    // Configuration
    static const float THREAT_UPDATE_INTERVAL;
    static const float PACK_COORDINATION_INTERVAL;
};