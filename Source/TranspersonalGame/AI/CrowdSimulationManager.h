// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: tribe members + dinosaur herds

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// Stats struct — must be at global scope (RULE 1)
USTRUCT(BlueprintType)
struct FCrowd_SimulationStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 TribeAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 HerdAgentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 RaptorPackCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 TotalAgents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    int32 MaxCapacity = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsActive = false;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent registration
    UFUNCTION(BlueprintCallable, Category = "Crowd|Tribe")
    void RegisterTribeAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerdAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Pack")
    void RegisterRaptorPackAgent(AActor* Agent);

    // Flocking computation (Craig Reynolds boids model)
    UFUNCTION(BlueprintCallable, Category = "Crowd|Flocking")
    FVector ComputeFlockingVector(AActor* Agent, const TArray<AActor*>& Group) const;

    // Stats
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stats")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stats")
    FCrowd_SimulationStats GetSimulationStats() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxTribeAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxRaptorPackAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float FlockingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius;

private:
    UPROPERTY()
    TArray<AActor*> TribeAgents;

    UPROPERTY()
    TArray<AActor*> HerdAgents;

    UPROPERTY()
    TArray<AActor*> RaptorPackAgents;

    UPROPERTY()
    bool bSimulationActive;
};
