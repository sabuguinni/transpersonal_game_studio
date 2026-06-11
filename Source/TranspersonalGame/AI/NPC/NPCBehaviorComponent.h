#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Interacting UMETA(DisplayName = "Interacting"),
    Working UMETA(DisplayName = "Working"),
    Sleeping UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    FNPC_BehaviorMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        bPlayerIsHostile = false;
        CurrentPatrolIndex = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_BehaviorMemory BehaviorMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Courage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdatePatrol();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartFleeing();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartInvestigating(const FVector& Location);

private:
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleInvestigatingState(float DeltaTime);
    void HandleFleeingState(float DeltaTime);

    AActor* GetPlayerActor();
    float GetDistanceToPlayer();
    bool IsPlayerInRange(float Range);
    FVector GetRandomPatrolPoint();
};