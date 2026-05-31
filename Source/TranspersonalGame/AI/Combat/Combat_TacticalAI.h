#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Combat_TacticalAI.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Patrol,
    Alert,
    Hunting,
    Combat,
    Retreat,
    Flanking,
    Ambush
};

UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    Predator_Alpha,     // T-Rex, Allosaurus
    Predator_Pack,      // Velociraptor, Deinonychus
    Herbivore_Large,    // Triceratops, Brachiosaurus
    Herbivore_Herd,     // Parasaurolophus, Hadrosaur
    Scavenger          // Compsognathus, small carnivores
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AmbushWaitTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 PackSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanCallReinforcements = true;

    FCombat_TacticalData()
    {
        AttackRange = 500.0f;
        FlankingRadius = 1000.0f;
        RetreatThreshold = 0.3f;
        AmbushWaitTime = 5.0f;
        PackSize = 3;
        bCanCallReinforcements = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalAI : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombat_TacticalData TacticalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    class UBehaviorTree* CombatBehaviorTree;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_TacticalState CurrentTacticalState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteFlankingManeuver();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CallForReinforcements();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdatePackCoordination();

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, struct FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void InitializePerception();
    void UpdateCombatBehavior(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteTacticalDecision();
    
    // Pack Behavior
    void FormPackFormation();
    void CoordinatePackAttack();
    void BreakPackFormation();

    // Combat Utilities
    float CalculateThreatLevel(AActor* Target) const;
    bool IsInAttackRange(AActor* Target) const;
    bool HasLineOfSight(AActor* Target) const;
    FVector CalculateFlankingPosition(AActor* Target) const;

private:
    // Timers and Cooldowns
    float StateChangeTime;
    float LastPerceptionUpdate;
    float CombatDecisionCooldown;
    
    // Combat Statistics
    int32 SuccessfulAttacks;
    int32 DamageDealt;
    float CombatDuration;
};