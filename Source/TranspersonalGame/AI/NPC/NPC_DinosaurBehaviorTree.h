#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

// Forward declarations
class APawn;
class ACharacter;
class UBlackboardComponent;

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSincePlayerSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPatrolIndex;

    FNPC_DinosaurMemory()
    {
        LastPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        ThreatLevel = 0.0f;
        CurrentPatrolIndex = 0;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange;

    FNPC_DinosaurStats()
    {
        Hunger = 50.0f;
        Energy = 100.0f;
        Aggression = 30.0f;
        Fear = 10.0f;
        PatrolRadius = 5000.0f;
        DetectionRange = 3000.0f;
        AttackRange = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    UBlackboardComponent* BlackboardComp;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeBehavior(APawn* OwnerPawn);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer(APawn* OwnerPawn, APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    float GetDistanceToPlayer(APawn* OwnerPawn, APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemory(APawn* Player, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetupSpeciesBehavior();

private:
    void SetupTRexBehavior();
    void SetupVelociraptorBehavior();
    void SetupTriceratopsBehavior();
    void SetupBrachiosaurusBehavior();

    UPROPERTY()
    float StateTimer;

    UPROPERTY()
    FVector HomeLocation;
};

// Behavior Tree Task Nodes
UCLASS()
class TRANSPERSONALGAME_API UNPC_BTTask_Patrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_Patrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

UCLASS()
class TRANSPERSONALGAME_API UNPC_BTTask_ChasePlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_ChasePlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

UCLASS()
class TRANSPERSONALGAME_API UNPC_BTTask_AttackPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UNPC_BTTask_AttackPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

UCLASS()
class TRANSPERSONALGAME_API UNPC_BTDecorator_CanSeePlayer : public UBTDecorator
{
    GENERATED_BODY()

public:
    UNPC_BTDecorator_CanSeePlayer();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, Category = "Detection")
    float SightRange;

    UPROPERTY(EditAnywhere, Category = "Detection")
    float SightAngle;
};