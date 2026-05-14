#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BehaviorTree/Services/BTService.h"
#include "BehaviorTree/Decorators/BTDecorator.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

// Forward declarations
class ANPC_DinosaurAIController;
class APawn;
class AActor;

/**
 * Dinosaur-specific behavior tree task for hunting behavior
 * Implements pack coordination and territorial hunting patterns
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_BTTask_DinosaurHunt : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinosaurHunt();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** Maximum hunt distance from territory center */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunt Behavior")
    float MaxHuntDistance = 5000.0f;

    /** Time to spend hunting before giving up */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunt Behavior")
    float HuntDuration = 30.0f;

    /** Speed multiplier during hunt */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunt Behavior")
    float HuntSpeedMultiplier = 1.5f;

private:
    bool FindHuntTarget(UBehaviorTreeComponent& OwnerComp, AActor*& OutTarget);
    bool IsValidHuntTarget(AActor* Target, APawn* Hunter);
    void SetHuntBlackboardKeys(UBehaviorTreeComponent& OwnerComp, AActor* Target);
};

/**
 * Dinosaur patrol behavior task
 * Implements territory patrol patterns with randomized waypoints
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_BTTask_DinosaurPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_DinosaurPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** Patrol radius around territory center */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol Behavior")
    float PatrolRadius = 3000.0f;

    /** Time to wait at each patrol point */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol Behavior")
    float WaitTime = 5.0f;

    /** Walking speed during patrol */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol Behavior")
    float PatrolSpeed = 200.0f;

private:
    FVector GeneratePatrolPoint(const FVector& TerritoryCenter, float Radius);
    bool HasReachedPatrolPoint(APawn* Pawn, const FVector& TargetLocation);
};

/**
 * Dinosaur pack coordination service
 * Manages pack behavior and communication between pack members
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_BTService_PackCoordination : public UBTService
{
    GENERATED_BODY()

public:
    UNPC_BTService_PackCoordination();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual FString GetStaticDescription() const override;

protected:
    /** Range to search for pack members */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    float PackDetectionRange = 2000.0f;

    /** Maximum pack size */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    int32 MaxPackSize = 6;

    /** Pack coordination update frequency */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    float CoordinationUpdateInterval = 2.0f;

private:
    void UpdatePackMembers(UBehaviorTreeComponent& OwnerComp);
    void ShareTargetInformation(UBehaviorTreeComponent& OwnerComp);
    TArray<APawn*> FindNearbyPackMembers(APawn* SelfPawn);
};

/**
 * Dinosaur fear response decorator
 * Checks if dinosaur should flee based on threat assessment
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_BTDecorator_FearResponse : public UBTDecorator
{
    GENERATED_BODY()

public:
    UNPC_BTDecorator_FearResponse();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
    virtual FString GetStaticDescription() const override;

protected:
    /** Fear threshold (0.0 = fearless, 1.0 = always afraid) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear Response", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearThreshold = 0.3f;

    /** Distance at which large predators trigger fear */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear Response")
    float PredatorFearDistance = 1500.0f;

    /** Health percentage below which fear increases */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear Response", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LowHealthThreshold = 0.4f;

private:
    float CalculateFearLevel(UBehaviorTreeComponent& OwnerComp) const;
    bool IsLargePredatorNearby(APawn* SelfPawn) const;
};

/**
 * Dinosaur behavior tree manager
 * Handles behavior tree assignment and switching based on dinosaur species and state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTreeManager : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTreeManager();

    /** Assign appropriate behavior tree based on dinosaur species */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    UBehaviorTree* GetBehaviorTreeForSpecies(ENPC_DinosaurSpecies Species, ENPC_DinosaurRole Role);

    /** Create and configure blackboard for dinosaur AI */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void ConfigureBlackboardForDinosaur(UBlackboardComponent* Blackboard, ENPC_DinosaurSpecies Species);

    /** Update behavior tree based on current state */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur AI")
    void UpdateBehaviorTreeForState(ANPC_DinosaurAIController* Controller, ENPC_DinosaurBehaviorState NewState);

protected:
    /** Behavior trees for different dinosaur species */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Trees")
    TMap<ENPC_DinosaurSpecies, UBehaviorTree*> SpeciesBehaviorTrees;

    /** Behavior trees for different roles */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Trees")
    TMap<ENPC_DinosaurRole, UBehaviorTree*> RoleBehaviorTrees;

    /** Default behavior tree for unknown species */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Trees")
    UBehaviorTree* DefaultBehaviorTree;

private:
    void InitializeDefaultBehaviorTrees();
    UBehaviorTree* CreateBehaviorTreeForSpecies(ENPC_DinosaurSpecies Species);
};

/**
 * Blackboard keys used by dinosaur AI
 */
UCLASS()
class TRANSPERSONALGAME_API UNPC_DinosaurBlackboardKeys : public UObject
{
    GENERATED_BODY()

public:
    // Target and navigation keys
    static const FName TargetActorKey;
    static const FName TargetLocationKey;
    static const FName PatrolPointKey;
    static const FName HomeLocationKey;
    static const FName LastKnownPlayerLocationKey;

    // State keys
    static const FName CurrentStateKey;
    static const FName PreviousStateKey;
    static const FName AlertLevelKey;
    static const FName HealthPercentageKey;
    static const FName StaminaPercentageKey;

    // Pack and social keys
    static const FName PackMembersKey;
    static const FName PackLeaderKey;
    static const FName IsPackLeaderKey;
    static const FName PackTargetKey;

    // Territory and environment keys
    static const FName TerritoryRadiusKey;
    static const FName CanSeePlayerKey;
    static const FName CanHearPlayerKey;
    static const FName LastSeenPlayerTimeKey;
    static const FName LastHeardPlayerTimeKey;

    // Behavior modifiers
    static const FName AggressionLevelKey;
    static const FName FearLevelKey;
    static const FName HungerLevelKey;
    static const FName IsRestingKey;
    static const FName IsFeedingKey;
};