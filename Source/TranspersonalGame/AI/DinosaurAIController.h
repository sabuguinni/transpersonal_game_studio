#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "DinosaurAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class APawn;

/**
 * AI Controller for dinosaur NPCs with advanced behavior trees and perception
 * Handles territorial behavior, pack dynamics, and threat assessment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;
    virtual void UnPossess() override;

    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* PerceptionComponent;

    // Behavior Trees
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* FlockingBehaviorTree;

    // Blackboard Data
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* DinosaurBlackboard;

    // AI Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    ENPC_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float TerritorialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    float PackLoyalty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    bool bIsAlphaSpecimen;

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngle;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "AI Memory")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    UPROPERTY(BlueprintReadOnly, Category = "AI Memory")
    TArray<FNPC_LocationMemory> LocationMemories;

    UPROPERTY(BlueprintReadOnly, Category = "AI Memory")
    TArray<AActor*> PackMembers;

public:
    // AI Behavior Interface
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorTree(UBehaviorTree* NewBehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SwitchToCombatMode();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SwitchToFlockingMode();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SwitchToDefaultMode();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "AI Threat")
    void AssessThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "AI Threat")
    bool IsActorThreat(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Threat")
    float CalculateThreatLevel(AActor* Actor) const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "AI Pack")
    void RegisterPackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "AI Pack")
    void RemovePackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "AI Pack")
    void CallForBackup(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "AI Pack")
    void RespondToPackCall(AActor* PackLeader, AActor* Threat);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void RememberThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void RememberLocation(FVector Location, ENPC_LocationImportance Importance);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void ForgetOldMemories();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Behavior Tree Helpers
    void InitializeBehaviorTree();
    void ConfigurePerception();
    void SetupBlackboardValues();

    // Species-specific behavior initialization
    void ConfigureTRexBehavior();
    void ConfigureRaptorBehavior();
    void ConfigureHerbivoreBehavior();

private:
    // Internal state
    AActor* CurrentTarget;
    FVector HomeLocation;
    float LastThreatAssessmentTime;
    bool bInCombatMode;
    bool bInFlockingMode;

    // Memory cleanup timer
    FTimerHandle MemoryCleanupTimer;
    void CleanupMemories();
};