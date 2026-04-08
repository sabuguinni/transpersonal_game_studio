#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

// Forward Declarations
class UBehaviorTree;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UEnvironmentQueryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetDetected, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLost, AActor*, Target);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Perception")
    class UAISenseConfig_Damage* DamageConfig;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // EQS Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI EQS")
    class UEnvironmentQueryComponent* EQSComponent;

    // Combat Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Profile")
    FDinosaurCombatProfile CombatProfile;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombatMemory CombatMemory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    float CurrentStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    float CurrentHealth;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float LastAttackTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float StateChangeTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timers")
    float AlertnessDecayTimer;

    // Target Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Tracking")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Tracking")
    TArray<AActor*> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target Tracking")
    TArray<AActor*> NearbyAllies;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnTargetDetected OnTargetDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnTargetLost OnTargetLost;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatProfile(const FDinosaurCombatProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FDinosaurCombatProfile GetCombatProfile() const { return CombatProfile; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanPerformAttack(EAttackType AttackType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FAttackPattern GetBestAttackForTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack(const FAttackPattern& AttackPattern);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(float DamageAmount, AActor* DamageSource);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool HasLineOfSightToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Target Management")
    void SetCurrentTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Target Management")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Target Management")
    void AddKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Target Management")
    void RemoveKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateLastKnownTargetLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetLastKnownTargetLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool HasEnoughStamina(float Required) const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetStaminaPercentage() const;

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Functions
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    void InitializeBlackboard();
    void UpdateCombatState(float DeltaTime);
    void UpdateStamina(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void UpdateAlertness(float DeltaTime);
    void EvaluateThreats();
    void SelectBestTarget();
    
    // Combat Logic
    bool ShouldEngageTarget(AActor* Target) const;
    bool ShouldFleeFromTarget(AActor* Target) const;
    float CalculateThreatLevel(AActor* Target) const;
    float CalculateTargetPriority(AActor* Target) const;
    
    // State Transitions
    void TransitionToIdle();
    void TransitionToPatrolling();
    void TransitionToInvestigating();
    void TransitionToStalking();
    void TransitionToHunting();
    void TransitionToAttacking();
    void TransitionToFleeing();
    void TransitionToFeeding();
    void TransitionToResting();

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName TargetLocationKey = TEXT("TargetLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName CombatStateKey = TEXT("CombatState");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName LastKnownLocationKey = TEXT("LastKnownLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName CanAttackKey = TEXT("CanAttack");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName StaminaPercentageKey = TEXT("StaminaPercentage");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName HealthPercentageKey = TEXT("HealthPercentage");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName AlertnessLevelKey = TEXT("AlertnessLevel");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName PatrolPointKey = TEXT("PatrolPoint");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackboard Keys")
    FName HomeLocationKey = TEXT("HomeLocation");
};