#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UCombatBehaviorComponent;
class UDinosaurPersonalityComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Territorial,
    Feeding,
    Investigating
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

/**
 * AI Controller especializado para combate tático de dinossauros
 * Implementa comportamentos complexos baseados em personalidade, estado emocional e contexto
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

    // ========== CORE COMPONENTS ==========
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UCombatBehaviorComponent* CombatBehavior;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UDinosaurPersonalityComponent* Personality;

    // ========== BEHAVIOR TREES ==========
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    class UBehaviorTree* IdleBehaviorTree;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    class UBehaviorTree* CombatBehaviorTree;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    class UBehaviorTree* HuntingBehaviorTree;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Behavior")
    class UBehaviorTree* FleeingBehaviorTree;

    // ========== COMBAT STATE ==========
    
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    EThreatLevel CurrentThreatLevel;
    
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* PrimaryTarget;
    
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> KnownThreats;

    // ========== PERCEPTION SETTINGS ==========
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
    float SightRadius = 2000.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
    float HearingRadius = 1500.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
    float PeripheralVisionAngle = 120.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
    float LoseTargetTime = 8.0f;

    // ========== COMBAT PARAMETERS ==========
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 300.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float OptimalCombatDistance = 500.0f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float FleeHealthThreshold = 0.3f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AggressionMultiplier = 1.0f;

public:
    // ========== STATE MANAGEMENT ==========
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(EThreatLevel NewLevel);
    
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }
    
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    EThreatLevel GetThreatLevel() const { return CurrentThreatLevel; }

    // ========== TARGET MANAGEMENT ==========
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddKnownThreat(AActor* Threat);
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveKnownThreat(AActor* Threat);
    
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget; }
    
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetNearestThreat() const;

    // ========== COMBAT DECISIONS ==========
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldAttack() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldStalk() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target) const;

    // ========== TACTICAL POSITIONING ==========
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetFleeDirection() const;
    
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetStalkingPosition() const;

protected:
    // ========== PERCEPTION CALLBACKS ==========
    
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
    
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ========== INTERNAL METHODS ==========
    
    void UpdateThreatAssessment();
    void SelectBehaviorTree();
    float CalculateThreatScore(AActor* Actor) const;
    void HandleCombatStateTransition(ECombatState NewState);
    
    // Timers
    FTimerHandle ThreatAssessmentTimer;
    FTimerHandle StateUpdateTimer;
};