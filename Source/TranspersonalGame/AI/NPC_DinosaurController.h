#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurController.generated.h"

class UNPC_BehaviorTreeManager;
class UBehaviorTree;
class UBlackboardData;

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ANPC_DinosaurController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_DinosaurController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Possess(APawn* InPawn) override;
    virtual void UnPossess() override;

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

public:
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurSpecies(ENPCSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    ENPCSpecies GetDinosaurSpecies() const { return DinosaurSpecies; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    ENPCBehaviorState GetBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TriggerPackAlert(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetPatrolRoute(const TArray<FVector>& PatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void FleeFromThreat(const FVector& ThreatLocation);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    TSoftObjectPtr<UBehaviorTree> DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    TSoftObjectPtr<UBlackboardData> DefaultBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    ENPCSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float FieldOfView;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    int32 CurrentPatrolIndex;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    TWeakObjectPtr<AActor> CurrentTarget;

private:
    void InitializePerception();
    void SetupBehaviorTree();
    void UpdateBlackboardValues();
    
    UNPC_BehaviorTreeManager* GetBehaviorManager() const;
};