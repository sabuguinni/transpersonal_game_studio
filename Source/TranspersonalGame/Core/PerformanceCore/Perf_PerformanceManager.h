#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Platform.h"
#include "Stats/Stats.h"
#include "Perf_PerformanceManager.generated.h"

DECLARE_STATS_GROUP(TEXT("TranspersonalGame Performance"), STATGROUP_TranspersonalPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Frame Time"), STAT_FrameTime, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("Dinosaur AI"), STAT_DinosaurAI, STATGROUP_TranspersonalPerformance);
DECLARE_CYCLE_STAT(TEXT("World Generation"), STAT_WorldGeneration, STATGROUP_TranspersonalPerformance);

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (60+ FPS)"),
    High        UMETA(DisplayName = "High (45-60 FPS)"),
    Medium      UMETA(DisplayName = "Medium (30-45 FPS)"),
    Low         UMETA(DisplayName = "Low (20-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<20 FPS)")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationTarget : uint8
{
    PC_HighEnd      UMETA(DisplayName = "PC High-End (60 FPS)"),
    PC_MidRange     UMETA(DisplayName = "PC Mid-Range (45 FPS)"),
    Console_Next    UMETA(DisplayName = "Next-Gen Console (30 FPS)"),
    Console_Last    UMETA(DisplayName = "Last-Gen Console (30 FPS)"),
    Mobile          UMETA(DisplayName = "Mobile (30 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::Medium;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 30.0f;
        AverageFrameTime = 33.33f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        PerformanceLevel = EPerf_PerformanceLevel::Medium;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float VegetationLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float RockLODDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DetailObjectLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxVisibleDinosaurs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxVisibleVegetation = 1000;

    FPerf_LODSettings()
    {
        DinosaurLODDistance = 5000.0f;
        VegetationLODDistance = 3000.0f;
        RockLODDistance = 2000.0f;
        DetailObjectLODDistance = 1000.0f;
        MaxVisibleDinosaurs = 50;
        MaxVisibleVegetation = 1000;
    }
};

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
    void SetOptimizationTarget(EPerf_OptimizationTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceSettings();

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    FPerf_LODSettings GetLODSettings() const;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODDistances();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicPerformanceScaling(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsDynamicScalingEnabled() const;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetMemoryUsageMB() const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeMemoryUsage();

    // Rendering optimization
    UFUNCTION(BlueprintCallable, Category = "Rendering")
    void SetShadowQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    void SetTextureQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    void SetPostProcessQuality(int32 Quality);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_OptimizationTarget CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bDynamicScalingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MinAcceptableFPS;

private:
    FTimerHandle PerformanceUpdateTimer;
    TArray<float> FrameTimeHistory;
    static const int32 FrameHistorySize = 60;

    void UpdatePerformanceMetrics();
    void CalculateAverageFrameTime();
    EPerf_PerformanceLevel DeterminePerformanceLevel(float FPS) const;
    void ApplyTargetSettings();
    void ApplyLODSettings();
    void ApplyRenderingSettings();
    void ApplyMemorySettings();
};