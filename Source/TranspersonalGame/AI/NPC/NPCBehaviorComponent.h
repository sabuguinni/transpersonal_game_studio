#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorMode CurrentMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    AActor* TargetActor;

    FNPC_BehaviorState()
    {
        CurrentMode = ENPC_BehaviorMode::Idle;
        StateTimer = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    FVector LastSeenPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 CurrentPatrolIndex;

    FNPC_Memory()
    {
        LastSeenPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 999.0f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_Memory NPCMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AggressionLevel;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorMode(ENPC_BehaviorMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdatePatrolBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateChaseBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPatrolPoint(FVector NewPoint);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ClearPatrolPoints();

private:
    void ProcessSensoryInput(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void ExecuteBehavior(float DeltaTime);
    
    APawn* GetPlayerPawn();
    float GetDistanceToPlayer();
    bool IsPlayerInSightRange();
    bool IsPlayerInHearingRange();
};

#include "NPCBehaviorComponent.generated.h"