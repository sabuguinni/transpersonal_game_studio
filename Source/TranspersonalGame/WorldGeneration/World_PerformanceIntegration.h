#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "WorldGenerationTypes.h"
#include "World_PerformanceIntegration.generated.h"

class UWorld_TerrainManager;
class UWorld_BiomeManager;
class UWorld_WeatherSystem;
class UPerf_CullingSystem;
class UPerf_DayNightCycle;

USTRUCT(BlueprintType)
struct FWorld_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float WorldGenTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PropsCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceWarning = false;
};

USTRUCT(BlueprintType)
struct FWorld_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableOcclusionCulling = false;
};

USTRUCT(BlueprintType)
struct FWorld_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxActiveChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxActorsPerChunk = 320;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bPreloadAdjacentChunks = true;
};

/**
 * Performance integration system for procedural world generation
 * Coordinates with Agent #4's performance systems to maintain 60fps
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_PerformanceIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_PerformanceIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FWorld_StreamingSettings StreamingSettings;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPropsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval = 2.0f;

    // Component references
    UPROPERTY()
    UWorld_TerrainManager* TerrainManager;

    UPROPERTY()
    UWorld_BiomeManager* BiomeManager;

    UPROPERTY()
    UWorld_WeatherSystem* WeatherSystem;

    UPROPERTY()
    UPerf_CullingSystem* CullingSystem;

    UPROPERTY()
    UPerf_DayNightCycle* DayNightCycle;

private:
    float LastPerformanceCheck = 0.0f;
    TArray<AActor*> CachedActors;
    bool bPerformanceOptimizationActive = false;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorld_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeWorldGeneration();

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateActorLOD(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void ApplyLODSettingsToActor(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    int32 CalculateLODLevel(float Distance) const;

    // Streaming management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void ManageWorldStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadChunk(const FVector& ChunkLocation);

    // Integration with performance systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void InitializePerformanceSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SyncWithCullingSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SyncWithDayNightCycle();

    // Actor management
    UFUNCTION(BlueprintCallable, Category = "Actors")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Actors")
    void PrioritizeActors(TArray<AActor*>& Actors);

    UFUNCTION(BlueprintCallable, Category = "Actors")
    bool CanSpawnNewActor(const FString& ActorType) const;

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawPerformanceDebugInfo() const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void TestPerformanceIntegration();
};