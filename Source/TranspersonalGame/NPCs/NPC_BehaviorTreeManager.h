#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "NPC_BehaviorTreeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_DinosaurBehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SightRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HearingRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 PackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RestDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HuntingCooldown = 120.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bIsResting = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UObject
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeDinosaurBehavior(AAIController* AIController, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(AAIController* AIController, ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdatePatrolRoute(AAIController* AIController, const TArray<FNPC_PatrolPoint>& PatrolPoints);

    // Pack behavior for social dinosaurs
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void FormPack(const TArray<AAIController*>& PackMembers, AAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void DisbandPack(AAIController* PackMember);

    // Territory and aggression management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(AAIController* AIController, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(AAIController* AIController, const FVector& Location);

    // Perception and threat assessment
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void ConfigurePerception(AAIController* AIController, const FNPC_DinosaurBehaviorData& BehaviorData);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    float CalculateThreatLevel(AAIController* AIController, AActor* Target);

    // Blackboard key management
    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardVector(UBlackboardComponent* Blackboard, const FName& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardObject(UBlackboardComponent* Blackboard, const FName& KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardBool(UBlackboardComponent* Blackboard, const FName& KeyName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardFloat(UBlackboardComponent* Blackboard, const FName& KeyName, float Value);

protected:
    // Behavior data templates for different species
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species Behavior")
    TMap<EDinosaurSpecies, FNPC_DinosaurBehaviorData> SpeciesBehaviorTemplates;

    // Active pack formations
    UPROPERTY()
    TMap<AAIController*, TArray<AAIController*>> ActivePacks;

    // Territory data
    UPROPERTY()
    TMap<AAIController*, FVector> TerritoryCenters;

    UPROPERTY()
    TMap<AAIController*, float> TerritoryRadii;

private:
    void InitializeSpeciesTemplates();
    void SetupPerceptionComponent(AAIController* AIController, const FNPC_DinosaurBehaviorData& BehaviorData);
    void InitializeBlackboardKeys(UBlackboardComponent* Blackboard);
};