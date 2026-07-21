#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "SharedTypes.h"
#include "DinosaurAIController.generated.h"

/**
 * AI Controller para dinossauros com comportamento territorial e de caça
 * Implementa patrulha, detecção de jogador e comportamento baseado na espécie
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Componentes de AI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Trees para diferentes espécies
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* RaptorBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTree* HerbivoreBehaviorTree;

    // Configurações de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float HearingRange;

    // Estado actual
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    EDinosaurSpecies DinosaurSpecies;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    AActor* TargetActor;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    float LastPlayerSightTime;

public:
    // Funções públicas para configuração
    UFUNCTION(BlueprintCallable, Category = "AI Setup")
    void SetDinosaurSpecies(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "AI Setup")
    void SetTerritoryRadius(float Radius);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ReturnToTerritory();

    // Funções de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    float GetDistanceToPlayer() const;

protected:
    // Configuração inicial
    void SetupAIPerception();
    void ConfigureBehaviorForSpecies();
    
    // Lógica de comportamento
    void UpdateBehaviorState();
    void HandlePatrolBehavior();
    void HandleChaseBehavior();
    void HandleIdleBehavior();
    
    // Utilitários
    FVector GetRandomPatrolPoint() const;
    bool IsInTerritory(const FVector& Location) const;
    void UpdateBlackboardValues();
};