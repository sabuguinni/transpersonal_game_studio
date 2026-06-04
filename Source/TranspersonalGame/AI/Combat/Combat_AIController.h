#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_AIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTree;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "3000.0"))
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIPersonality AIPersonality;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownTargetLocation;

    // Pack Behavior (for pack hunters like raptors)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack AI")
    bool bIsPackHunter;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    TArray<ACombat_AIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack AI")
    ACombat_AIController* PackLeader;

public:
    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    // Pack Functions
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(ACombat_AIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void AddPackMember(ACombat_AIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(AActor* Target);

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteTacticalMovement();
    void UpdatePackCoordination();

    // Combat Timers
    float LastAttackTime;
    float AttackCooldown;
    float ThreatAssessmentTimer;
};