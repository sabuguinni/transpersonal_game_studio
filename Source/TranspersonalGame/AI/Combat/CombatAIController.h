#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Circling,
    Ambushing,
    Retreating
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Combat State Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EThreatLevel CurrentThreatLevel;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CirclingDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AmbushWaitTime = 3.0f;

    // Tactical Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bUsePackTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanAmbush = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bPrefersFlanking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float TacticalUpdateInterval = 0.5f;

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    AActor* GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget(AActor* Target) const;

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector CalculateAmbushPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldAttack() const;

    // Pack Coordination (for pack hunters)
    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ACombatAIController*> GetNearbyPackMembers(float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack(AActor* Target);

private:
    // Internal state tracking
    AActor* CurrentTarget;
    FVector LastKnownTargetLocation;
    float LastTargetSightTime;
    float TacticalUpdateTimer;
    
    // Tactical positioning
    FVector PreferredAttackPosition;
    bool bHasValidAttackPosition;
    
    void UpdateTacticalPosition();
    void HandleLostTarget();
    void ExecuteCombatBehavior();
};