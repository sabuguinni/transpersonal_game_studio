#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "DinosaurAIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior Tree para este dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração de visão
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISightConfig* SightConfig;

    // Raio de patrulha
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PatrolRadius;

    // Velocidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MovementSpeed;

    // Distância de detecção do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float PlayerDetectionRange;

    // Estado actual do AI
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bIsChasing;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bIsPatrolling;

public:
    // Funções públicas para controlo do AI
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChasing(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopChasing();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetPatrolLocation(FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartPatrolling();

    // Callback para percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

protected:
    // Localização inicial para patrulha
    FVector HomeLocation;
    
    // Target actual (jogador ou outro)
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    // Timer para patrulha
    FTimerHandle PatrolTimerHandle;

    // Função para patrulha
    UFUNCTION()
    void PatrolTick();

    // Função para verificar distância ao jogador
    void CheckPlayerDistance();
};