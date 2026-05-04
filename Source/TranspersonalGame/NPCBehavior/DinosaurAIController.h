#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "DinosaurAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;

/**
 * AI Controller para dinossauros com comportamento baseado em Behavior Trees
 * Implementa percepção visual e auditiva, estados de comportamento e reações a estímulos
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Behavior Tree e Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Configurações de visão
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
    float SightRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
    float LoseSightRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Sight")
    float PeripheralVisionAngleDegrees;

    // Configurações de audição
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Hearing")
    float HearingRange;

    // Estado atual do dinossauro
    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    // Tipo de dinossauro
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Type")
    ENPC_DinosaurSpecies DinosaurSpecies;

    // Agressividade (0.0 = pacífico, 1.0 = muito agressivo)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    // Curiosidade (0.0 = ignora estímulos, 1.0 = investiga tudo)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    // Sociabilidade (0.0 = solitário, 1.0 = sempre em grupo)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

public:
    // Funções públicas para comportamento
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPC_DinosaurBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartFleeing(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ReturnToPatrol();

    // Funções de percepção
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool CanSeeActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToActor(AActor* Actor) const;

    // Configuração específica por espécie
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ConfigureForSpecies(ENPC_DinosaurSpecies Species);

protected:
    // Configuração de percepção
    void SetupPerception();
    
    // Configuração de Behavior Tree
    void StartBehaviorTree();

    // Atualização de blackboard
    void UpdateBlackboard();

    // Funções auxiliares
    AActor* FindNearestThreat() const;
    AActor* FindNearestPrey() const;
    TArray<AActor*> FindNearbyAllies() const;
};