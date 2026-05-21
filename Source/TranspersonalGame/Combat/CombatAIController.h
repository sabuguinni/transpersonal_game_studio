#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Pack        UMETA(DisplayName = "Pack Coordination")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator    UMETA(DisplayName = "Predator"),
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
    Apex        UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FCombat_AIStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float PackCoordination = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    // Combat Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_AIStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurType DinosaurType;

    // Target Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> KnownEnemies;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<ACombatAIController*> PackMembers;

public:
    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AddToKnownEnemies(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RemoveFromKnownEnemies(AActor* Enemy);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombatRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInDetectionRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat();

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombatAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackAttack(AActor* Target);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombat_AIState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombat_AIStats GetCombatStats() const { return CombatStats; }

private:
    void UpdateCombatLogic();
    void UpdatePackBehavior();
    void ProcessPerceptionData();
    
    float LastAttackTime;
    float AttackCooldown = 2.0f;
    
    FVector LastKnownTargetLocation;
    float TimeWithoutTarget = 0.0f;
};