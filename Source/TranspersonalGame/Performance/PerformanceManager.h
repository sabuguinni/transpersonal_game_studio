/**
 * @file PerformanceManager.h
 * @brief Central performance management system for transpersonal game mechanics
 * @author Performance Optimizer
 * @version 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/ThreadSafeBool.h"
#include "PerformanceManager.generated.h"

// Forward declarations
class UConsciousnessPhysicsOptimized;
class UConsciousnessFieldComponent;

/**
 * Performance monitoring categories
 */
UENUM(BlueprintType)
enum class EPerformanceCategory : uint8
{
    Physics         UMETA(DisplayName = "Physics"),
    Consciousness   UMETA(DisplayName = "Consciousness"),
    Rendering       UMETA(DisplayName = "Rendering"),
    Audio           UMETA(DisplayName = "Audio"),
    Networking      UMETA(DisplayName = "Networking"),
    Overall         UMETA(DisplayName = "Overall")
};

/**
 * Performance quality presets
 */
UENUM(BlueprintType)
enum class EPerformancePreset : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato"),
    Custom      UMETA(DisplayName = "Custom")
};

/**
 * Performance metrics structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsage = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConsciousnessFields = 0;

    FPerformanceMetrics()
    {
        FrameTime = 0.0f;
        FPS = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
        MemoryUsage = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        ConsciousnessPhysicsTime = 0.0f;
        ActiveConsciousnessFields = 0;
    }
};

/**
 * Central performance management subsystem
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetPerformanceScore() const;

    // Quality management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformancePreset(EPerformancePreset Preset);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerformancePreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsAdaptiveQualityEnabled() const { return bAdaptiveQualityEnabled; }

    // Target performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MinAcceptableFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxCPUTime = 12.0f; // milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxGPUTime = 16.0f; // milliseconds

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimizeConsciousnessPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bAutoOptimizeRendering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float OptimizationCheckInterval = 1.0f;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChanged, const FPerformanceMetrics&, Metrics);
    UPROPERTY(BlueprintAssignable)
    FOnPerformanceChanged OnPerformanceChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualityChanged, EPerformancePreset, NewPreset);
    UPROPERTY(BlueprintAssignable)
    FOnQualityChanged OnQualityChanged;

private:
    // Performance tracking
    FPerformanceMetrics CurrentMetrics;
    TArray<float> FrameTimeHistory;
    float AverageFrameTime = 0.0f;
    bool bMonitoringEnabled = false;
    
    // Quality management
    EPerformancePreset CurrentPreset = EPerformancePreset::High;
    bool bAdaptiveQualityEnabled = true;
    float LastOptimizationTime = 0.0f;
    
    // Optimization state
    bool bNeedsOptimization = false;
    int32 ConsecutivePoorFrames = 0;
    static constexpr int32 PoorFrameThreshold = 30; // 0.5 seconds at 60 FPS
    
    // Internal methods
    void UpdatePerformanceMetrics();
    void CheckForOptimization();
    void ApplyPerformancePreset(EPerformancePreset Preset);
    void OptimizeConsciousnessPhysics();
    void OptimizeRendering();
    void CollectRenderingStats();
    void CollectConsciousnessStats();
    
    // Timer handle for periodic updates
    FTimerHandle PerformanceUpdateTimer;
    static constexpr float UpdateInterval = 0.1f; // 10 times per second
};

/**
 * Performance optimization helper functions
 */
UCLASS()
class TRANSPERSONALGAME_API UPerformanceOptimizationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // System information
    UFUNCTION(BlueprintPure, Category = "Performance")
    static int32 GetCPUCoreCount();

    UFUNCTION(BlueprintPure, Category = "Performance")
    static float GetAvailableMemoryGB();

    UFUNCTION(BlueprintPure, Category = "Performance")
    static FString GetGPUName();

    UFUNCTION(BlueprintPure, Category = "Performance")
    static bool IsVRMode();

    // Optimization utilities
    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void OptimizeTextureStreaming();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void SetConsoleVariable(const FString& VariableName, const FString& Value);

    UFUNCTION(BlueprintPure, Category = "Performance")
    static FString GetConsoleVariable(const FString& VariableName);

    // Consciousness-specific optimizations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void OptimizeConsciousnessFieldsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void SetConsciousnessLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static void EnableConsciousnessMultithreading(bool bEnable);
};