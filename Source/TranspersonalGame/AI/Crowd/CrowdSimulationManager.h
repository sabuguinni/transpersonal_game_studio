#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "MassEntityTypes.h"
#include "MassSpawnerTypes.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TranspersonalGame/SharedTypes.h"
#include "CrowdSimulationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Speed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsAlive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Fear = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 100.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    FCrowd_FlockingParams FlockingParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> CrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    UStaticMesh* AgentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<UStaticMeshComponent*> AgentMeshComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bUseFlocking = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    bool bAvoidPlayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float PlayerAvoidanceRadius = 800.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnAgents(int32 NumAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateFlocking(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateSeparation(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateAlignment(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculateCohesion(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    FVector CalculatePlayerAvoidance(int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentPositions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetAgentType(int32 AgentIndex, ECrowd_AgentType NewType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllAgents();

private:
    FVector GetPlayerLocation() const;
    void CreateAgentMeshComponent(int32 AgentIndex);
    void UpdateAgentMeshTransform(int32 AgentIndex);
};