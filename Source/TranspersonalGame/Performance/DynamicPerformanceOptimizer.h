#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceTargets.h"
#include "PerformanceProfiler.h"
#include "DynamicPerformanceOptimizer.generated.h"

/**
 * Dynamic Performance Optimizer
 * 
 * Automatically adjusts game settings in real-time to maintain target framerate
 * Implements Richard Fabian's principle: "A beautiful game that runs at 20fps is a broken game"
 * 
 * Key Features:
 * - Real-time scalability adjustment based on performance metrics
 * - Dinosaur-specific optimizations (LOD, culling, behavior complexity)
 * - Mass AI performance scaling (critical for 50k dinosaur simulation)
 * - Nanite and Lumen optimization for large open worlds
 * - Memory pressure management with streaming optimization
 */

UENUM(BlueprintType)
enum class EOptimizationLevel : uint8
{
    Ultra = 0,      // Maximum quality, no compromises
    High,           // Minor optimizations, maintain visual fidelity
    Medium,         // Balanced quality/performance
    Low,            // Performance priority, acceptable quality loss
    Emergency       // Minimum quality to maintain playability
};

UENUM(BlueprintType)
enum class EOptimizationCategory : uint8
{
    Rendering = 0,
    MassAI,
    Physics,
    Audio,
    Streaming,
    Effects
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FOptimizationSettings
{
    GENERATED_BODY()

    // Rendering optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ScreenPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 ShadowQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 PostProcessQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 EffectsQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 TextureQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 MaxDrawCalls = 5000;

    // Nanite optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    float NaniteTriangleDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nanite")
    int32 NaniteMaxPixelsPerTriangle = 1;

    // Lumen optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float LumenSceneViewDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 LumenReflectionQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 LumenGlobalIlluminationQuality = 2;

    // Mass AI optimizations (critical for dinosaur ecosystem)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    int32 MaxActiveMassAgents = 50000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    float MassAIUpdateFrequency = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    float DinosaurBehaviorComplexity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    float PerceptionRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass AI")
    int32 MaxBehaviorTreesPerFrame = 1000;

    // Physics optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsBodies = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxDestructionChunks = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float RagdollCullDistance = 15000.0f;

    // Streaming optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxStreamingCells = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float TextureStreamingPoolSize = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingDistanceMultiplier = 1.0f;

    // Audio optimizations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    int32 MaxAudioSources = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    int32 Max3DAudioSources = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AudioCullDistance = 10000.0f;

    FOptimizationSettings()
    {
        // Default to Ultra settings
    }

    static FOptimizationSettings GetSettingsForLevel(EOptimizationLevel Level);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EOptimizationLevel CurrentLevel = EOptimizationLevel::Ultra;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceHealth = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsStable = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TimeInCurrentLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OptimizationChangesThisFrame = 0;

    FPerformanceState()
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOptimizationLevelChanged, EOptimizationLevel, OldLevel, EOptimizationLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPerformanceCritical, float, FrameTime, float, TargetTime, FString, BottleneckSystem);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDynamicPerformanceOptimizer : public UObject
{
    GENERATED_BODY()

public:
    UDynamicPerformanceOptimizer();

    // Initialize optimizer with performance profiler
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void Initialize(UPerformanceProfiler* InProfiler, EPerformanceTarget InTarget);

    // Main update function - call every frame
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateOptimization(float DeltaTime);

    // Get current optimization state
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceState GetCurrentState() const { return CurrentState; }

    // Get current optimization settings
    UFUNCTION(BlueprintPure, Category = "Performance")
    FOptimizationSettings GetCurrentSettings() const { return CurrentSettings; }

    // Manual optimization level override
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EOptimizationLevel NewLevel, bool bForceApply = false);

    // Enable/disable automatic optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoOptimizationEnabled(bool bEnabled);

    // Optimization events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnOptimizationLevelChanged OnOptimizationLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceCritical OnPerformanceCritical;

    // Console commands for runtime tuning
    UFUNCTION(Exec, Category = "Performance")
    void SetOptimizationLevelExec(int32 Level);

    UFUNCTION(Exec, Category = "Performance")
    void ToggleAutoOptimization();

    UFUNCTION(Exec, Category = "Performance")
    void ShowOptimizationStats(bool bShow);

    UFUNCTION(Exec, Category = "Performance")
    void ForceOptimizationUpdate();

    // Specific system optimizations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRendering(EOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMassAI(EOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysics(EOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeStreaming(EOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeAudio(EOptimizationLevel Level);

protected:
    UPROPERTY()
    UPerformanceProfiler* PerformanceProfiler;

    UPROPERTY()
    EPerformanceTarget TargetPlatform;

    UPROPERTY()
    FPerformanceState CurrentState;

    UPROPERTY()
    FOptimizationSettings CurrentSettings;

    UPROPERTY()
    bool bAutoOptimizationEnabled = true;

    UPROPERTY()
    bool bShowDebugStats = false;

    // Performance monitoring
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    float AverageFrameTime = 16.67f;

    UPROPERTY()
    float PerformanceStabilityTimer = 0.0f;

    UPROPERTY()
    int32 ConsecutiveBadFrames = 0;

    UPROPERTY()
    int32 ConsecutiveGoodFrames = 0;

    // Optimization thresholds
    static constexpr float PERFORMANCE_GOOD_THRESHOLD = 0.85f;      // 85% of budget = good
    static constexpr float PERFORMANCE_BAD_THRESHOLD = 1.1f;        // 110% of budget = bad
    static constexpr float PERFORMANCE_CRITICAL_THRESHOLD = 1.5f;   // 150% of budget = critical
    static constexpr float STABILITY_REQUIRED_TIME = 2.0f;         // 2 seconds of stability before changing level
    static constexpr int32 MAX_FRAME_HISTORY = 60;                 // 1 second at 60fps
    static constexpr int32 CONSECUTIVE_FRAMES_THRESHOLD = 5;        // 5 consecutive bad/good frames to trigger change

    // Internal methods
    void AnalyzePerformance(const FPerformanceMetrics& Metrics);
    void DetermineOptimalLevel(float PerformanceHealth);
    void ApplyOptimizationSettings(const FOptimizationSettings& Settings);
    void UpdatePerformanceHistory(float FrameTime);
    float CalculatePerformanceHealth(const FPerformanceMetrics& Metrics);
    FString IdentifyBottleneckSystem(const FPerformanceMetrics& Metrics);
    void LogOptimizationChange(EOptimizationLevel OldLevel, EOptimizationLevel NewLevel, const FString& Reason);

    // Console variable setters
    void SetRenderingCVars(const FOptimizationSettings& Settings);
    void SetNaniteCVars(const FOptimizationSettings& Settings);
    void SetLumenCVars(const FOptimizationSettings& Settings);
    void SetMassAICVars(const FOptimizationSettings& Settings);
    void SetPhysicsCVars(const FOptimizationSettings& Settings);
    void SetStreamingCVars(const FOptimizationSettings& Settings);
    void SetAudioCVars(const FOptimizationSettings& Settings);
};

// Global optimizer subsystem
UCLASS()
class TRANSPERSONALGAME_API UDynamicPerformanceOptimizerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { return TStatId(); }

    UFUNCTION(BlueprintPure, Category = "Performance", meta = (CallInEditor = "true"))
    static UDynamicPerformanceOptimizer* GetPerformanceOptimizer();

    UPROPERTY()
    UDynamicPerformanceOptimizer* OptimizerInstance;
};