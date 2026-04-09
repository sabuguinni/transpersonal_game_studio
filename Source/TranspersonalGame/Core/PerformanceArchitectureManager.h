// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "PerformanceArchitectureManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceArchitecture, Log, All);

/**
 * Performance Architecture Manager
 * 
 * PERFORMANCE TARGETS (as defined by Engine Architect):
 * - PC High-End: 60fps @ 1440p, 8GB RAM
 * - PC Mid-Range: 45fps @ 1080p, 6GB RAM  
 * - Console Next-Gen: 30fps @ 4K, 16GB RAM
 * - Console Current-Gen: 30fps @ 1080p, 8GB RAM
 * 
 * CRITICAL SYSTEMS BUDGETS:
 * - World Partition: 2GB memory, 16.67ms frame budget
 * - Nanite: 1GB memory, 8ms frame budget
 * - Lumen: 1.5GB memory, 6ms frame budget
 * - Mass Entity: 512MB memory, 4ms frame budget
 * - Audio: 256MB memory, 2ms frame budget
 * - UI: 128MB memory, 1ms frame budget
 */

UENUM(BlueprintType)
enum class EPerformanceTier : uint8
{
    Low = 0         UMETA(DisplayName = "Low - 30fps target"),
    Medium = 1      UMETA(DisplayName = "Medium - 45fps target"),
    High = 2        UMETA(DisplayName = "High - 60fps target"),
    Ultra = 3       UMETA(DisplayName = "Ultra - 120fps target")
};

UENUM(BlueprintType)
enum class EPerformanceCategory : uint8
{
    Rendering = 0   UMETA(DisplayName = "Rendering Pipeline"),
    Physics = 1     UMETA(DisplayName = "Physics Simulation"),
    AI = 2          UMETA(DisplayName = "AI & Behavior"),
    Audio = 3       UMETA(DisplayName = "Audio Processing"),
    Streaming = 4   UMETA(DisplayName = "Asset Streaming"),
    UI = 5          UMETA(DisplayName = "User Interface"),
    Network = 6     UMETA(DisplayName = "Networking"),
    Memory = 7      UMETA(DisplayName = "Memory Management")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryBudgetMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCriticalSystem;

    FPerformanceBudget()
    {
        SystemName = TEXT("");
        Category = EPerformanceCategory::Rendering;
        FrameTimeBudgetMS = 16.67f; // 60fps default
        MemoryBudgetMB = 100.0f;
        GPUBudgetMS = 8.0f;
        Priority = 100;
        bCriticalSystem = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CPUTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float StreamingBandwidthMBps;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FDateTime Timestamp;

    FPerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTimeMS = 0.0f;
        MemoryUsageMB = 0.0f;
        GPUTimeMS = 0.0f;
        CPUTimeMS = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        StreamingBandwidthMBps = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceAlert
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FString AlertMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    float Severity; // 0.0 = Info, 1.0 = Critical

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Alert")
    bool bResolved;

    FPerformanceAlert()
    {
        SystemName = TEXT("");
        AlertMessage = TEXT("");
        Severity = 0.0f;
        Timestamp = FDateTime::Now();
        bResolved = false;
    }
};

/**
 * Performance Architecture Manager
 * Monitors and enforces performance budgets across all engine systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceArchitectureManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Performance Budget Management
    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    void SetPerformanceTier(EPerformanceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    void RegisterSystemBudget(const FPerformanceBudget& Budget);

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    FPerformanceBudget GetSystemBudget(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    TArray<FPerformanceBudget> GetBudgetsByCategory(EPerformanceCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Performance Budget")
    void UpdateSystemBudget(const FString& SystemName, const FPerformanceBudget& NewBudget);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    FPerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    TArray<FPerformanceMetrics> GetMetricsHistory(int32 SampleCount = 60);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StartPerformanceCapture(float DurationSeconds);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    void StopPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitoring")
    bool IsPerformanceTargetMet();

    // Alert System
    UFUNCTION(BlueprintCallable, Category = "Performance Alerts")
    TArray<FPerformanceAlert> GetActiveAlerts();

    UFUNCTION(BlueprintCallable, Category = "Performance Alerts")
    void ClearAlert(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance Alerts")
    void ClearAllAlerts();

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeForCurrentHardware();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ApplyPerformancePreset(EPerformanceTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void EnableAdaptiveQuality(bool bEnabled);

    // Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Performance Diagnostics")
    FString GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Diagnostics")
    void ExportPerformanceData(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Performance Diagnostics")
    TArray<FString> GetPerformanceBottlenecks();

    // Architecture Compliance
    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    bool ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    TArray<FString> GetNonCompliantSystems();

    UFUNCTION(BlueprintCallable, Category = "Architecture Compliance")
    void EnforceArchitecturalStandards();

protected:
    // Performance budgets for all systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TMap<FString, FPerformanceBudget> SystemBudgets;

    // Current performance tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerformanceTier CurrentPerformanceTier;

    // Performance monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bPerformanceMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    int32 MaxMetricsHistorySize;

    // Adaptive quality settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Quality")
    float QualityAdjustmentThreshold;

    // Performance data
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TArray<FPerformanceMetrics> MetricsHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TArray<FPerformanceAlert> ActiveAlerts;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    bool bPerformanceCaptureActive;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    float CaptureStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    float CaptureDuration;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceBudgets();
    void ProcessPerformanceAlerts();
    void ApplyAdaptiveQuality();
    
    // Budget management
    void SetupDefaultBudgets();
    void ApplyTierSpecificBudgets(EPerformanceTier Tier);
    
    // Alert management
    void CreateAlert(const FString& SystemName, const FString& Message, float Severity);
    void ResolveAlert(const FString& SystemName);
    
    // Optimization helpers
    void OptimizeRenderingSettings();
    void OptimizePhysicsSettings();
    void OptimizeAudioSettings();
    void OptimizeStreamingSettings();
    
    // Metrics collection
    FPerformanceMetrics CollectCurrentMetrics();
    void LogPerformanceData(const FPerformanceMetrics& Metrics);
    
    // Validation
    bool IsSystemWithinBudget(const FString& SystemName, const FPerformanceMetrics& Metrics);
    float CalculateSystemCompliance(const FString& SystemName);
    
    // Timing
    float LastUpdateTime;
    float UpdateInterval;
};

/**
 * World Performance Manager
 * Handles world-specific performance concerns
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UWorldPerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Tick(float DeltaTime) override;

    // World-specific performance management
    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void OptimizeWorldForPerformance();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void SetWorldComplexityBudget(float ComplexityScore);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    float GetWorldComplexityScore();

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void EnableLODManagement(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "World Performance")
    void ConfigureCullingDistances(float NearDistance, float FarDistance);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float WorldComplexityScore;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bLODManagementEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float NearCullingDistance;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float FarCullingDistance;

private:
    void AnalyzeWorldComplexity();
    void ApplyWorldOptimizations();
    void UpdateLODSettings();
    void ManageCullingDistances();
};