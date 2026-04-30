#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "../SharedTypes.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"), 
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float UsedMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsActorCount;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        UsedMemoryMB = 0.0f;
        ActiveActorCount = 0;
        PhysicsActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxViewDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel;

    FPerf_PerformanceSettings()
    {
        TargetFPS = 60.0f;
        MinimumFPS = 30.0f;
        bEnableAutomaticLOD = true;
        bEnableDynamicResolution = true;
        MaxViewDistance = 10000.0f;
        MaxPhysicsActors = 100;
        PerformanceLevel = EPerf_PerformanceLevel::High;
    }
};

/**
 * Performance Manager - Monitors and optimizes game performance in real-time
 * Handles FPS monitoring, automatic LOD adjustment, memory management, and performance profiling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    // Performance settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFPS() const;

    // Automatic optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutomaticOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentHardware();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceSettings(const FPerf_PerformanceSettings& Settings);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetUsedMemoryMB() const;

    // Actor management for performance
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxPhysicsActors(int32 MaxActors);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODBias(float LODBias);

    // Console commands for performance
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void ShowPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void HidePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void DumpPerformanceReport();

protected:
    // Performance monitoring data
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceSettings PerformanceSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAutomaticOptimization;

    // Timing
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    float UpdateInterval;

    // Frame time tracking
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameHistorySize;

private:
    // Internal performance tracking
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutomaticOptimizations();
    
    // Console command handlers
    void ExecuteConsoleCommand(const FString& Command);
    
    // Performance calculation helpers
    float CalculateAverageFrameTime() const;
    void UpdateFrameTimeHistory(float DeltaTime);
    
    // Hardware detection
    void DetectHardwareCapabilities();
    
    // Timer handle for periodic updates
    FTimerHandle PerformanceUpdateTimer;
};