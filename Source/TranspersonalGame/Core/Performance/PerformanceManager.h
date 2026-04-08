// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "PerformanceManager.generated.h"

/**
 * @brief Core Performance Management System for Transpersonal Game
 * 
 * Ensures stable performance targets:
 * - 60fps on PC (High-end desktop)
 * - 30fps on Console (PS5/Xbox Series X)
 * - Dynamic quality scaling based on performance metrics
 * - Real-time monitoring and adjustment of physics systems
 * 
 * Performance Philosophy:
 * - Performance is not the enemy of quality — it's the condition for quality to be experienced
 * - A beautiful effect that causes stuttering doesn't exist for the player
 * - Find ways to say YES to visual features within the frame budget
 * 
 * @author Performance Optimizer — Agent #4
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    /**
     * @brief Initialize performance monitoring and target settings
     * 
     * Detects platform and sets appropriate performance targets:
     * - PC: 60fps target (16.67ms frame budget)
     * - Console: 30fps target (33.33ms frame budget)
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void InitializePerformanceTargets();

    /**
     * @brief Get current frame performance metrics
     * 
     * @return Current performance data including frame time, GPU time, memory usage
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerformanceMetrics GetCurrentPerformanceMetrics() const;

    /**
     * @brief Adjust physics quality based on current performance
     * 
     * Dynamically scales:
     * - Physics substeps
     * - Collision complexity
     * - Ragdoll simulation count
     * - Destruction detail level
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustPhysicsQuality();

    /**
     * @brief Set performance quality preset
     * 
     * @param QualityLevel Quality level to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceQuality(EPerformanceQuality QualityLevel);

    /**
     * @brief Enable/disable dynamic performance scaling
     * 
     * @param bEnabled Whether to enable automatic performance adjustments
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicScaling(bool bEnabled);

    /**
     * @brief Force garbage collection if memory usage is high
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollectionIfNeeded();

    /**
     * @brief Get performance bottleneck analysis
     * 
     * @return Analysis of current performance bottlenecks
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceBottleneckAnalysis AnalyzePerformanceBottlenecks();

    /**
     * @brief Log performance metrics to file for analysis
     */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceMetrics();

