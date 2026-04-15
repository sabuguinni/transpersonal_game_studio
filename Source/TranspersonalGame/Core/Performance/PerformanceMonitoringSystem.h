#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "PerformanceMonitoringSystem.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentLevel = EPerf_PerformanceLevel::High;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        FrameTime = 16.67f;
        DrawCalls = 0;
        VisibleActors = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        CurrentLevel = EPerf_PerformanceLevel::High;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutomaticLOD = true;

    FPerf_LODSettings()
    {
        LODDistanceScale = 1.0f;
        CullingDistanceScale = 1.0f;
        MaxLODLevel = 3;
        bEnableAutomaticLOD = true;
    }
};

/**
 * Performance Monitoring System for Transpersonal Game
 * Tracks FPS, memory usage, draw calls, and automatically adjusts quality settings
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitoringSystem : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitoringSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutomaticQualityAdjustment(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings(const FPerf_LODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxDrawCalls = 2000;

protected:
    // Internal performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutomaticQualityAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings CurrentLODSettings;

private:
    // Internal tracking variables
    float LastMetricsUpdate;
    TArray<float> FPSHistory;
    int32 MaxFPSHistorySize = 60;

    // Performance adjustment functions
    void UpdatePerformanceMetrics();
    void AdjustQualitySettings();
    void ApplyPerformanceLevelSettings(EPerf_PerformanceLevel Level);
    float CalculateAverageFPS() const;
    void UpdateFPSHistory(float CurrentFPS);
};

/**
 * World Subsystem for global performance management
 */
UCLASS()
class TRANSPERSONALGAME_API UPerf_PerformanceWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    static UPerf_PerformanceWorldSubsystem* Get(const UObject* WorldContext);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    APerf_PerformanceMonitoringSystem* GetPerformanceMonitor() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterPerformanceMonitor(APerf_PerformanceMonitoringSystem* Monitor);

protected:
    UPROPERTY()
    APerf_PerformanceMonitoringSystem* PerformanceMonitor;
};