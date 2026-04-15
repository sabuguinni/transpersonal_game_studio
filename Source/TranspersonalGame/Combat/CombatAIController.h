#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class UCombat_TacticalBehaviorComponent;
class UCombat_CombatStateComponent;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    // Combat Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_TacticalBehaviorComponent* TacticalBehavior;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_CombatStateComponent* CombatState;

    // Combat AI Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIPersonality AIPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TacticalIntelligence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FleeHealthThreshold;

public:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCombatTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExitCombatMode();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInCombat() const;

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalFlankPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldCallForHelp() const;

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RequestPackAssistance();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RespondToPackCall(ACombat_AIController* Caller);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<ACombat_AIController*> GetNearbyPackMembers(float SearchRadius = 1000.0f);

private:
    // Internal state
    AActor* CurrentTarget;
    bool bInCombatMode;
    float LastAttackTime;
    float LastFlankTime;
    FVector LastKnownTargetLocation;

    // Tactical timers
    FTimerHandle TacticalUpdateTimer;
    FTimerHandle PerceptionUpdateTimer;

    // Internal methods
    void UpdateTacticalState();
    void UpdateBlackboardValues();
    bool CanSeeTarget(AActor* Target) const;
    float GetDistanceToTarget() const;
    void HandleTargetLost();
    void HandleTargetFound(AActor* NewTarget);
};