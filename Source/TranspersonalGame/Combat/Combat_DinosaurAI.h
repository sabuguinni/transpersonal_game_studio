#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "Combat_DinosaurAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Territorial     UMETA(DisplayName = "Territorial"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    Feeding         UMETA(DisplayName = "Feeding")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    SolitaryPredator    UMETA(DisplayName = "Solitary Predator"),
    PackHunter          UMETA(DisplayName = "Pack Hunter"),
    Herbivore           UMETA(DisplayName = "Herbivore"),
    Scavenger           UMETA(DisplayName = "Scavenger"),
    ApexPredator        UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurStats
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
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float TerritoryRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float PackCoordination = 0.8f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_DinosaurType DinosaurType = ECombat_DinosaurType::SolitaryPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FCombat_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    class UBehaviorTree* BehaviorTree;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector TerritoryCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime = 0.0f;

public:
    // AI Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializePackHunting(const TArray<AActor*>& PackMemberActors);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BroadcastPackAlert(AActor* ThreatTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetHealthPercentage() const;

protected:
    // Internal AI Logic
    void UpdateAIBehavior(float DeltaTime);
    void ProcessPerception();
    void UpdatePackCoordination();
    void HandleTerritorialBehavior();
    void ExecuteHuntingBehavior();
    void ProcessCombatLogic();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Pack Coordination
    void CoordinatePackAttack();
    void UpdatePackFormation();
    FVector CalculateFlankingPosition(AActor* Target);

    // Combat Utilities
    bool IsValidTarget(AActor* Actor) const;
    float CalculateThreatLevel(AActor* Actor) const;
    void SelectBestTarget();
    void UpdateAggressionLevel();

private:
    // Internal State
    float StateTimer = 0.0f;
    float NextBehaviorUpdate = 0.0f;
    float PackCoordinationTimer = 0.0f;
    TArray<AActor*> DetectedThreats;
    TArray<AActor*> DetectedPrey;
    
    // Combat Timers
    float AttackCooldown = 2.0f;
    float TerritoryCheckInterval = 5.0f;
    float NextTerritoryCheck = 0.0f;
};