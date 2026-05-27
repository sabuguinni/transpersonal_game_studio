#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Combat_DinosaurAIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Combat AI properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    float PatrolSpeed;

    // AI behavior functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StopHunting();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PatrolArea();

private:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    AActor* CurrentTarget;
    bool bIsHunting;
    FVector PatrolCenter;
    float PatrolRadius;
};