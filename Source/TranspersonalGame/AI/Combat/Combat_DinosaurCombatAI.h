#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "Combat_DinosaurCombatAI.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float FleeHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    ECombat_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    ECombat_CombatState CurrentCombatState;

    // Combat Methods
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
    // Combat State Tracking
    AActor* CurrentTarget;
    float LastAttackTime;
    float CombatStartTime;
    bool bIsInCombat;

    // Species-specific combat behavior
    void InitializeSpeciesBehavior();
    void UpdateTRexBehavior();
    void UpdateVelociraptorBehavior();
    void UpdateTriceratopsBehavior();
    void UpdateBrachiosaurusBehavior();
};