#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class ACrowdSpawnPoint;
class ACrowdWaypoint;

/**
 * Crowd spawn point data structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnPointData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    FString SpawnPointName = TEXT("DefaultSpawnPoint");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    int32 MaxCrowdSize = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Spawn")
    bool bIsActive = true;

    FCrowd_SpawnPointData()
    {
        Location = FVector::ZeroVector;
        SpawnPointName = TEXT("DefaultSpawnPoint");
        MaxCrowdSize = 50;
        SpawnRadius = 500.0f;
        bIsActive = true;
    }
};

/**
 * Crowd LOD level enumeration
 */
UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    High        UMETA(DisplayName = "High LOD"),
    Medium      UMETA(DisplayName = "Medium LOD"),
    Low         UMETA(DisplayName = "Low LOD"),
    Culled      UMETA(DisplayName = "Culled")
};

/**
 * Crowd pathfinding connection data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Pathfinding")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Pathfinding")
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Pathfinding")
    float PathWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Pathfinding")
    bool bIsBidirectional = true;

    FCrowd_PathConnection()
    {
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector::ZeroVector;
        PathWeight = 1.0f;
        bIsBidirectional = true;
    }
};

/**
 * Mass Entity Crowd Simulation Subsystem
 * Manages large-scale crowd simulation using UE5's Mass Entity framework
 * Handles up to 50,000 simultaneous crowd agents with LOD optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Crowd spawn point management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterSpawnPoint(const FCrowd_SpawnPointData& SpawnPointData);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UnregisterSpawnPoint(const FString& SpawnPointName);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_SpawnPointData> GetActiveSpawnPoints() const;

    // Crowd pathfinding network
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void AddPathConnection(const FCrowd_PathConnection& PathConnection);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RemovePathConnection(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FVector> FindPath(const FVector& StartLocation, const FVector& EndLocation) const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    ECrowd_LODLevel CalculateLODLevel(const FVector& CrowdAgentLocation, const FVector& ViewerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetLODDistances(float HighLODDistance, float MediumLODDistance, float LowLODDistance);

    // Mass Entity integration
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdAtLocation(const FVector& Location, int32 CrowdSize = 20);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnAllCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    float GetCrowdSimulationFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetMaxCrowdAgents(int32 MaxAgents);

protected:
    // Crowd spawn points registry
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_SpawnPointData> SpawnPoints;

    // Pathfinding network
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_PathConnection> PathConnections;

    // LOD configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float MediumLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float LowLODDistance = 3000.0f;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Performance")
    int32 MaxCrowdAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Performance")
    float UpdateFrequency = 60.0f;

    // Runtime data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Runtime")
    int32 ActiveCrowdCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd Runtime")
    float LastFrameTime = 0.0f;

private:
    // Internal pathfinding helpers
    TArray<FVector> BuildPathGraph() const;
    float CalculatePathDistance(const FVector& Start, const FVector& End) const;
    bool IsValidPathConnection(const FCrowd_PathConnection& Connection) const;

    // LOD optimization helpers
    void UpdateCrowdLOD();
    void CullDistantCrowd(const FVector& ViewerLocation);

    // Mass Entity integration
    void InitializeMassEntitySystem();
    void ShutdownMassEntitySystem();
    bool bMassEntityInitialized = false;
};