#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "CombatAIController.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Engaging,
    Retreating,
    Circling,
    Ambushing,
    Fleeing
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

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Combat Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* CombatBehaviorTree;

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName CombatStateKey = TEXT("CombatState");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName ThreatLevelKey = TEXT("ThreatLevel");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName LastKnownLocationKey = TEXT("LastKnownLocation");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName PatrolPointKey = TEXT("PatrolPoint");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName FlankingPositionKey = TEXT("FlankingPosition");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    FName RetreatPositionKey = TEXT("RetreatPosition");

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float OptimalCombatDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatThreshold = 0.3f; // 30% health

    // Tactical Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanFlank = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanAmbush = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanRetreat = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    bool bCanCallForHelp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
    float AggressionLevel = 0.7f; // 0.0 = Passive, 1.0 = Extremely Aggressive

    // Gameplay Tags for Combat States
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Tags")
    FGameplayTagContainer CombatTags;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintPure, Category = "Combat")
    EThreatLevel GetThreatLevel() const;

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat")
    AActor* GetTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearTarget();

    // Tactical Decisions
    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldEngageTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    bool ShouldFlank() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector GetFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Tactics")
    FVector GetRetreatPosition() const;

    // Combat Utilities
    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool HasLineOfSightToTarget() const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AlertNearbyAllies(const FVector& ThreatLocation, float AlertRadius = 1000.0f);

private:
    // Internal state tracking
    ECombatState CurrentCombatState = ECombatState::Idle;
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;
    
    // Timers for tactical decisions
    float LastFlankingTime = 0.0f;
    float LastRetreatTime = 0.0f;
    float LastAlertTime = 0.0f;
    
    // Cooldowns
    float FlankingCooldown = 5.0f;
    float RetreatCooldown = 10.0f;
    float AlertCooldown = 15.0f;

    // Helper functions
    void InitializePerception();
    void UpdateTacticalState();
    EThreatLevel CalculateThreatLevel() const;
    bool IsInCombatRange() const;
    bool CanSeeTarget() const;
    void ProcessSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
    void ProcessHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);
};