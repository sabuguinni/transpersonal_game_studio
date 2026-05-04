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

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPCDinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange;

    // Estado actual
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector PatrolCenter;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentPatrolIndex;

public:
    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void GeneratePatrolPoints();

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetDistanceToTarget(AActor* Target) const;

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Configuração inicial
    void SetupPerception();
    void SetupBehaviorTree();
    void ConfigureForSpecies();
};