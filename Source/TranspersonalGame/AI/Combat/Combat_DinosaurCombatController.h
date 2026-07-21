#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../../SharedTypes.h"
#include "Combat_DinosaurCombatController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_CombatState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_CombatMode CombatMode = ECombat_CombatMode::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanAttack = true;

    FCombat_CombatState()
    {
        CombatMode = ECombat_CombatMode::Passive;
        AggressionLevel = 0.5f;
        ThreatLevel = 0.0f;
        CombatRange = 500.0f;
        AttackCooldown = 2.0f;
        bIsInCombat = false;
        bCanAttack = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AttackPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    FString AttackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float Cooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float WindupTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    bool bRequiresLineOfSight = true;

    FCombat_AttackPattern()
    {
        AttackName = TEXT("Basic Attack");
        Damage = 25.0f;
        Range = 200.0f;
        Cooldown = 3.0f;
        WindupTime = 1.0f;
        bRequiresLineOfSight = true;
    }
};

/**
 * Advanced combat AI controller for dinosaurs with tactical behavior
 * Handles aggression states, attack patterns, and threat assessment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core combat components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat state management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    FCombat_CombatState CombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    TArray<FCombat_AttackPattern> AttackPatterns;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboard* CombatBlackboard;

    // Perception settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 1200.0f;

    // Combat timing
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float DistanceToTarget = 0.0f;

public:
    // Combat state functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatMode(ECombat_CombatMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateThreatLevel(float NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EnterCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExitCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(int32 AttackIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombat_AttackPattern GetBestAttackForRange(float Range) const;

    // Tactical functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatBlackboard();

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    FCombat_CombatState GetCombatState() const { return CombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const { return CombatState.bIsInCombat; }

private:
    void SetupPerception();
    void UpdateCombatState(float DeltaTime);
    void ProcessThreatAssessment();
    void HandleCombatLogic();
    AActor* FindBestTarget() const;
    float CalculateThreatScore(AActor* Actor) const;
};