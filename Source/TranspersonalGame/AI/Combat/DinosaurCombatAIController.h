#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTree;

/**
 * AI Controller specialized for dinosaur combat behavior
 * Implements tactical combat AI with memory, adaptation, and individual personality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    TObjectPtr<UBlackboardComponent> BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

    // Perception Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    // Behavior Trees for different archetypes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> AmbusherBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> BerserkerBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> StalkerBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> PackHunterBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> DefenderBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TObjectPtr<UBehaviorTree> OpportunistBehaviorTree;

public:
    // Combat Profile and State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FDinosaurCombatProfile CombatProfile;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombatMemoryEntry CombatMemory;

    // Individual Identity
    UPROPERTY(BlueprintReadOnly, Category = "Individual Identity")
    FString IndividualName;

    UPROPERTY(BlueprintReadOnly, Category = "Individual Identity")
    int32 UniqueID;

    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatMemory(AActor* Player, bool bCanSeePlayer);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* Target) const;

    // Archetype-specific behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SwitchToArchetypeBehavior(ECombatArchetype NewArchetype);

    // Individual trait system
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void GenerateIndividualTraits();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool HasTrait(FGameplayTag TraitTag) const;

    // Tactical decision making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalAttackPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldAmbush() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldCircle() const;

    // Communication with other dinosaurs
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertNearbyDinosaurs(AActor* Threat, float AlertRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ReceiveAlert(AActor* Threat, ADinosaurCombatAIController* AlertSource);

    // Domestication system support
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    float GetDomesticationProgress() const { return DomesticationProgress; }

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void UpdateDomesticationProgress(float DeltaProgress);

protected:
    // Internal state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    float LastPlayerSightingTime;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Internal State")
    float StateChangeTime;

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationProgress = 0.0f;

    // Tactical decision timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Timing")
    float DecisionUpdateInterval = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Tactical Timing")
    float LastDecisionTime = 0.0f;

    // Helper functions
    void InitializePerception();
    void ConfigureSightSense();
    void ConfigureHearingSense();
    void UpdateTacticalDecisions();
    void HandleStateTransitions();
    UBehaviorTree* GetBehaviorTreeForArchetype(ECombatArchetype Archetype) const;

    // Blackboard key names (static for performance)
    static const FName BB_CurrentTarget;
    static const FName BB_LastKnownPlayerLocation;
    static const FName BB_CombatState;
    static const FName BB_ThreatLevel;
    static const FName BB_OptimalAttackPosition;
    static const FName BB_ShouldRetreat;
    static const FName BB_CanSeePlayer;
    static const FName BB_TimeSinceLastSighting;
};