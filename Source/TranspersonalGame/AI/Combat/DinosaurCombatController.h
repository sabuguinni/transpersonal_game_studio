#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTree;
class UCombatSpeciesData;
class UEnvironmentalQueryContext;

/**
 * Controller especializado para IA de combate de dinossauros
 * Implementa comportamento tático adaptativo baseado em personalidade e contexto
 */
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

    // Componentes de IA
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Dados da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    UCombatSpeciesData* SpeciesData;

    // Behavior Tree principal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    // Personalidade individual (gerada proceduralmente)
    UPROPERTY(BlueprintReadOnly, Category = "Personality")
    FCombatPersonality IndividualPersonality;

    // Memória de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    FCombatMemory CombatMemory;

    // Estado atual de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState;

    // Padrão de ataque atual
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAttackPattern CurrentAttackPattern;

public:
    // Funções de inicialização
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI(UCombatSpeciesData* InSpeciesData);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void GenerateIndividualPersonality();

    // Funções de percepção
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION()
    void OnTargetPerceptionForgotten(AActor* Actor);

    // Funções de estado de combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateAlertLevel(float DeltaAlert);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterDamage(float Damage, AActor* DamageSource);

    // Funções de tomada de decisão
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    EAttackPattern SelectAttackPattern();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindOptimalCombatPosition();

    // Funções de coordenação de grupo
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void NotifyNearbyAllies(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<ADinosaurCombatController*> GetNearbyAllies(float Radius = 1000.0f);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FCombatPersonality GetPersonality() const { return IndividualPersonality; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetAlertLevel() const { return CombatMemory.AlertLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    bool IsHostileToPlayer() const;

private:
    // Funções internas de atualização
    void UpdateCombatMemory(float DeltaTime);
    void UpdateBehaviorTreeKeys();
    void ProcessPerceptionData();
    
    // Timer para redução gradual do alerta
    FTimerHandle AlertDecayTimer;
    void DecayAlertLevel();

    // Cache de alvos percebidos
    UPROPERTY()
    TArray<AActor*> PerceivedTargets;

    // Última posição de patrulha
    FVector LastPatrolLocation;
    
    // Tempo desde a última decisão tática
    float TimeSinceLastDecision;
    
    // Cooldown entre ataques
    float AttackCooldown;
};