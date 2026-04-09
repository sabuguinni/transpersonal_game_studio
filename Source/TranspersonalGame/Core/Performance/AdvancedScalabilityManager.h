// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Scalability.h"
#include "AdvancedScalabilityManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedScalability, Log, All);

/**
 * Advanced scalability presets beyond engine defaults
 */
UENUM(BlueprintType)
enum class EAdvancedScalabilityPreset : uint8
{
    UltraPerformance    UMETA(DisplayName = "Ultra Performance"),
    HighPerformance     UMETA(DisplayName = "High Performance"),
    Balanced            UMETA(DisplayName = "Balanced"),
    HighQuality         UMETA(DisplayName = "High Quality"),
    UltraQuality        UMETA(DisplayName = "Ultra Quality"),
    Custom              UMETA(DisplayName = "Custom")
};

/**
 * Platform-specific scalability targets
 */
UENUM(BlueprintType)
enum class EScalabilityPlatformTarget : uint8
{
    HighEndPC           UMETA(DisplayName = "High-End PC (RTX 4080+)"),
    MidRangePC          UMETA(DisplayName = "Mid-Range PC (RTX 3070)"),
    LowEndPC            UMETA(DisplayName = "Low-End PC (GTX 1660)"),
    NextGenConsole      UMETA(DisplayName = "Next-Gen Console (PS5/Xbox Series X)"),
    CurrentGenConsole   UMETA(DisplayName = "Current-Gen Console (PS4 Pro/Xbox One X)"),
    LastGenConsole      UMETA(DisplayName = "Last-Gen Console (PS4/Xbox One)"),
    HighEndMobile       UMETA(DisplayName = "High-End Mobile"),
    MidRangeMobile      UMETA(DisplayName = "Mid-Range Mobile"),
    VRHeadset           UMETA(DisplayName = "VR Headset")
};

