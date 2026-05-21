#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Crowd_MassSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Montanha    UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Carnivore   UMETA(DisplayName = "Carnivore"),
    Scavenger   UMETA(DisplayName = "Scavenger"),
    Neutral     UMETA(DisplayName = "Neutral")
};

USTRUCT(BlueprintType)
struct FCrowd_BiomeLocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECrowd_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxAgents;

    FCrowd_BiomeLocation()
    {
        BiomeType = ECrowd_BiomeType::Savana;
        Location = FVector::ZeroVector;
        Radius = 10000.0f;
        MaxAgents = 1000;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 GroupID;

    FCrowd_AgentData()
    {
        AgentType = ECrowd_AgentType::Herbivore;
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        Health = 100.0f;
        GroupID = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_BiomeLocation> BiomeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float SimulationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float LODDistance3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 AgentsPerFrame;

private:
    float LastUpdateTime;
    int32 CurrentUpdateIndex;
    TArray<AActor*> SpawnedAgents;

public:
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnAgentsInBiome(ECrowd_BiomeType BiomeType, int32 Count, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateAgentBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ApplyLODCulling(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void MigrateAgentsBetweenBiomes(ECrowd_BiomeType FromBiome, ECrowd_BiomeType ToBiome, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSimulation();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    float GetSimulationPerformance() const;

private:
    void UpdateAgentFlocking(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents);
    void UpdateAgentSeparation(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents);
    void UpdateAgentAlignment(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents);
    void UpdateAgentCohesion(FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& NearbyAgents);
    
    FVector GetBiomeCenter(ECrowd_BiomeType BiomeType) const;
    TArray<FCrowd_AgentData> GetAgentsInRadius(const FVector& Center, float Radius) const;
    void CleanupDeadAgents();
    void SpawnVisualAgent(const FCrowd_AgentData& AgentData);
};