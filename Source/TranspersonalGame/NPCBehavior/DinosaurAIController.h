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
 * AI Controller para dinossauros - controla comportamento, perceção e tomada de decisões
 * Suporta diferentes espécies com comportamentos únicos (T-Rex, Raptor, Herbívoros)
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
    virtual void OnPossess(APawn* InPawn) override;

    // Componente de comportamento
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    // Componente de perceção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração de visão
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISightConfig* SightConfig;

public:
    // Behavior Tree para este dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    // Blackboard para armazenar dados de AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Tipo de dinossauro (determina comportamento)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur")
    ENPC_DinosaurSpecies DinosaurSpecies;

    // Estados de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    // Configurações de perceção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float FieldOfViewAngle;

    // Configurações de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PatrolRadius;

    // Configurações de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel;

    // Referências importantes
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    FVector CurrentPatrolPoint;

protected:
    // Callbacks de perceção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Funções de comportamento
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    // Funções de movimento
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementSpeed(float Speed);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsAtLocation(FVector TargetLocation, float Tolerance = 100.0f);

    // Funções de combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackTarget(AActor* Target);

    // Funções de utilidade
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardValue(const FString& KeyName, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardVector(const FString& KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBlackboardActor(const FString& KeyName, AActor* Actor);

private:
    // Configurar perceção baseada na espécie
    void ConfigurePerceptionForSpecies();

    // Configurar comportamento baseado na espécie
    void ConfigureBehaviorForSpecies();

    // Timer para patrulhamento
    FTimerHandle PatrolTimerHandle;

    // Timer para verificação de estado
    FTimerHandle StateCheckTimerHandle;

    // Última vez que viu o alvo
    float LastTargetSeenTime;

    // Tempo máximo para perseguir sem ver o alvo
    float MaxChaseTime;
};