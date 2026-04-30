#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

class APawn;
class UBehaviorTree;

/**
 * Combat AI Controller for dinosaur enemies
 * Handles tactical combat behavior, threat assessment, and pack coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Combat State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float ThreatLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeHealthThreshold;

    // Species-specific behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaur_Species DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsAmbushPredator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    bool bIsTerritorial;

public:
    // AI Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldFlee() const;

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Pack")
    TArray<ACombatAIController*> GetNearbyPackMembers(float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void CoordinatePackAttack();

    // Blackboard keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName TargetActorKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName CombatStateKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName ThreatLevelKey;

private:
    void InitializeAIPerception();
    void InitializeBlackboard();
    void UpdateBlackboard();
    
    float CalculateThreatLevel(AActor* Target) const;
    bool IsTargetInAttackRange() const;
    bool IsTargetVisible() const;
    
    // Pack coordination
    void SendPackSignal(ECombat_PackSignal Signal);
    void ReceivePackSignal(ECombat_PackSignal Signal, ACombatAIController* Sender);
};