#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "../SharedTypes.h"
#include "NPC_BehaviorTreeManager.generated.h"

class UBehaviorTree;
class UBlackboardData;
class AAIController;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorTreeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSoftObjectPtr<UBlackboardData> BlackboardAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FleeRadius;

    FNPC_BehaviorTreeConfig()
    {
        UpdateInterval = 0.5f;
        DetectionRadius = 1500.0f;
        FleeRadius = 3000.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void RegisterAIController(AAIController* Controller, ENPCSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void UnregisterAIController(AAIController* Controller);

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void UpdateAllBehaviors();

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void SetGlobalThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    FNPC_BehaviorTreeConfig GetBehaviorConfig(ENPCSpecies Species) const;

    UFUNCTION(BlueprintCallable, Category = "NPC AI")
    void TriggerPackBehavior(ENPCSpecies Species, const FVector& Location, ENPCBehaviorState NewState);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    TMap<ENPCSpecies, FNPC_BehaviorTreeConfig> BehaviorConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    TArray<TWeakObjectPtr<AAIController>> RegisteredControllers;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    float GlobalThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FTimerHandle BehaviorUpdateTimer;

private:
    void InitializeDefaultConfigs();
    void UpdateControllerBehavior(AAIController* Controller);
    void CleanupInvalidControllers();
};