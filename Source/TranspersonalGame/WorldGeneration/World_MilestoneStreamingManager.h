#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "../SharedTypes.h"
#include "World_MilestoneStreamingManager.generated.h"

/**
 * MILESTONE 1 Streaming Manager - Performance-optimized world streaming for "Walk Around" gameplay
 * Integrates with Performance Optimizer to maintain stable 60 FPS during terrain exploration
 * Handles biome loading/unloading based on player proximity and performance budgets
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MilestoneStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MilestoneStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === MILESTONE 1 CORE PROPERTIES ===
    
    /** Target frame rate for streaming decisions (syncs with Performance Optimizer) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Performance")
    float TargetFrameRate = 60.0f;
    
    /** Maximum streaming distance for terrain chunks (optimized for walk-around gameplay) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Streaming")
    float MaxStreamingDistance = 5000.0f;
    
    /** Minimum streaming distance to prevent pop-in during character movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Streaming")
    float MinStreamingDistance = 1000.0f;
    
    /** Performance budget for streaming operations (milliseconds per frame) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Performance")
    float StreamingBudgetMs = 2.0f;
    
    // === BIOME MANAGEMENT ===
    
    /** Currently loaded biome chunks */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MILESTONE 1 Biomes")
    TArray<FString> LoadedBiomeChunks;
    
    /** Biome streaming priority queue */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MILESTONE 1 Biomes") 
    TArray<FString> BiomeLoadQueue;
    
    /** Maximum number of biome chunks loaded simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Biomes")
    int32 MaxLoadedBiomes = 4;
    
    // === TERRAIN COMPONENTS ===
    
    /** Reference to main landscape actor */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MILESTONE 1 Terrain")
    class ALandscape* MainLandscape;
    
    /** Terrain LOD levels for performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MILESTONE 1 Terrain")
    TArray<float> TerrainLODDistances;
    
    /** Current terrain LOD level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MILESTONE 1 Terrain")
    int32 CurrentTerrainLOD = 0;

public:
    // === MILESTONE 1 CORE FUNCTIONS ===
    
    /** Initialize streaming system for MILESTONE 1 requirements */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1")
    void InitializeMilestoneStreaming();
    
    /** Update streaming based on player position and performance */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1")
    void UpdateStreamingForPlayer(FVector PlayerLocation);
    
    /** Apply performance optimizations for walk-around gameplay */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1")
    void ApplyWalkAroundOptimizations();
    
    /** Check if streaming system is maintaining target frame rate */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MILESTONE 1")
    bool IsPerformanceTargetMet() const;
    
    // === BIOME STREAMING FUNCTIONS ===
    
    /** Load biome chunk at specified location */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Biomes")
    void LoadBiomeChunk(const FString& BiomeName, FVector ChunkLocation);
    
    /** Unload distant biome chunks to maintain performance */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Biomes")
    void UnloadDistantBiomes(FVector PlayerLocation);
    
    /** Get closest biome type to player position */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MILESTONE 1 Biomes")
    FString GetClosestBiomeType(FVector PlayerLocation) const;
    
    /** Check if biome chunk is currently loaded */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MILESTONE 1 Biomes")
    bool IsBiomeChunkLoaded(const FString& BiomeName) const;
    
    // === TERRAIN OPTIMIZATION FUNCTIONS ===
    
    /** Update terrain LOD based on distance and performance */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Terrain")
    void UpdateTerrainLOD(FVector PlayerLocation);
    
    /** Set terrain detail level for performance optimization */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Terrain")
    void SetTerrainDetailLevel(int32 LODLevel);
    
    /** Create basic landscape for MILESTONE 1 if none exists */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "MILESTONE 1 Terrain")
    void CreateMilestoneLandscape();
    
    // === PERFORMANCE INTEGRATION ===
    
    /** Get current streaming performance metrics */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MILESTONE 1 Performance")
    float GetStreamingPerformanceMs() const;
    
    /** Set performance budget from Performance Optimizer */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Performance")
    void SetPerformanceBudget(float BudgetMs);
    
    /** Emergency performance mode - aggressive optimization */
    UFUNCTION(BlueprintCallable, Category = "MILESTONE 1 Performance")
    void EnableEmergencyPerformanceMode();

protected:
    // === INTERNAL TRACKING ===
    
    /** Last player position for streaming calculations */
    FVector LastPlayerPosition;
    
    /** Time of last streaming update */
    float LastStreamingUpdate = 0.0f;
    
    /** Current frame time for performance monitoring */
    float CurrentFrameTimeMs = 0.0f;
    
    /** Emergency performance mode active */
    bool bEmergencyPerformanceMode = false;
    
    /** Streaming update frequency (seconds) */
    float StreamingUpdateFrequency = 1.0f;
    
    // === INTERNAL FUNCTIONS ===
    
    /** Calculate streaming priority for biome chunk */
    float CalculateStreamingPriority(const FString& BiomeName, FVector ChunkLocation, FVector PlayerLocation) const;
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Check if streaming budget allows new operations */
    bool CanPerformStreamingOperation() const;
};