/**
 * Comprehensive scalability settings structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAdvancedScalabilitySettings
{
    GENERATED_BODY()

    // Basic scalability groups (0-3 scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ViewDistanceQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 AntiAliasingQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ShadowQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 PostProcessQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 TextureQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 EffectsQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 FoliageQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Scalability", meta = (ClampMin = "0", ClampMax = "3"))
    int32 ShadingQuality;

    // Advanced settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "10", ClampMax = "100"))
    float ResolutionScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bEnableDynamicResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.5", ClampMax = "1.0"))
    float MinDynamicResolutionScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced", meta = (ClampMin = "0.0", ClampMax = "3.0"))
    float LODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
    bool bEnableTemporalUpsampling;

    // Physics scalability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.008", ClampMax = "0.033"))
    float PhysicsSubstepDeltaTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "1", ClampMax = "8"))
    int32 PhysicsIterationCount;

    // Rendering scalability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (ClampMin = "512", ClampMax = "8192"))
    int32 MaxShadowResolution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (ClampMin = "1", ClampMax = "4"))
    int32 MaxCascades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableHZBOcclusion;

    // Memory scalability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "512", ClampMax = "8192"))
    int32 TextureStreamingPoolSizeMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TextureMipBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (ClampMin = "128", ClampMax = "1024"))
    int32 AudioMemoryPoolSizeMB;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets", meta = (ClampMin = "30", ClampMax = "120"))
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets", meta = (ClampMin = "8.0", ClampMax = "33.0"))
    float MaxFrameTime;

    FAdvancedScalabilitySettings()
        : ViewDistanceQuality(3)
        , AntiAliasingQuality(3)
        , ShadowQuality(3)
        , PostProcessQuality(3)
        , TextureQuality(3)
        , EffectsQuality(3)
        , FoliageQuality(3)
        , ShadingQuality(3)
        , ResolutionScale(100.0f)
        , bEnableDynamicResolution(true)
        , MinDynamicResolutionScale(0.7f)
        , LODBias(0.0f)
        , bEnableTemporalUpsampling(true)
        , MaxPhysicsObjects(1000)
        , PhysicsSubstepDeltaTime(0.016f)
        , PhysicsIterationCount(4)
        , MaxShadowResolution(2048)
        , MaxCascades(4)
        , bEnableOcclusionCulling(true)
        , bEnableHZBOcclusion(true)
        , TextureStreamingPoolSizeMB(2048)
        , TextureMipBias(0.0f)
        , AudioMemoryPoolSizeMB(256)
        , TargetFrameRate(60.0f)
        , MaxFrameTime(16.67f)
    {}
};

/**
 * Runtime performance metrics for adaptive scalability
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRuntimePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bIsPerformanceTarget;

    FRuntimePerformanceMetrics()
        : CurrentFrameTime(0.0f)
        , AverageFrameTime(0.0f)
        , CurrentFPS(0.0f)
        , MemoryUsageMB(0.0f)
        , GPUMemoryUsageMB(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , bIsPerformanceTarget(true)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScalabilityChanged, const FAdvancedScalabilitySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceTargetChanged, bool, bMetTarget, float, CurrentFPS);

/**
 * Advanced Scalability Manager
 * Provides comprehensive scalability management beyond engine defaults
 * Includes adaptive quality scaling based on runtime performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAdvancedScalabilityManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAdvancedScalabilityManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdaptiveScalability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AdaptiveUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDetailedLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EScalabilityPlatformTarget PlatformTarget;

    // Current settings
    UPROPERTY(BlueprintReadOnly, Category = "Current Settings")
    FAdvancedScalabilitySettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Current Settings")
    EAdvancedScalabilityPreset CurrentPreset;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScalabilityChanged OnScalabilityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPerformanceTargetChanged OnPerformanceTargetChanged;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void ApplyScalabilityPreset(EAdvancedScalabilityPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void ApplyPlatformOptimizedSettings(EScalabilityPlatformTarget Platform);

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void ApplyCustomSettings(const FAdvancedScalabilitySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    FAdvancedScalabilitySettings GetCurrentSettings() const { return CurrentSettings; }

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void SetTargetFrameRate(float FrameRate);

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void SetResolutionScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Scalability")
    void SetLODBias(float Bias);

    // Adaptive scalability
    UFUNCTION(BlueprintCallable, Category = "Adaptive")
    void EnableAdaptiveScalability(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Adaptive")
    void ForceAdaptiveUpdate();

    UFUNCTION(BlueprintCallable, Category = "Adaptive")
    FRuntimePerformanceMetrics GetCurrentPerformanceMetrics();

    // Quality presets
    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SetUltraPerformancePreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SetHighPerformancePreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SetBalancedPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SetHighQualityPreset();

    UFUNCTION(BlueprintCallable, Category = "Presets")
    void SetUltraQualityPreset();

    // Platform-specific methods
    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForHighEndPC();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForMidRangePC();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForLowEndPC();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForNextGenConsole();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForCurrentGenConsole();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForMobile();

    UFUNCTION(BlueprintCallable, Category = "Platform")
    void OptimizeForVR();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SaveCurrentSettingsAsPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LoadSettingsFromPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ResetToEngineDefaults();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void BenchmarkCurrentSettings(float Duration = 30.0f);

    // Debug methods
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogCurrentSettings();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ExportSettingsToFile(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ImportSettingsFromFile(const FString& FilePath);

private:
    // Internal state
    bool bAdaptiveScalabilityEnabled;
    float LastAdaptiveUpdateTime;
    TArray<float> RecentFrameTimes;
    float PerformanceScore;
    bool bMeetingPerformanceTarget;
    
    // Timer handles
    FTimerHandle AdaptiveUpdateTimerHandle;
    FTimerHandle BenchmarkTimerHandle;
    
    // Benchmarking
    bool bBenchmarkActive;
    TArray<float> BenchmarkFrameTimes;
    TArray<float> BenchmarkMemoryUsage;
    float BenchmarkStartTime;
    
    // Preset storage
    TMap<FString, FAdvancedScalabilitySettings> CustomPresets;
    
    // Internal methods
    void UpdateAdaptiveScalability();
    void AnalyzePerformance();
    void AdjustQualitySettings();
    void ApplySettingsToEngine(const FAdvancedScalabilitySettings& Settings);
    
    // Performance monitoring
    FRuntimePerformanceMetrics CollectPerformanceMetrics();
    float CalculatePerformanceScore();
    bool IsPerformanceTargetMet();
    
    // Quality adjustment algorithms
    void AdjustForLowPerformance();
    void AdjustForHighPerformance();
    void AdjustResolutionScale(float TargetAdjustment);
    void AdjustLODBias(float TargetAdjustment);
    void AdjustShadowQuality(int32 TargetAdjustment);
    void AdjustEffectsQuality(int32 TargetAdjustment);
    
    // Console variable management
    void SetConsoleVariable(const FString& VariableName, float Value);
    void SetConsoleVariable(const FString& VariableName, int32 Value);
    void SetScalabilityGroup(const FString& GroupName, int32 Value);
    
    // Platform detection and optimization
    EScalabilityPlatformTarget DetectCurrentPlatform();
    FAdvancedScalabilitySettings GetPlatformOptimizedSettings(EScalabilityPlatformTarget Platform);
    
    // Preset management
    FAdvancedScalabilitySettings GetPresetSettings(EAdvancedScalabilityPreset Preset);
    void SavePresetToFile(const FString& PresetName, const FAdvancedScalabilitySettings& Settings);
    FAdvancedScalabilitySettings LoadPresetFromFile(const FString& PresetName);
    
    // File I/O
    void SaveSettingsToConfig();
    void LoadSettingsFromConfig();
    FString SerializeSettings(const FAdvancedScalabilitySettings& Settings);
    FAdvancedScalabilitySettings DeserializeSettings(const FString& SettingsString);
    
    // Benchmarking
    void StartBenchmark();
    void UpdateBenchmark();
    void FinishBenchmark();
    void AnalyzeBenchmarkResults();
    
    // Validation
    bool ValidateSettings(const FAdvancedScalabilitySettings& Settings);
    void ClampSettingsToValidRange(FAdvancedScalabilitySettings& Settings);
    
    // Event handling
    void OnSettingsChanged();
    void OnPerformanceTargetStatusChanged(bool bMetTarget, float CurrentFPS);
};