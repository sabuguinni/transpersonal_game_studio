#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "CrowdDensityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ZoneExtents = FVector(1000.0f, 1000.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAgents = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinAgents = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRate = 5.0f; // agents per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DespawnDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bActiveZone = true;

    // Zone type affects behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ZoneType = TEXT("Urban");

    // Current state
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentAgentCount = 0;

    UPROPERTY(BlueprintReadOnly)
    float LastSpawnTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdFlowPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathWidth = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowRate = 10.0f; // agents per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBidirectional = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FlowType = TEXT("Pedestrian");
};

/**
 * Manages crowd density and spawning across different zones
 * Handles dynamic population adjustment based on player proximity and performance
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdDensityManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdDensityManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Density")
    void AddSpawnZone(const FCrowdSpawnZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Crowd Density")
    void RemoveSpawnZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd Density")
    void SetZoneActive(int32 ZoneIndex, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Crowd Density")
    void UpdateZoneDensity(int32 ZoneIndex, int32 NewMaxAgents);

    // Flow Path Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Flow")
    void AddFlowPath(const FCrowdFlowPath& NewPath);

    UFUNCTION(BlueprintCallable, Category = "Crowd Flow")
    void SetFlowPathActive(int32 PathIndex, bool bActive);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalDensityMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPlayerProximityScaling(bool bEnable, float MaxDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(bool bHighPerformance);

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetAverageDensity() const;

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    TArray<int32> GetZoneAgentCounts() const;

protected:
    // Spawn zones for different areas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zones")
    TArray<FCrowdSpawnZone> SpawnZones;

    // Flow paths for directed movement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Paths")
    TArray<FCrowdFlowPath> FlowPaths;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GlobalDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUsePlayerProximityScaling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PlayerProximityMaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bHighPerformanceMode = false;

    // Update frequencies
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Rates")
    float DensityUpdateFrequency = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Update Rates")
    float SpawnUpdateFrequency = 10.0f;

    // Maximum limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 GlobalMaxAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxAgentsPerFrame = 100;

private:
    // Internal management
    void UpdateSpawnZones(float DeltaTime);
    void UpdateFlowPaths(float DeltaTime);
    void ManageAgentCulling();
    void UpdatePerformanceMetrics();
    
    // Spawning logic
    void SpawnAgentsInZone(FCrowdSpawnZone& Zone, int32 AgentsToSpawn);
    void SpawnAgentsOnPath(const FCrowdFlowPath& Path, int32 AgentsToSpawn);
    
    // Performance optimization
    float CalculatePlayerDistanceScaling(const FVector& Position);
    bool ShouldSpawnInZone(const FCrowdSpawnZone& Zone);
    
    // Subsystem references
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Timing
    float LastDensityUpdate = 0.0f;
    float LastSpawnUpdate = 0.0f;
    
    // Statistics
    int32 TotalActiveAgents = 0;
    float CurrentAverageDensity = 0.0f;
    
    // Performance tracking
    float LastFrameTime = 0.0f;
    int32 AgentsSpawnedThisFrame = 0;
};