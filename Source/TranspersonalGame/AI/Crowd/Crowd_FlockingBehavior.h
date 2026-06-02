#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Crowd_FlockingBehavior.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 200.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Acceleration = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 FlockID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float AgentRadius = 50.0f;
};

/**
 * Flocking behavior component for crowd simulation
 * Implements Reynolds flocking algorithm for natural group movement
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_FlockingBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_FlockingBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Flocking Management
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void InitializeFlocking(const FCrowd_FlockingRules& Rules);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void AddFlockingAgent(const FCrowd_FlockingAgent& Agent);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void RemoveFlockingAgent(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    void UpdateFlockingRules(const FCrowd_FlockingRules& NewRules);

    // Flocking Calculations
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateSeparation(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateAlignment(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FVector CalculateCohesion(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors);

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    TArray<FCrowd_FlockingAgent> FindNeighbors(int32 AgentIndex, float SearchRadius);

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Flocking")
    int32 GetFlockingAgentCount() const { return FlockingAgents.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Flocking")
    FCrowd_FlockingRules GetFlockingRules() const { return FlockingRules; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking", meta = (AllowPrivateAccess = "true"))
    FCrowd_FlockingRules FlockingRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_FlockingAgent> FlockingAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking", meta = (AllowPrivateAccess = "true"))
    bool bFlockingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking", meta = (AllowPrivateAccess = "true"))
    float UpdateFrequency = 30.0f;

private:
    float LastUpdateTime = 0.0f;
    
    void UpdateFlockingBehavior(float DeltaTime);
    FVector LimitVector(const FVector& Vector, float MaxMagnitude);
    void ApplyFlockingForces(int32 AgentIndex, float DeltaTime);
};