#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_OptimizationLevel CurrentOptimizationLevel;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        TargetFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        ActiveActorCount = 0;
        CurrentOptimizationLevel = EPerf_OptimizationLevel::Medium;
    }
};

USTRUCT(BlueprintType)
struct FPerf_OptimizationZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EPerf_OptimizationLevel OptimizationLevel;

    FPerf_OptimizationZone()
    {
        ZoneName = TEXT("DefaultZone");
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        TargetFPS = 60.0f;
        OptimizationLevel = EPerf_OptimizationLevel::Medium;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    // Optimization management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationToActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CreateOptimizationZone(const FPerf_OptimizationZone& ZoneData);

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(float LOD0Distance, float LOD1Distance, float LOD2Distance);

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RunPerformanceBenchmark();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<FPerf_OptimizationZone> OptimizationZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsMonitoringActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LOD2Distance;

private:
    void UpdatePerformanceMetrics();
    void ApplyOptimizationCommands(EPerf_OptimizationLevel Level);
    void OptimizeStaticMeshActors();
    void OptimizeLightActors();
    void CleanupDuplicateActors();

    FTimerHandle PerformanceUpdateTimer;
    float LastFrameTime;
    int32 FrameCounter;
};