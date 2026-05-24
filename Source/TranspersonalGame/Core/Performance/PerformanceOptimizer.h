#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "PerformanceOptimizer.generated.h"

DECLARE_STATS_GROUP(TEXT("Performance Optimizer"), STATGROUP_PerformanceOptimizer, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Performance Analysis"), STAT_PerformanceAnalysis, STATGROUP_PerformanceOptimizer);
DECLARE_CYCLE_STAT(TEXT("LOD Management"), STAT_LODManagement, STATGROUP_PerformanceOptimizer);
DECLARE_CYCLE_STAT(TEXT("Culling Optimization"), STAT_CullingOptimization, STATGROUP_PerformanceOptimizer);
DECLARE_CYCLE_STAT(TEXT("Memory Optimization"), STAT_MemoryOptimization, STATGROUP_PerformanceOptimizer);

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low (30 FPS)"),
    Medium      UMETA(DisplayName = "Medium (45 FPS)"),
    High        UMETA(DisplayName = "High (60 FPS)"),
    Ultra       UMETA(DisplayName = "Ultra (90+ FPS)")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationCategory : uint8
{
    Rendering   UMETA(DisplayName = "Rendering"),
    Memory      UMETA(DisplayName = "Memory"),
    LOD         UMETA(DisplayName = "Level of Detail"),
    Culling     UMETA(DisplayName = "Culling"),
    Lighting    UMETA(DisplayName = "Lighting"),
    Particles   UMETA(DisplayName = "Particles"),
    Physics     UMETA(DisplayName = "Physics"),
    Audio       UMETA(DisplayName = "Audio")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PerformanceMetrics_40B
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
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    FCore_PerformanceMetrics_40B()
    {
        CurrentFPS = 0.0f;
        FrameTimeMS = 0.0f;
        GameThreadTimeMS = 0.0f;
        RenderThreadTimeMS = 0.0f;
        PhysicsTimeMS = 0.0f;
        MemoryUsageMB = 0.0f;
        ActivePhysicsObjects = 0;
        RenderedTriangles = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 ShadowCascades;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ShadowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableTemporalAA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LumenQuality;

    FPerf_OptimizationSettings()
        : LODDistanceScale(1.0f)
        , MaxLODLevel(3)
        , CullingDistance(5000.0f)
        , bEnableOcclusionCulling(true)
        , bEnableFrustumCulling(true)
        , ShadowCascades(4)
        , ShadowDistance(3000.0f)
        , bEnableTemporalAA(true)
        , bEnableLumen(true)
        , LumenQuality(1.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PerformanceMetrics_40B GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceMonitoringActive() const { return bIsMonitoringActive; }

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyPerformanceLevel(EPerf_PerformanceLevel PerformanceLevel);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    FPerf_OptimizationSettings GetCurrentOptimizationSettings() const { return CurrentSettings; }

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void OptimizeLODSettings(float DistanceScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetGlobalLODScale(float Scale);

    /**
     * Get singleton instance
     */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    static UPerformanceOptimizer* GetPerformanceOptimizer(UWorld* World);

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();

    // Internal optimization methods
    void OptimizeActorLODs();
    void OptimizeLightSources();
    void OptimizeParticleSystems();
    void OptimizeStaticMeshes();

protected:
    /** Target frame rate (60fps PC, 30fps console) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY()
    FPerf_PerformanceMetrics LastMetrics;

    UPROPERTY()
    bool bIsMonitoringActive;

    UPROPERTY()
    bool bAutoOptimizationEnabled;

    UPROPERTY()
    float TargetFPS;

    UPROPERTY()
    float MonitoringInterval;

    /** Current performance metrics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Status", meta = (AllowPrivateAccess = "true"))
    FCore_PerformanceMetrics_40B CurrentMetrics;

    // Performance tracking
    TArray<float> FPSHistory;
    TArray<float> FrameTimeHistory;
    int32 MaxHistorySize;

    // Console variables
    TAutoConsoleVariable<float> CVarLODDistanceScale;
    TAutoConsoleVariable<int32> CVarMaxLODLevel;
    TAutoConsoleVariable<float> CVarCullingDistance;
    TAutoConsoleVariable<int32> CVarShadowQuality;
    TAutoConsoleVariable<float> CVarLumenQuality;
};