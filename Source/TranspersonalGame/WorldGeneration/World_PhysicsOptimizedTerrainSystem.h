#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Landscape/Landscape.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_PhysicsOptimizedTerrainSystem.generated.h"

// Forward declarations
class UWorld_PerformanceIntegration;
class APerf_PhysicsIntegrationMasterOptimizer;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    FVector Scale = FVector(20.0f, 20.0f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    int32 LODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    bool bIsPerformanceOptimized = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    float PerformanceScore = 100.0f;

    FWorld_TerrainChunk()
    {
        Location = FVector::ZeroVector;
        Scale = FVector(20.0f, 20.0f, 2.0f);
        BiomeType = EBiomeType::Forest;
        LODLevel = 0;
        bIsPerformanceOptimized = false;
        PerformanceScore = 100.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTerrainChunks = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval = 1.0f;

    FWorld_PerformanceSettings()
    {
        TargetFPS = 60.0f;
        MinimumFPS = 30.0f;
        MaxTerrainChunks = 50;
        LODDistanceMultiplier = 1.0f;
        bEnableAutoOptimization = true;
        OptimizationCheckInterval = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PhysicsOptimizedTerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PhysicsOptimizedTerrainSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core terrain management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain System")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_PerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIntegrateWithPhysicsOptimizer = true;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentMemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 ActiveTerrainChunks = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 OptimizedTerrainChunks = 0;

    // Physics integration
    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    TWeakObjectPtr<APerf_PhysicsIntegrationMasterOptimizer> PhysicsOptimizer;

    // Timers
    UPROPERTY()
    float PerformanceCheckTimer = 0.0f;

public:
    // Terrain management functions
    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void CreateTerrainChunk(const FVector& Location, EBiomeType BiomeType, const FVector& Scale = FVector(20.0f, 20.0f, 2.0f));

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void RemoveTerrainChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void OptimizeTerrainChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Terrain System")
    void OptimizeAllTerrainChunks();

    // Performance functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldOptimizeTerrain() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(int32 ChunkIndex, int32 LODLevel);

    // Physics integration functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void IntegrateWithPhysicsOptimizer();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdatePhysicsOptimization();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    float GetTerrainPhysicsPerformanceScore() const;

    // Milestone 1 specific functions
    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    void SetupMilestone1Terrain();

    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    bool ValidateMilestone1Performance();

    UFUNCTION(BlueprintCallable, Category = "Milestone 1")
    void CreateWalkAroundTerrain();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetTerrainChunkCount() const { return TerrainChunks.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FWorld_TerrainChunk GetTerrainChunk(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetPerformanceSettings(const FWorld_PerformanceSettings& NewSettings);

private:
    // Internal optimization functions
    void PerformanceOptimizationTick(float DeltaTime);
    void UpdateTerrainLOD();
    void CullDistantTerrainChunks();
    void ValidateTerrainPerformance();
    
    // Physics integration helpers
    void FindPhysicsOptimizer();
    void SyncWithPhysicsOptimizer();
};