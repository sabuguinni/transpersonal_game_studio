#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityCrowdSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_CrowdBehaviorState : uint8
{
    Idle = 0,
    Wandering = 1,
    Following = 2,
    Fleeing = 3,
    Gathering = 4,
    Dispersing = 5
};

USTRUCT(BlueprintType)
struct FCrowd_CrowdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float Speed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    ECrowd_CrowdBehaviorState BehaviorState = ECrowd_CrowdBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_FlockingParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float NeighborRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 500.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityCrowdSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityCrowdSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    int32 MaxCrowdSize = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    FCrowd_FlockingParameters FlockingParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    TArray<FCrowd_CrowdAgent> CrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    bool bEnableFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    bool bEnablePathfinding = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd System")
    bool bEnableLODSystem = true;

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void SpawnCrowdAgents(int32 NumAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void UpdateCrowdBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void SetCrowdBehaviorState(ECrowd_CrowdBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    FVector CalculateFlockingForce(const FCrowd_CrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    FVector CalculateSeparationForce(const FCrowd_CrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    FVector CalculateAlignmentForce(const FCrowd_CrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    FVector CalculateCohesionForce(const FCrowd_CrowdAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    void UpdateLODSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd System")
    int32 GetActiveCrowdAgentCount() const;

private:
    float LODUpdateTimer = 0.0f;
    float LODUpdateInterval = 0.5f;
    
    TArray<int32> HighDetailAgents;
    TArray<int32> MediumDetailAgents;
    TArray<int32> LowDetailAgents;
};