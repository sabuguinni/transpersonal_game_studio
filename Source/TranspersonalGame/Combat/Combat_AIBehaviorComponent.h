#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "Combat_AIBehaviorComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AIState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIBehaviorState CurrentState = ECombat_AIBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float DetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    AActor* CurrentTarget = nullptr;

    FCombat_AIState()
    {
        CurrentState = ECombat_AIBehaviorState::Idle;
        AggressionLevel = 0.5f;
        DetectionRadius = 2000.0f;
        AttackRange = 500.0f;
        CurrentTarget = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_AIBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_AIBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_AIState AIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float StateUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bEnableDebugLogging = false;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetBehaviorState(ECombat_AIBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIBehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* DetectNearestPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateAIBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetDetected(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void OnTargetLost();

private:
    float LastStateUpdateTime = 0.0f;
    
    void ProcessIdleState();
    void ProcessPatrolState();
    void ProcessChaseState();
    void ProcessAttackState();
    void ProcessFleeState();
};