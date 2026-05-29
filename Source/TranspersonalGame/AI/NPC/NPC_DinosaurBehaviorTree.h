#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Hunt        UMETA(DisplayName = "Hunt"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastFeedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsInTerritory;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        LastPlayerSightTime = 0.0f;
        CurrentPatrolIndex = 0;
        LastFeedTime = 0.0f;
        bIsInTerritory = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* VelociraptorBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* HerbivoreBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBlackboardData* DinosaurBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_DinosaurMemory DinosaurMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeBehaviorTree(class AAIController* AIController);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateDinosaurState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer(class APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddPatrolPoint(FVector Point);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearMemory();

protected:
    UPROPERTY()
    FVector HomeLocation;

    UPROPERTY()
    float StateChangeTime;

    UFUNCTION()
    void OnStateChanged(ENPC_DinosaurState OldState, ENPC_DinosaurState NewState);
};