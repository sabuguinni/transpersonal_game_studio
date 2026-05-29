#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "../SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    ENPC_DinosaurBehavior CurrentBehavior = ENPC_DinosaurBehavior::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float HungerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float FearLevel = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    float StateTimer = 0.0f;

    FNPC_DinosaurState()
    {
        CurrentBehavior = ENPC_DinosaurBehavior::Idle;
        AggressionLevel = 0.5f;
        HungerLevel = 0.3f;
        FearLevel = 0.1f;
        LastKnownThreatLocation = FVector::ZeroVector;
        CurrentTarget = nullptr;
        StateTimer = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* RaptorBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* HerbivoreBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBlackboardAsset* DinosaurBlackboard;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void InitializeDinosaurAI(APawn* DinosaurPawn, ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdateDinosaurBehavior(APawn* DinosaurPawn, const FNPC_DinosaurState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    FNPC_DinosaurState GetDinosaurState(APawn* DinosaurPawn);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void SetDinosaurTarget(APawn* DinosaurPawn, AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void TriggerDinosaurAlert(APawn* DinosaurPawn, FVector ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    bool IsInPackFormation(APawn* DinosaurPawn);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    TArray<APawn*> GetNearbyPackMembers(APawn* DinosaurPawn, float SearchRadius = 2000.0f);

protected:
    UPROPERTY()
    TMap<APawn*, FNPC_DinosaurState> DinosaurStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float TRexDetectionRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float RaptorDetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float PackCoordinationRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float BehaviorUpdateInterval = 0.5f;

private:
    void CreateDefaultBehaviorTrees();
    void SetupBlackboardKeys(UBlackboardComponent* BlackboardComp, const FNPC_DinosaurState& State);
    ENPC_DinosaurBehavior CalculateOptimalBehavior(const FNPC_DinosaurState& CurrentState, ENPC_DinosaurSpecies Species);
};