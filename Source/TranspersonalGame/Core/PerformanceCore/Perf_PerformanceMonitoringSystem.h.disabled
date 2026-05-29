#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "Perf_PerformanceMonitoringSystem.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerf, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Physics Update Time"), STAT_PhysicsUpdateTime, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("AI Processing Time"), STAT_AIProcessingTime, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("Rendering Time"), STAT_RenderingTime, STATGROUP_TranspersonalPerf);
DECLARE_CYCLE_STAT(TEXT("Animation Update Time"), STAT_AnimationUpdateTime, STATGROUP_TranspersonalPerf);

DECLARE_DWORD_COUNTER_STAT(TEXT("Active Actors"), STAT_ActiveActors, STATGROUP_TranspersonalPerf);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible Primitives"), STAT_VisiblePrimitives, STATGROUP_TranspersonalPerf);
DECLARE_DWORD_COUNTER_STAT(TEXT("Draw Calls"), STAT_DrawCalls, STATGROUP_TranspersonalPerf);

DECLARE_MEMORY_STAT(TEXT("Physics Memory"), STAT_PhysicsMemory, STATGROUP_TranspersonalPerf);
DECLARE_MEMORY_STAT(TEXT("Animation Memory"), STAT_AnimationMemory, STATGROUP_TranspersonalPerf);
DECLARE_MEMORY_STAT(TEXT("Texture Memory"), STAT_TextureMemory, STATGROUP_TranspersonalPerf);

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationMode : uint8
{
    Automatic   UMETA(DisplayName = "Automatic Optimization"),
    Manual      UMETA(DisplayName = "Manual Control"),
    Aggressive  UMETA(DisplayName = "Aggressive Optimization"),
    Conservative UMETA(DisplayName = "Conservative Optimization")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

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
    int32 ActiveActorCount;

    FPerf_PerformanceMetrics()
        : FrameTime(0.0f)
        , FPS(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , GPUTime(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , MemoryUsageMB(0.0f)
        , ActiveActorCount(0)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODBias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bEnableDynamicShadows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float ParticleSystemScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    bool bEnablePostProcessing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    float ScreenPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullDistanceScale;

    FPerf_OptimizationSettings()
        : LODBias(0.0f)
        , LODDistanceScale(1.0f)
        , bEnableDynamicShadows(true)
        , ShadowDistanceScale(1.0f)
        , ParticleSystemScale(1.0f)
        , bEnablePostProcessing(true)
        , ScreenPercentage(100.0f)
        , CullDistanceScale(1.0f)
    {
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceMonitoringComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceMonitoringComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationMode(EPerf_OptimizationMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceLevel(EPerf_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationMode OptimizationMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel TargetPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

private:
    float LastMonitoringTime;
    TArray<float> FrameTimeHistory;
    
    void UpdatePerformanceMetrics();
    void ApplyOptimizations();
    void AdjustLODSettings();
    void AdjustShadowSettings();
    void AdjustEffectSettings();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitoringSystem : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitoringSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    static APerf_PerformanceMonitoringSystem* GetPerformanceMonitoringSystem(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterPerformanceComponent(UPerf_PerformanceMonitoringComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterPerformanceComponent(UPerf_PerformanceMonitoringComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetGlobalPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalOptimizationMode(EPerf_OptimizationMode Mode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceBenchmark();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPerf_PerformanceMonitoringComponent* MonitoringComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationThreshold;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<UPerf_PerformanceMonitoringComponent*> RegisteredComponents;

private:
    static APerf_PerformanceMonitoringSystem* Instance;
    
    void PerformGlobalOptimization();
    void UpdateGlobalMetrics();
    bool ShouldOptimize() const;
};