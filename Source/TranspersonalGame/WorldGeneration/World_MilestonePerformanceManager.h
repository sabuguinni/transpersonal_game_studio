#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "Engine/LevelStreamingVolume.h"
#include "../SharedTypes.h"
#include "World_MilestonePerformanceManager.generated.h"

/**
 * MILESTONE 1 Performance Manager for World Generation
 * Ensures 60fps during character movement through optimized terrain and biome streaming
 * Integrates with Performance Optimizer's physics profiler for coordinated optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MilestonePerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MilestonePerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === MILESTONE 1 PERFORMANCE TARGETS ===
    
    /** Target FPS for character movement (60fps minimum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS = 60.0f;
    
    /** Maximum terrain complexity for smooth movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxTerrainComplexity = 1000;
    
    /** Biome streaming distance for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float BiomeStreamingDistance = 2000.0f;

    // === LANDSCAPE OPTIMIZATION ===
    
    /** Main landscape reference for LOD management */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    TObjectPtr<ALandscapeProxy> MainLandscape;
    
    /** Current landscape LOD level */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landscape")
    int32 CurrentLandscapeLOD = 0;
    
    /** Landscape LOD distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    TArray<float> LandscapeLODDistances = {500.0f, 1500.0f, 3000.0f, 5000.0f};

    // === BIOME STREAMING SYSTEM ===
    
    /** Active biome streaming volumes */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Streaming")
    TArray<TObjectPtr<ALevelStreamingVolume>> BiomeStreamingVolumes;
    
    /** Current active biome */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Streaming")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Forest;
    
    /** Biome transition progress (0-1) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Streaming")
    float BiomeTransitionProgress = 0.0f;

    // === WATER SYSTEM OPTIMIZATION ===
    
    /** Water bodies with LOD management */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water System")
    TArray<TObjectPtr<AActor>> WaterBodies;
    
    /** Water rendering distance for performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterRenderDistance = 3000.0f;
    
    /** Water reflection quality (0-3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System", meta = (ClampMin = "0", ClampMax = "3"))
    int32 WaterReflectionQuality = 2;

    // === PERFORMANCE MONITORING ===
    
    /** Current frame time in milliseconds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFrameTime = 0.0f;
    
    /** Current FPS */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFPS = 60.0f;
    
    /** Performance optimization active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    bool bPerformanceOptimizationActive = false;
    
    /** Frame time history for averaging */
    UPROPERTY()
    TArray<float> FrameTimeHistory;

public:
    // === MILESTONE 1 CORE FUNCTIONS ===
    
    /** Initialize performance-optimized world generation for character movement */
    UFUNCTION(BlueprintCallable, Category = "Milestone Performance")
    void InitializeMilestonePerformance();
    
    /** Update performance optimization based on character position */
    UFUNCTION(BlueprintCallable, Category = "Milestone Performance")
    void UpdatePerformanceOptimization(const FVector& CharacterLocation);
    
    /** Validate MILESTONE 1 performance targets */
    UFUNCTION(BlueprintCallable, Category = "Milestone Performance")
    bool ValidateMilestonePerformance() const;

    // === LANDSCAPE OPTIMIZATION ===
    
    /** Optimize landscape LOD based on distance */
    UFUNCTION(BlueprintCallable, Category = "Landscape Optimization")
    void OptimizeLandscapeLOD(const FVector& ViewerLocation);
    
    /** Set landscape complexity for performance */
    UFUNCTION(BlueprintCallable, Category = "Landscape Optimization")
    void SetLandscapeComplexity(int32 ComplexityLevel);

    // === BIOME STREAMING ===
    
    /** Update biome streaming based on character position */
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateBiomeStreaming(const FVector& CharacterLocation);
    
    /** Get optimal biome for current location */
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    EWorld_BiomeType GetOptimalBiome(const FVector& Location) const;
    
    /** Trigger biome transition */
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void TriggerBiomeTransition(EWorld_BiomeType NewBiome);

    // === WATER SYSTEM ===
    
    /** Optimize water rendering for performance */
    UFUNCTION(BlueprintCallable, Category = "Water Optimization")
    void OptimizeWaterRendering(const FVector& ViewerLocation);
    
    /** Set water quality level */
    UFUNCTION(BlueprintCallable, Category = "Water Optimization")
    void SetWaterQuality(int32 QualityLevel);

    // === PERFORMANCE INTEGRATION ===
    
    /** Get current performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    FWorld_PerformanceMetrics GetPerformanceMetrics() const;
    
    /** Apply performance optimization settings */
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    void ApplyPerformanceSettings(const FWorld_PerformanceSettings& Settings);
    
    /** Check if performance optimization is needed */
    UFUNCTION(BlueprintCallable, Category = "Performance Integration")
    bool ShouldOptimizePerformance() const;

private:
    // === INTERNAL OPTIMIZATION ===
    
    /** Update frame time tracking */
    void UpdateFrameTimeTracking(float DeltaTime);
    
    /** Calculate average FPS */
    float CalculateAverageFPS() const;
    
    /** Apply automatic performance adjustments */
    void ApplyAutomaticOptimizations();
    
    /** Validate performance targets */
    bool IsPerformanceTargetMet() const;
    
    /** Get distance-based LOD level */
    int32 GetLODLevel(float Distance) const;
    
    /** Update streaming volume visibility */
    void UpdateStreamingVolumeVisibility(const FVector& ViewerLocation);
    
    /** Optimize actor culling for performance */
    void OptimizeActorCulling(const FVector& ViewerLocation);
};