protected:
    /** Performance quality levels */
    UENUM(BlueprintType)
    enum class EPerformanceQuality : uint8
    {
        Low         UMETA(DisplayName = "Low (30fps Console)"),
        Medium      UMETA(DisplayName = "Medium (45fps)"),
        High        UMETA(DisplayName = "High (60fps PC)"),
        Epic        UMETA(DisplayName = "Epic (60fps High-end)")
    };

    /** Platform performance targets */
    UENUM(BlueprintType)
    enum class EPerformanceTarget : uint8
    {
        Console_30fps   UMETA(DisplayName = "Console 30fps"),
        PC_60fps        UMETA(DisplayName = "PC 60fps"),
        PC_120fps       UMETA(DisplayName = "PC 120fps")
    };

    /** Performance metrics structure */
    USTRUCT(BlueprintType)
    struct FPerformanceMetrics
    {
        GENERATED_BODY()

        /** Current frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float FrameTimeMS = 0.0f;

        /** Current frames per second */
        UPROPERTY(BlueprintReadOnly)
        float CurrentFPS = 0.0f;

        /** GPU frame time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GPUTimeMS = 0.0f;

        /** Game thread time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float GameThreadTimeMS = 0.0f;

        /** Render thread time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float RenderThreadTimeMS = 0.0f;

        /** Memory usage in MB */
        UPROPERTY(BlueprintReadOnly)
        float MemoryUsageMB = 0.0f;

        /** Number of draw calls */
        UPROPERTY(BlueprintReadOnly)
        int32 DrawCalls = 0;

        /** Number of triangles rendered */
        UPROPERTY(BlueprintReadOnly)
        int32 TriangleCount = 0;

        /** Physics simulation time in milliseconds */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsTimeMS = 0.0f;

        /** Number of active physics bodies */
        UPROPERTY(BlueprintReadOnly)
        int32 ActivePhysicsBodies = 0;

        /** Whether performance target is being met */
        UPROPERTY(BlueprintReadOnly)
        bool bMeetingTarget = true;
    };

    /** Performance bottleneck analysis */
    USTRUCT(BlueprintType)
    struct FPerformanceBottleneckAnalysis
    {
        GENERATED_BODY()

        /** Primary bottleneck type */
        UPROPERTY(BlueprintReadOnly)
        FString PrimaryBottleneck = TEXT("None");

        /** CPU bound percentage */
        UPROPERTY(BlueprintReadOnly)
        float CPUBoundPercentage = 0.0f;

        /** GPU bound percentage */
        UPROPERTY(BlueprintReadOnly)
        float GPUBoundPercentage = 0.0f;

        /** Memory pressure level (0-100) */
        UPROPERTY(BlueprintReadOnly)
        float MemoryPressure = 0.0f;

        /** Physics overhead percentage */
        UPROPERTY(BlueprintReadOnly)
        float PhysicsOverhead = 0.0f;

        /** Recommended actions */
        UPROPERTY(BlueprintReadOnly)
        TArray<FString> RecommendedActions;
    };

    /** Current performance target */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    EPerformanceTarget PerformanceTarget = EPerformanceTarget::PC_60fps;

    /** Current quality level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    EPerformanceQuality CurrentQuality = EPerformanceQuality::High;

    /** Enable dynamic quality scaling */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    bool bEnableDynamicScaling = true;

    /** Target frame time in milliseconds */
    UPROPERTY(BlueprintReadOnly, Category = "Performance Settings")
    float TargetFrameTimeMS = 16.67f; // 60fps default

    /** Frame time tolerance before scaling kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float FrameTimeTolerance = 2.0f; // 2ms tolerance

    /** Memory usage threshold for GC (MB) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (ClampMin = "1000", ClampMax = "8000"))
    float MemoryThresholdMB = 4000.0f;

    /** Physics quality scaling settings */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    TMap<EPerformanceQuality, int32> PhysicsSubstepCounts = {
        {EPerformanceQuality::Low, 2},
        {EPerformanceQuality::Medium, 4},
        {EPerformanceQuality::High, 6},
        {EPerformanceQuality::Epic, 8}
    };

    /** Maximum physics bodies per quality level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    TMap<EPerformanceQuality, int32> MaxPhysicsBodies = {
        {EPerformanceQuality::Low, 500},
        {EPerformanceQuality::Medium, 1000},
        {EPerformanceQuality::High, 2000},
        {EPerformanceQuality::Epic, 4000}
    };

    /** Collision LOD distances per quality level */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Performance")
    TMap<EPerformanceQuality, float> CollisionLODDistances = {
        {EPerformanceQuality::Low, 1000.0f},
        {EPerformanceQuality::Medium, 2000.0f},
        {EPerformanceQuality::High, 5000.0f},
        {EPerformanceQuality::Epic, 10000.0f}
    };

private:
    /** Initialize platform-specific settings */
    void InitializePlatformSettings();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Check if performance target is being met */
    bool IsPerformanceTargetMet() const;
    
    /** Scale quality up if performance allows */
    void ScaleQualityUp();
    
    /** Scale quality down if performance requires */
    void ScaleQualityDown();
    
    /** Apply quality settings to engine systems */
    void ApplyQualitySettings();
    
    /** Get current platform type */
    FString GetCurrentPlatform() const;
    
    /** Calculate performance score (0-100) */
    float CalculatePerformanceScore() const;

    /** Performance tracking variables */
    FPerformanceMetrics CurrentMetrics;
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 60; // 1 second at 60fps
    
    /** Timing for performance adjustments */
    float LastQualityAdjustmentTime = 0.0f;
    float QualityAdjustmentCooldown = 2.0f; // 2 seconds between adjustments
    
    /** Performance logging */
    bool bLogPerformance = false;
    FString PerformanceLogPath;
    
    /** Cached references */
    UPROPERTY()
    UWorld* CachedWorld;
    
    /** Performance delegates */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceChanged, EPerformanceQuality, NewQuality);
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceChanged OnPerformanceQualityChanged;
};