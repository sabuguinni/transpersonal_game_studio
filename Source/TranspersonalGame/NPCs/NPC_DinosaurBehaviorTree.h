#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "../Core/SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    EDinosaurSpecies Species = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsHunting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    bool bIsPatrolling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float LastSeenTargetTime = 0.0f;

    FNPC_DinosaurState()
    {
        Health = 100.0f;
        Hunger = 50.0f;
        Aggression = 30.0f;
        bIsHunting = false;
        bIsPatrolling = true;
        TerritoryRadius = 5000.0f;
        CurrentTarget = nullptr;
        LastSeenTargetTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeBehaviorTree(AAIController* AIController, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdateDinosaurState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetHuntingTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ClearHuntingTarget();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    FVector GetNextPatrolPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void OnTargetSpotted(AActor* Target);

    // Species-specific behaviors
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ExecuteTRexBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ExecuteRaptorBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ExecuteBrachiosaurusBehavior(float DeltaTime);

    // Pack behavior for raptors
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void JoinPack(UNPC_DinosaurBehaviorTree* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackHunt(AActor* Target);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    FNPC_DinosaurState DinosaurState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Components")
    TWeakObjectPtr<AAIController> OwnerController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Components")
    TWeakObjectPtr<UBlackboardComponent> BlackboardComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Components")
    TWeakObjectPtr<UBehaviorTreeComponent> BehaviorTreeComp;

    // Pack system for raptors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack System")
    TArray<TWeakObjectPtr<UNPC_DinosaurBehaviorTree>> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack System")
    TWeakObjectPtr<UNPC_DinosaurBehaviorTree> PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack System")
    bool bIsPackLeader = false;

    // Behavior timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastBehaviorUpdate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BehaviorUpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float PatrolPointChangeTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastPatrolChange = 0.0f;

private:
    void UpdateHungerAndAggression(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    FVector GenerateRandomPatrolPoint() const;
    bool IsValidPatrolPoint(const FVector& Point) const;
};