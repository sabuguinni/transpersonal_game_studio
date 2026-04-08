#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "CombatAITypes.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class UCombatAIComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetAcquired, AActor*, Target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLost, AActor*, Target);

/**
 * AI Controller especializado para combate de dinossauros
 * Integra Behavior Trees, AI Perception e sistema de combate tático
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

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombatAIComponent* CombatAIComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração de Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI")
    UBlackboardAsset* CombatBlackboard;

    // Configuração de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatAIConfig CombatConfig;

    // Dados de runtime
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombatRuntimeData RuntimeData;

    // Eventos
    UPROPERTY(BlueprintAssignable)
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable)
    FOnTargetAcquired OnTargetAcquired;

    UPROPERTY(BlueprintAssignable)
    FOnTargetLost OnTargetLost;

public:
    // Funções públicas de combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombatState GetCombatState() const { return RuntimeData.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCurrentTarget() const { return RuntimeData.CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(EAttackType AttackType);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RespondToHelpCall(AActor* Caller);

    // Funções de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Funções de análise tática
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateThreatLevel(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindOptimalCombatPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee() const;

    // Funções de Blackboard
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateBlackboardValues();

protected:
    // Funções internas
    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateCombatLogic(float DeltaTime);
    void UpdateAggression(float DeltaTime);
    void UpdateFear(float DeltaTime);
    void ProcessNearbyActors();
    
    // Análise tática
    TArray<AActor*> FindAlliesInRange() const;
    TArray<AActor*> FindEnemiesInRange() const;
    float CalculatePositionalAdvantage(const FVector& Position, AActor* Target) const;
    
    // Blackboard keys (definidas como FName para performance)
    static const FName BB_CurrentTarget;
    static const FName BB_CombatState;
    static const FName BB_LastKnownTargetLocation;
    static const FName BB_CurrentAggression;
    static const FName BB_CurrentFear;
    static const FName BB_InCombat;
    static const FName BB_ShouldRetreat;
    static const FName BB_ShouldFlee;
    static const FName BB_OptimalCombatPosition;
    static const FName BB_AlliesInRange;
    static const FName BB_EnemiesInRange;
};