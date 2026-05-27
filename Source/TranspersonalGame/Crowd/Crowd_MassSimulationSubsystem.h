#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "Crowd_MassSimulationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Speed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAgents(int32 NumAgents, FVector SpawnCenter, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdTarget(FVector NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd")
    TArray<FCrowd_AgentData> CrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxCrowdAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AgentSeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MaxSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector GlobalTarget = FVector::ZeroVector;

private:
    void UpdateAgentMovement(FCrowd_AgentData& Agent, float DeltaTime);
    FVector CalculateSteeringForce(const FCrowd_AgentData& Agent);
    FVector CalculateSeparationForce(const FCrowd_AgentData& Agent);
    FVector CalculateSeekForce(const FCrowd_AgentData& Agent);

    bool bIsInitialized = false;
};