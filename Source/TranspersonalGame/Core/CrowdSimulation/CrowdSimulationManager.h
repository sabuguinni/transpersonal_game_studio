#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState;

    FCrowd_AgentData()
    {
        Location = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        AgentID = 0;
        BehaviorState = ECrowd_BehaviorState::Wandering;
    }
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Wandering UMETA(DisplayName = "Wandering"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Following UMETA(DisplayName = "Following"),
    Gathering UMETA(DisplayName = "Gathering"),
    Hunting UMETA(DisplayName = "Hunting")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float AgentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float FlockingRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> CrowdAgents;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(int32 NumAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentBehavior(int32 AgentIndex, ECrowd_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetFleeTarget(FVector FleeFromLocation);

private:
    void UpdateAgentMovement(float DeltaTime);
    void ApplyFlocking(int32 AgentIndex);
    void ApplyFleeBehavior(int32 AgentIndex, FVector FleeTarget);
    
    FVector FleeTargetLocation;
    bool bIsFleeingActive;
};