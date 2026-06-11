#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/Engine.h"
#include "DinosaurBehaviorTree.generated.h"

/**
 * Dinosaur-specific behavior tree controller with species-based behavior patterns
 * Handles territorial behavior, pack dynamics, hunting patterns, and player interaction
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UDinosaurBehaviorTree : public UBehaviorTreeComponent
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorTree();

    // Core behavior states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    class UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    class UBehaviorTree* RaptorBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Behavior")
    class UBehaviorTree* HerbivoreBehaviorTree;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float DetectionRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Parameters")
    float PackCoordinationRange = 1500.0f;

    // Behavior state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    bool bIsHunting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    bool bIsPatrolling = true;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    bool bIsInPack = false;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior State")
    FVector TerritoryCenter = FVector::ZeroVector;

    // Behavior methods
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeDinosaurBehavior(const FString& DinosaurType, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBehaviorState(const FString& NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdateTerritoryCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInDetectionRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    FVector GetRandomPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartHuntingBehavior(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StopHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void UpdatePackBehavior();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Internal behavior tracking
    float LastBehaviorUpdate = 0.0f;
    float BehaviorUpdateInterval = 1.0f;
    
    // Pack behavior data
    TArray<AActor*> PackMembers;
    AActor* PackLeader = nullptr;
    
    // Territory patrol data
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex = 0;
    
    // Helper methods
    void UpdatePatrolBehavior();
    void UpdateHuntingBehavior();
    void UpdatePackCoordination();
    void GeneratePatrolPoints();
    AActor* FindNearestPlayer() const;
    float GetDistanceToPlayer() const;
};