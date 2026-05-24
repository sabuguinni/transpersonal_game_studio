#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "Combat_BehaviorTreeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BehaviorTreeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    EDinosaurSpecies TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombatState RequiredCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float Priority;

    FCombat_BehaviorTreeConfig()
    {
        BehaviorTree = nullptr;
        TargetSpecies = EDinosaurSpecies::TRex;
        RequiredCombatState = ECombatState::Neutral;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_AIMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    TArray<FVector> PatrolWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    bool bPlayerIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float ThreatLevel;

    FCombat_AIMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        CurrentWaypointIndex = 0;
        bPlayerIsHostile = false;
        ThreatLevel = 0.0f;
    }
};

/**
 * Combat Behavior Tree Manager
 * Manages behavior trees for combat AI, handles dynamic tree switching,
 * and coordinates AI decision making for dinosaur combat behaviors
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_BehaviorTreeManager : public UObject
{
    GENERATED_BODY()

public:
    UCombat_BehaviorTreeManager();

    // Behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterBehaviorTree(UBehaviorTree* BehaviorTree, EDinosaurSpecies Species, ECombatState CombatState, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    UBehaviorTree* GetBehaviorTreeForState(EDinosaurSpecies Species, ECombatState CombatState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SwitchBehaviorTree(AAIController* AIController, UBehaviorTree* NewBehaviorTree);

    // AI memory management
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void UpdateAIMemory(AAIController* AIController, const FCombat_AIMemory& NewMemory);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    FCombat_AIMemory GetAIMemory(AAIController* AIController);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void SetBlackboardValue(AAIController* AIController, const FString& KeyName, const FString& Value);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void SetBlackboardVector(AAIController* AIController, const FString& KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void SetBlackboardFloat(AAIController* AIController, const FString& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void SetBlackboardBool(AAIController* AIController, const FString& KeyName, bool Value);

    // Combat decision making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombatState DetermineOptimalCombatState(AAIController* AIController, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageInCombat(AAIController* AIController, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateOptimalAttackPosition(AAIController* AIController, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInPackFormation(AAIController* AIController);

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RegisterPackMember(AAIController* PackLeader, AAIController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void RemovePackMember(AAIController* PackLeader, AAIController* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    TArray<AAIController*> GetPackMembers(AAIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack AI")
    void CoordinatePackAttack(AAIController* PackLeader, AActor* Target);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeDefaultBehaviorTrees();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CleanupAIMemory();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    TArray<FCombat_BehaviorTreeConfig> RegisteredBehaviorTrees;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Memory")
    TMap<AAIController*, FCombat_AIMemory> AIMemoryMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack AI")
    TMap<AAIController*, TArray<AAIController*>> PackMembersMap;

private:
    // Internal helper functions
    UBehaviorTree* FindBestBehaviorTree(EDinosaurSpecies Species, ECombatState CombatState);
    void UpdateBlackboardForCombatState(AAIController* AIController, ECombatState NewState);
    float CalculateThreatLevel(AAIController* AIController, AActor* Target);
    bool IsValidAIController(AAIController* AIController);
};