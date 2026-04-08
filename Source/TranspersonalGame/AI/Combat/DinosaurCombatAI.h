#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "CombatAITypes.h"
#include "DinosaurCombatAI.generated.h"

class UBehaviorTree;
class UBlackboardAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThreatDetected, AActor*, ThreatActor, float, ThreatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ECombatBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPackFormation, ECombatFormation, Formation, TArray<AActor*>, PackMembers, AActor*, Target);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* PerceptionComponent;

    // Combat Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FDinosaurCombatProfile CombatProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UBlackboardAsset* CombatBlackboard;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    UAISenseConfig_Damage* DamageConfig;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatContext CombatContext;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float CurrentAggression = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float CurrentFear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsInCombat = false;

public:
    // Events
    UPROPERTY(BlueprintAssignable)
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable)
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnPackFormation OnPackFormation;

    // Combat Management
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void EndCombat();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FormPack(const TArray<AActor*>& PackMembers, ECombatFormation Formation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float CalculateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector GetOptimalAttackPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool ShouldAmbush(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateAggression(float DeltaAggression);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateFear(float DeltaFear);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatBehaviorState GetCombatState() const { return CombatContext.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCurrentAggression() const { return CurrentAggression; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    float GetCurrentFear() const { return CurrentFear; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsInCombat() const { return bIsInCombat; }

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Combat Logic
    void ProcessThreatDetection(AActor* Actor);
    void UpdateCombatBehavior(float DeltaTime);
    void HandlePackCoordination();
    void ExecuteFormation(ECombatFormation Formation);
    bool ValidateTarget(AActor* Target) const;
    void UpdateBlackboardValues();

    // Utility Functions
    float GetDistanceToTarget(AActor* Target) const;
    bool IsTargetInAttackRange(AActor* Target) const;
    bool HasLineOfSight(AActor* Target) const;
    FVector PredictTargetLocation(AActor* Target, float PredictionTime) const;
};