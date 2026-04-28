#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PerformanceOptimizerV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceOptimizer, Log, All);

DECLARE_STATS_GROUP(TEXT("TranspersonalPerformance"), STATGROUP_TranspersonalPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Performance Analysis"), STAT_PerformanceAnalysis, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("LOD Optimization"), STAT_LODOptimization, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("Culling Optimization"), STAT_CullingOptimization, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("Memory Optimization"), STAT_MemoryOptimization, STATGROUP_TranspersonalPerformance);

UENUM(BlueprintType)
enum class ECore_PerformanceTarget : uint8
{
    PC_60FPS        UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS   UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS    UMETA(DisplayName = "Mobile 30 FPS"),
    VR_90FPS        UMETA(DisplayName = "VR 90 FPS")
};

UENUM(BlueprintType)
enum class ECore_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Epic        UMETA(DisplayName = "Epic"),
    Cinematic   UMETA(DisplayName = "Cinematic")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PerformanceMetrics_042
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

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
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisiblePrimitives;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OccludedPrimitives;

    FCore_PerformanceMetrics_042()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        VisiblePrimitives = 0;
        OccludedPrimitives = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    ECore_PerformanceTarget TargetPlatform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    ECore_PerformanceLevel QualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    float ViewDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    int32 ShadowQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    int32 TextureQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    bool bEnableNanite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    bool bEnableLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Profile")
    bool bEnableTSR;

    FPerformanceProfile()
    {
        TargetPlatform = ECore_PerformanceTarget::PC_60FPS;
        QualityLevel = ECore_PerformanceLevel::High;
        TargetFrameRate = 60.0f;
        MaxFrameTime = 16.67f; // 60fps
        MaxDrawCalls = 5000;
        MaxMemoryUsageMB = 8192.0f;
        ViewDistanceScale = 1.0f;
        ShadowQuality = 3;
        TextureQuality = 3;
        bEnableNanite = true;
        bEnableLumen = true;
        bEnableTSR = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceOptimizerV43 : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceOptimizerV43();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void StartPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void StopPerformanceAnalysis();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    FCore_PerformanceMetrics_042 GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void SetPerformanceProfile(const FPerformanceProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    FPerformanceProfile GetCurrentPerformanceProfile() const { return CurrentProfile; }

    // Optimization Functions
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeForTarget(ECore_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void ApplyDynamicLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeCullingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeForDinosaurAI(int32 DinosaurCount);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeRenderingPipeline();

    // Nanite Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeNaniteSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void EnableNaniteForMeshes(const TArray<UStaticMesh*>& Meshes);

    // Lumen Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeLumenSettings();

    // TSR Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeTSRSettings();

    // Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void SavePerformanceProfile(const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    bool LoadPerformanceProfile(const FString& ProfileName);

protected:
    // Internal optimization functions
    void ApplyScalabilitySettings();
    void OptimizeTextureStreaming();
    void OptimizeShadowSettings();
    void OptimizePostProcessing();
    void OptimizePhysicsSettings();
    void MonitorPerformanceMetrics();
    void AdjustDynamicSettings();
    void HandlePerformanceDrop();
    void HandlePerformanceSpike();

    // Console command helpers
    void ExecuteConsoleCommand(const FString& Command);
    void SetCVar(const FString& CVarName, float Value);
    void SetCVar(const FString& CVarName, int32 Value);
    void SetCVar(const FString& CVarName, bool bValue);

private:
    UPROPERTY()
    FPerformanceProfile CurrentProfile;

    UPROPERTY()
    FCore_PerformanceMetrics_042 CurrentMetrics;

    // Performance monitoring
    FTimerHandle PerformanceMonitorTimer;
    bool bIsAnalyzing;
    float AnalysisStartTime;
    
    // Performance history
    TArray<FCore_PerformanceMetrics_042> MetricsHistory;
    int32 MaxHistorySize;

    // Dynamic adjustment thresholds
    float FrameTimeThreshold;
    float MemoryThreshold;
    int32 DrawCallThreshold;

    // Optimization flags
    bool bDynamicLODEnabled;
    bool bDynamicCullingEnabled;
    bool bDynamicQualityEnabled;

    // Performance targets
    static const float PC_60FPS_TARGET;
    static const float CONSOLE_30FPS_TARGET;
    static const float MOBILE_30FPS_TARGET;
    static const float VR_90FPS_TARGET;
};