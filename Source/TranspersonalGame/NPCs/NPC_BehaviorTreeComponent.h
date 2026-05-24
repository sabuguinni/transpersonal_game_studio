#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/World.h"
#include "NPC_BehaviorTreeComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Feeding UMETA(DisplayName = "Feeding"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> LastKnownPlayerPositions;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float LastPlayerEncounterTime;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsHostile;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float TerritoryRadius;

    FNPC_BehaviorMemory()
    {
        LastKnownPlayerPositions.Empty();
        LastPlayerEncounterTime = 0.0f;
        bPlayerIsHostile = false;
        HomeLocation = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBlackboardAsset* BlackboardAsset;

    UPROPERTY(BlueprintReadOnly, Category = "AI Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    // NPC Memory System
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FNPC_BehaviorMemory NPCMemory;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Settings")
    TArray<FVector> PatrolPoints;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdatePlayerMemory(FVector PlayerLocation, bool bIsHostile);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    bool IsPlayerInTerritory(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StopBehaviorTree();

private:
    UPROPERTY()
    class AAIController* AIControllerRef;

    UPROPERTY()
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;

    int32 CurrentPatrolIndex;
    float StateChangeTimer;
    float MemoryUpdateTimer;

    void UpdateAIBlackboard();
    void ProcessBehaviorLogic(float DeltaTime);
    AActor* FindNearestPlayer();
};

#include "NPC_BehaviorTreeComponent.generated.h"