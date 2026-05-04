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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // Behavior Tree e Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração de visão
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    float SightRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    float LoseSightRadius;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    float MaxAge;

    // Configuração de audição
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
    float HearingRange;

    // Tipo de dinossauro para comportamento específico
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    ENPC_DinosaurType DinosaurType;

    // Estados de comportamento
    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "AI|State")
    int32 CurrentPatrolIndex;

public:
    // Funções públicas para controlo de comportamento
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerInSight() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToPlayer() const;

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Configurar percepção baseada no tipo de dinossauro
    void SetupPerceptionForDinosaurType();

    // Lógica específica por tipo
    void HandleTRexBehavior();
    void HandleRaptorBehavior();
    void HandleHerbivoreBehavior();

private:
    // Timer para updates de comportamento
    FTimerHandle BehaviorUpdateTimer;

    void UpdateBehavior();
};