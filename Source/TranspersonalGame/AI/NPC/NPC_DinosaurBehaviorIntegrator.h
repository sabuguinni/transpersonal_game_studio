#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorIntegrator.generated.h"

class ANPC_DinosaurAIController;
class UNPC_DinosaurPersonalitySystem;
class UNPC_DinosaurSensorySystem;
class UNPC_DinosaurVocalSystem;

/**
 * Master integration component that coordinates all dinosaur NPC behavior systems
 * Manages the interaction between AI controllers, personality, sensory, and vocal systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorSystems();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void HandleThreatDetection(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void HandleSocialInteraction(AActor* OtherActor, ENPCInteractionType InteractionType);

    // Behavior state management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorMode(ENPCBehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCBehaviorMode GetCurrentBehaviorMode() const { return CurrentBehaviorMode; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(ENPCEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    // Integration with other systems
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RegisterWithPackManager();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateTerritorialBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessEnvironmentalStimuli();

protected:
    // Core behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    ENPCBehaviorMode CurrentBehaviorMode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    ENPCEmotionalState CurrentEmotionalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    float BehaviorIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    float StressLevel;

    // System references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    ANPC_DinosaurAIController* DinosaurAIController;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    UNPC_DinosaurPersonalitySystem* PersonalitySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    UNPC_DinosaurSensorySystem* SensorySystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "System References")
    UNPC_DinosaurVocalSystem* VocalSystem;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float BehaviorUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float SocialInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float StressDecayRate;

    // Integration timers
    float LastBehaviorUpdate;
    float LastEnvironmentalScan;
    float LastSocialUpdate;

private:
    // Internal helper functions
    void UpdateSystemReferences();
    void ProcessBehaviorTransitions();
    void HandleStressManagement(float DeltaTime);
    void UpdateBehaviorIntensity();
    bool ShouldTransitionBehavior() const;
    void NotifyBehaviorChange();
};