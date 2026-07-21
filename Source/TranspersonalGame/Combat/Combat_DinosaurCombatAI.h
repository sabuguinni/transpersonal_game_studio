#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Combat_DinosaurCombatAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Hunting UMETA(DisplayName = "Hunting"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Patrolling UMETA(DisplayName = "Patrolling")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Pteranodon UMETA(DisplayName = "Pteranodon")
};

USTRUCT(BlueprintType)
struct FCombat_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float Aggression = 0.7f;

    FCombat_DinosaurStats()
    {
        Health = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 300.0f;
        DetectionRange = 1500.0f;
        MovementSpeed = 400.0f;
        Aggression = 0.7f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Dinosaur Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    ECombat_DinosaurSpecies DinosaurSpecies = ECombat_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FCombat_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    class UBehaviorTree* BehaviorTreeAsset;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ECombat_DinosaurState CurrentState = ECombat_DinosaurState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    class AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    float LastAttackTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AttackCooldown = 2.0f;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetDinosaurState(ECombat_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void MoveToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeDinosaurStats();

protected:
    // Internal AI Logic
    void UpdateAIBehavior(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Utility Functions
    void SetupAIPerception();
    void ConfigureBlackboard();
    AActor* FindNearestPlayer() const;
    bool IsPlayerInRange(float Range) const;

private:
    float StateTimer = 0.0f;
    FVector PatrolStartLocation;
    FVector PatrolTargetLocation;
    bool bIsPatrolling = false;
    float NextDecisionTime = 0.0f;
};